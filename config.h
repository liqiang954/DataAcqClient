#ifndef CONFIG_H
#define CONFIG_H
#include <qstring.h>
#include <qlist.h>
#include <map>
#include <vector>



class Config
{


public:
/*************************串口 配置*********************************/
    enum CONNECTION_TYPE{
        serial_port=0
    };
    //modbus config
    static CONNECTION_TYPE conn_type;
    static QString PortName;            //串口号
    static int BaudRate;                //波特率
    static int DataBit;                 //数据位
    static QString Parity;              //校验位
    static double StopBit;              //停止位

/*************************modbus 配置*********************************/
    static std::list<int32_t> list_Slave_addrs;     //从机地址数组
    static std::map<int32_t,bool> map_Slave_state;  //从机的链接状态
    static std::vector<int> size_slaveBuf;      //每个从机结构体大小





/*************************函数*********************************/
public:
    static void config_Init();


};

typedef struct{
    struct{
        time_t clock_count;		//utc时间，在主机中转换
    }clock;
    struct{
        int16_t AccX;
        int16_t AccY;
        int16_t AccZ;
        int16_t GyroX;
        int16_t GyroY;
        int16_t GyroZ;
    }MPU6050_data;		//6050数据
    struct{
        int16_t data_hum;	//湿度
        int16_t data_temp;	//温度
    }DHT11_data;
}buf_struct;        //从机0x01的数据结构体

typedef struct{
    struct{
        time_t clock_count;		//utc时间，
    }clock;
    struct{
        int16_t throttleSite;
    }throttle;
    struct{
        int16_t pedalPower;
    }pedal;
}buf_struct_02;        //从机0x02的数据结构体

struct totil_bufferData
{
public:
    buf_struct* m_buf_struct;
    buf_struct_02* m_buf_struct_02;
public:
    totil_bufferData();
    ~totil_bufferData();

    totil_bufferData(const totil_bufferData& others)=delete;    //只保留移动构造和移动复制
    totil_bufferData& operator=(const totil_bufferData& others)=delete;
    totil_bufferData(totil_bufferData&& other) noexcept;
    totil_bufferData& operator=(totil_bufferData&& other) noexcept;

};     //总数据结构




#endif // CONFIG_H
