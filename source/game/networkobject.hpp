#include "../engine/object.hpp"
#include "../socket/socketdef.hpp"


#pragma once


class NetworkObject: public GameObject{
    public:
        NetworkObject(int tp){m_type = tp;};
        Rect InterpolationBox;
        virtual void CreationFunction(SocketMessage *msg){
            msg->Add<int>(box.x);
            msg->Add<int>(box.y);
        };
        virtual bool IsLocal(){return m_local;};
        virtual bool NetworkClientUpdate(SocketMessage *msg){return false;};
        int GetObjType(){ return m_type; };
    protected:
        int m_type;
        bool m_local;
};
