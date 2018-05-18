#include "socketdef.hpp"

#ifndef DISABLE_SOCKET

class TcpClient: public SocketClient{
    public:
        TcpClient():SocketClient(){};

        bool IsConnected();
        void Update(float dt);
        bool Send(SocketMessage *msg,int pid=-1);
        bool ReceiveBytes(SocketMessage *msg,uint16_t amount,int pid=-1);
        bool Receive(SocketMessage *msg,int pid=-1);

        bool Connect(std::string addr,uint16_t port,int duration=0);
};

#endif
