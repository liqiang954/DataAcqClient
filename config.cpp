#include "config.h"


Config::CONNECTION_TYPE Config::conn_type=Config::CONNECTION_TYPE::serial_port;
QString Config::PortName="None";
int Config::BaudRate=9600;
int Config::DataBit=8;
QString Config::Parity="None Parity";
double Config::StopBit=1;


//modbus config
std::list<int32_t> Config::list_Slave_addrs={0x01,0x02};
std::map<int32_t,bool> Config::map_Slave_state;
std::vector<int> Config::size_slaveBuf;





void Config::config_Init()
{
    for(const auto& a :list_Slave_addrs)
    {
        map_Slave_state[a]=false;
    }
    size_slaveBuf.push_back(sizeof(buf_struct));
    size_slaveBuf.push_back(sizeof(buf_struct_02));
}

totil_bufferData::totil_bufferData()
{
    m_buf_struct=new buf_struct();
    m_buf_struct_02=new buf_struct_02();

}

totil_bufferData::totil_bufferData(totil_bufferData &&other) noexcept:
    m_buf_struct(other.m_buf_struct),
    m_buf_struct_02(other.m_buf_struct_02)
{
    other.m_buf_struct = nullptr;
    other.m_buf_struct_02 = nullptr;
}

totil_bufferData &totil_bufferData::operator=(totil_bufferData &&other) noexcept
{
    if (this != &other) {
            delete m_buf_struct;
            delete m_buf_struct_02;

            m_buf_struct = other.m_buf_struct;
            m_buf_struct_02 = other.m_buf_struct_02;

            other.m_buf_struct = nullptr;
            other.m_buf_struct_02 = nullptr;
        }
     return *this;
}

totil_bufferData::~totil_bufferData()
{
    delete m_buf_struct;
    delete m_buf_struct_02;
}

