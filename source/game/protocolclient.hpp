#pragma once
#include "protocol.hpp"

class ProtocolClient: public Protocol{
    public:
        ProtocolClient(Connection *m):Protocol(m){};
        ~ProtocolClient(){};
        void Update(NetworkState *s,float dt);
        bool IsHosting(){return false;};

        void RequestTower(int x,int y,int type);



    private:
        void ParsePacket(NetworkState *s,int,SocketMessage *msg);
        void ParseFirstPacket(SocketMessage *msg);

        void ParseBearMove(SocketMessage *msg);
        void ParseStorageChange(SocketMessage *msg);


        bool CreateObject(SocketMessage *msg);
        void ParseCancel(SocketMessage *msg);
        void ParseProceed(SocketMessage *msg);
        void ParseDestroy(SocketMessage *msg);
        void ParseAttack(SocketMessage *msg);
        void ParseBearAttack(SocketMessage *msg);
        void ParseChangeHealth(SocketMessage *msg);
        void ParseCarry(SocketMessage *msg);
        void ParseEffect(SocketMessage *msg);

        int m_myPid;
};

