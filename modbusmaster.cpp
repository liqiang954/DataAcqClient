#include "modbusmaster.h"
#include "QSerialPort.h"
#include "time.h"
#include <QEventLoop>
#include <QElapsedTimer>
#include <cstring>



ModbusMaster::ModbusMaster()
{
    flag_stop=false;
    timer=new QTimer();
    loopooling_thread=std::thread();

    modbus_rtuMaster = new QModbusRtuSerialMaster(this);
    modbus_rtuMaster->setConnectionParameter(QModbusDevice::SerialPortNameParameter, Config::PortName);
    //need change edit
    modbus_rtuMaster->setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::NoParity);
    modbus_rtuMaster->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, QSerialPort::Baud9600);
    modbus_rtuMaster->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
    modbus_rtuMaster->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop);



}

ModbusMaster::~ModbusMaster()
{
    this->flag_stop=true;   //quit acq thread
    delete modbus_rtuMaster;
    delete timer;

}

void ModbusMaster::ModbusMaster_init()
{
    if(modbus_rtuMaster->connectDevice())
        qDebug()<<"modbus_rtuMaster 连接成功";
    else
        qDebug()<<"modbus_rtuMaster 连接失败";


}

void ModbusMaster::testConnection()
{
    time_t UTC_clock=time(NULL);
    //广播置标志位，启动time3定时器
    //保持寄存器addrs 0x0064 ,2*16
    // 将时间戳拆分为两个 16 位寄存器
    quint16 highWord = (UTC_clock >> 16) & 0xFFFF;
    quint16 lowWord = UTC_clock & 0xFFFF;
    // 创建数据单元
    QModbusDataUnit writeUnit(QModbusDataUnit::HoldingRegisters, 0x0064, 2);
    writeUnit.setValues({highWord, lowWord});

    for(const auto& a: Config::list_Slave_addrs)
    {
        int slave_addrs = a;
        // 发送写请求
        QModbusReply *reply = modbus_rtuMaster->sendWriteRequest(writeUnit, slave_addrs);
        if(reply && !reply->isFinished())
        {
            connect(reply,&QModbusReply::finished,[=](){
                QModbusReply *reply_1 = qobject_cast<QModbusReply *>(sender());
                if (reply_1->error() == QModbusDevice::NoError)
                {
                    Config::map_Slave_state[slave_addrs] = true;
                    qDebug()<<"slave_addrs:"<<slave_addrs<<" connection success";
                }
                reply_1->deleteLater();
            });
        }
     }
    //延迟1000ms
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, &QEventLoop::quit);
    loop.exec();
}

bool ModbusMaster::timeSynchro_andSetSingleStart()
{
    //get UTC clock
    time_t UTC_clock=time(NULL);
    //保持寄存器addrs 0x0064 ,2*16


    // 置时间戳
    quint16 highWord = (UTC_clock >> 16) & 0xFFFF;
    quint16 lowWord = UTC_clock & 0xFFFF;
    // 创建数据单元
    QModbusDataUnit writeHoldUnit(QModbusDataUnit::HoldingRegisters, 0x0064, 2);
    writeHoldUnit.setValues({highWord, lowWord});
    for(const auto& a: Config::list_Slave_addrs)
    {
        int slave_addrs = a;
        //同步等待应答，会很快
        if (auto *reply = modbus_rtuMaster->sendWriteRequest(writeHoldUnit, slave_addrs)) {
            QEventLoop loop;
            QObject::connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
            loop.exec();

            if (reply->error() == QModbusDevice::NoError) {
                Config::map_Slave_state[slave_addrs] = true;
                qDebug()<<"slave_addrs:"<<slave_addrs<<" clockSynchro OK ";
            }
            else{
                reply->deleteLater();
                return false;
            }
            reply->deleteLater();
        }
     }
    //广播置标志位，启动time3定时器
    QModbusDataUnit writeCoilUnit(QModbusDataUnit::Coils, 0x006C, 1);
    writeCoilUnit.setValues({0xff00});
    QModbusReply *reply_start = modbus_rtuMaster->sendWriteRequest(writeCoilUnit, 0x00);
    reply_start->deleteLater();     //广播没有应答
//    if(reply_start && !reply_start->isFinished())
//    {
//        connect(reply_start,&QModbusReply::finished,[=](){
//            QModbusReply *reply_1 = qobject_cast<QModbusReply *>(sender());
//            if (reply_1->error() == QModbusDevice::NoError)
//            {
//                qDebug()<<"set TIME3 timer OK";
//            }
//            else{
//                reply_1->deleteLater();
//            }
//            reply_1->deleteLater();
//        });
//    }
    return true;
}

bool is_getAllresult(bool* result1,size_t size)
{
    if(size<=0)
        return false;

    bool ret=*result1;
    for(uint i=1;i<size;++i)
    {
        ret=ret && result1[i];
    }
    return ret;
}

void ModbusMaster::loop_pollingSlaver()
{
    timeoutFlag=false;
    timer=new QTimer(this);
    timer->setInterval(timeout_ms);
    connect(timer,&QTimer::timeout,this,[this](){
        timeoutFlag=true;
    });

    for(;;)
    {
        //获取数据加入到队列
        bool* response_state_slave=new bool[Config::list_Slave_addrs.size()];//是否应答
        result_slave=new totil_bufferData();
        int poll_index=MAX_poll_count;
        while(poll_index--)
        {
            //等两个循环，如果条件满足则退出
            int i=1; //从站地址从1开始
            for(const auto& slave:Config::list_Slave_addrs)
            {
                if(response_state_slave[i] ==false) //此从站结果是否已获取
                {
                    timeoutFlag=false;
                    if(ask_dataReady(slave))
                    {
                        //读数据
                        if(read_SlaveData(slave))
                        {
                            // 置从机数据位
                            clearState_dataReady(slave);
                            response_state_slave[i]=true;
                            timeoutFlag=false;
                        }
                    }
                }
            }
            //检查从站结果是否全部获取完成
            if(is_getAllresult(response_state_slave,Config::list_Slave_addrs.size()))
            {
                queue_buf_struct.push(std::move(*result_slave));
                break;
            }
        }
        delete []response_state_slave;
        if(flag_stop)
        {
            break;
            qDebug()<<"loop_pollingSlaver quit!";
        }

    }
    timer->deleteLater();
    //轮询一次



}

bool ModbusMaster::ask_dataReady(int addrs)
{
   //线圈 0x006C   +16-addrs
    bool reply_ok=false;
    int coilsAddrs=0x006C+(16-addrs);
    QModbusDataUnit readUnit(QModbusDataUnit::Coils, 0x006C+coilsAddrs, 1);
    if (auto *reply = modbus_rtuMaster->sendReadRequest(readUnit, addrs)) {
        connect(reply, &QModbusReply::finished,this,[&reply_ok](){
            reply_ok=true;
        });
        while(!reply_ok || !timeoutFlag){       };//wait
        timeoutFlag=false;
        if( reply && reply->error() == QModbusDevice::NoError)
        {
            if(reply->result().value(0) == 0xff00)  //数据已经准备好
            {
                reply->deleteLater();
                return true;
            }
            else{
                reply->deleteLater();
                return false;
            }
        }
        reply->deleteLater();
        return false;
    }
}

bool ModbusMaster::clearState_dataReady(int addrs)
{
    bool reply_ok=false;
    int coilsAddrs=0x006C+(16-addrs);
    QModbusDataUnit writeUnit(QModbusDataUnit::Coils, 0x006C+coilsAddrs, 1);
    writeUnit.setValues({0x0000});
    if (auto *reply = modbus_rtuMaster->sendWriteRequest(writeUnit, addrs)) {
        connect(reply, &QModbusReply::finished,this,[&reply_ok](){
            reply_ok=true;
        });
        while(!reply_ok ){       };//wait
        if( reply && reply->error() == QModbusDevice::NoError)
        {
            reply->deleteLater();
            return true;
        }
        reply->deleteLater();
        return false;
    }
    return false;
}

bool ModbusMaster::read_SlaveData(int addrs)
{
    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, 0x0000, Config::size_slaveBuf[addrs-1]/2);
     bool reply_ok=false;
    if(auto *reply = modbus_rtuMaster->sendReadRequest(readUnit,addrs))
    {
        connect(reply, &QModbusReply::finished,this,[&reply_ok](){
            reply_ok=true;
        });
        while(!reply_ok){       }; //wait
        if (reply && reply->error() == QModbusDevice::NoError)
        {
            copy_toStructAddress(addrs,reply->result().values().data(),reply->result().values().size());
            reply->deleteLater();
            return true;
        }
        else{
            reply->deleteLater();
            return false;
        }

        reply->deleteLater();
    }


//    QModbusDataUnit readUnit(QModbusDataUnit::HoldingRegisters, 0x0000, Config::size_slaveBuf[i]/2);
//                    auto *reply = modbus_rtuMaster->sendReadRequest(readUnit,slave);
//                    while(!flag){       //wait      };
//                    if(reply && !reply->isFinished())
//                    {
//                        //从站结果写道对应结构体
//                        if (reply->error() == QModbusDevice::NoError) {
//                            // 获取回复的数据
//                            const QModbusDataUnit unit = reply->result();
//                            // 将数据写入 result_slave->m_buf_struct
//                            buf_struct* buf = result_slave->getBufStruct();
//                            for (int j = 0; j < unit.valueCount() && j < sizeof(buf_struct) / sizeof(uint16_t); ++j) {
//                                reinterpret_cast<uint16_t*>(buf)[j] = unit.value(j);
//                            }
//                        result_slave->m_buf_struct


}


void ModbusMaster::start_dataAcq()
{
    loopooling_thread=std::thread(&ModbusMaster::loop_pollingSlaver,this);
    loopooling_thread.detach();
}

void ModbusMaster::stop_dataAcq()
{
    flag_stop=true;
}

void ModbusMaster::copy_toStructAddress(int StructIndex,uint16_t* address,size_t copyLength)
{
    //StructIndex start 1,
    --StructIndex;
    if((uint)StructIndex < Config::list_Slave_addrs.size())
    {
        if(StructIndex ==0)
        {
            if(copyLength*2 >=(uint)Config::size_slaveBuf[StructIndex])
            std::memcpy(this->result_slave->m_buf_struct,address,copyLength);
        }
        else if(StructIndex ==1)
        {
            if(copyLength*2 >=(uint)Config::size_slaveBuf[StructIndex])
            std::memcpy(this->result_slave->m_buf_struct_02,address,copyLength);
        }
        else{

        }
    }
}


