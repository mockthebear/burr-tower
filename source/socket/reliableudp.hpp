#include "socketdef.hpp"
#ifndef DISABLE_SOCKET
#pragma once
#include <enet/enet.h>
#include <queue>
#include <list>

class ReliableUdpClient: public SocketClient{
    public:
        ReliableUdpClient():SocketClient(),client(nullptr),peer(nullptr){};
        ~ReliableUdpClient();
        bool IsConnected();

        void Update(float dt);

        bool Send(SocketMessage *msg,int pid=-1);
        bool Send(SocketMessage msg,int pid=-1){
            static SocketMessage svd;
            svd = msg;
            return Send(&svd);
        };
        bool ReceiveBytes(SocketMessage *msg,uint16_t amount,int pid=-1);
        bool Receive(SocketMessage *msg,int pid=-1);

        bool Start();
        bool Connect(std::string addr,uint16_t port,int wait=1000);
        static bool StartedEnet;

        static void SetSendMode(ENetPacketFlag mode){ sendMode = mode;};

        void Close();
    private:

        static ENetPacketFlag sendMode;
        ENetHost * client;
        ENetPeer *peer;
        ENetEvent event;
        std::queue<SocketMessage> m_messages;
};


class ReliableUdpServer: public SocketHost{
    public:
        ~ReliableUdpServer();
        ReliableUdpServer():SocketHost(),server(nullptr){for (int i=0;i<100;i++) peers[i] = nullptr;};
        bool Bind(uint16_t port);
        void Update(float dt);
        bool Receive(SocketMessage *msg,int pid);
        bool ReceiveBytes(SocketMessage *msg,uint16_t amount,int pid=-1){
            return Receive(msg,pid);
        };
        bool Send(SocketMessage *msg,int pid);
        void Close();

        static void SetSendMode(ENetPacketFlag mode){ sendMode = mode;};
    private:
        static ENetPacketFlag sendMode;
        ENetHost *server;
        ENetEvent event;
        ENetPeer *peers[100];
};

#endif
