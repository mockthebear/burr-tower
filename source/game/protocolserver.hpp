#pragma once
#include "protocol.hpp"

class ProtocolServer: public Protocol{
    public:

        ProtocolServer(Connection *m);
        ~ProtocolServer(){};
        void Update(NetworkState *s,float dt);
        bool IsHosting(){return true;};


        int SendNewObject(NetworkObject *obj,int id=-1);
        void SendCancel(int id=-1);
        void SendProceed(int obj,int id=-1);
        void SendDestroyObject(NetworkObject *obj);
        void SendAttack(int id,Point pos);
        void SendHealth(int id,int health);
        void UpdateStorage(int field,int val,int who = -1);
    private:
        void UpdateBears();
        void UpdateEnemy();


        void ParsePacket(NetworkState *s,int,SocketMessage *msg);
        void ParseFirstPacket(SocketMessage *msg);
        void ParseBearMoved(SocketMessage *msg);
        void ParseBearAttack(SocketMessage *msg);
        void ParseRequestedTower(SocketMessage *msg);
        void ParseClientDamage(SocketMessage *msg);
        void ParseCarry(SocketMessage *msg);
        int bearCount;

        std::map<int,std::list<int>> Ownership;
};
