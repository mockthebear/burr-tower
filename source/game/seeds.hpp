#pragma once


#include "networkobject.hpp"
#include "../engine/sprite.hpp"
#include "../engine/text.hpp"
#include "targeting.hpp"
#include <list>


class Seeds: public NetworkObject{
    public:
        Seeds();
        Seeds(int x,int y,int value=1);
        void Update(float dt);
        void Render();
        bool IsDead();
        bool NetworkClientUpdate(SocketMessage *msg);
        void CreationFunction(SocketMessage *msg){
            msg->Add<int>(box.x);
            msg->Add<int>(box.y);
            msg->Add<int>(value);
        };
        void Kill(){health = 0;};
        void NotifyDamage(GameObject *bj,int n);
    private:
        int health;
        Sprite spr;
        int value;

};
