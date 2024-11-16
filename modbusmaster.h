#ifndef MODBUSMASTER_H
#define MODBUSMASTER_H

#include <QModbusRtuSerialMaster>
#include <qtimer.h>
#include "const.h"
#include <thread>

class ModbusMaster:public QObject
{
private:
    QModbusRtuSerialMaster* modbus_rtuMaster;
    std::atomic_bool flag_stop;     //控制轮询线程退出循环
    const int timeout_ms=12;
    const int MAX_poll_count= 2;

    totil_bufferData* result_slave; //获取到的结果
    bool timeoutFlag;
    QTimer* timer;

    std::thread loopooling_thread;
public:
    ModbusMaster();
    ~ModbusMaster();
    void ModbusMaster_init();

    void testConnection();
    bool timeSynchro_andSetSingleStart();
    void loop_pollingSlaver();         //轮询
    bool ask_dataReady(int addrs);
    bool clearState_dataReady(int addrs);
    bool read_SlaveData(int addrs);
    void copy_toStructAddress(int StructIndex,uint16_t* address,size_t copyLength);

    void start_dataAcq();
    void stop_dataAcq();





};

#endif // MODBUSMASTER_H
