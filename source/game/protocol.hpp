#pragma once
#include "../socket/socketdef.hpp"
#include "state.hpp"

class Protocol{
    public:
        Protocol(Connection *m){
            m_sock = m;
            updateInterval = 0.1f;
            updateCounter = 0;
        };
        virtual ~Protocol(){};
        virtual void SetConnection(Connection *c){m_sock = c;};
        virtual void Update(NetworkState *s,float dt)=0;
        virtual bool IsHosting() = 0;
        virtual void Send(SocketMessage *s,int pid=-1){
            m_sock->Send(s,pid);
        }

        virtual void RequestTower(int x,int y,int type){};
        virtual int SendNewObject(NetworkObject *obj,int id=-1){return 0;};
        virtual void SendCancel(int id=-1){};
        virtual void SendProceed(int obj,int id=-1){};
        virtual void SendDestroyObject(NetworkObject *obj){};
        virtual void SendHealth(int id,int hp){};
        virtual void SendAttack(int id,Point pos){};
        virtual void UpdateStorage(int field,int val,int who=-1){};

    protected:



        NetworkState *currentState;
        int m_Pid;
        int GetPid(){return m_Pid;};
        NetworkState *GetState(){return currentState;};


        float updateInterval;
        float updateCounter;
        Connection *m_sock;
        friend class State;
        friend class NetworkState;

};
