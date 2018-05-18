#pragma once


#include "networkobject.hpp"
#include "../engine/sprite.hpp"
#include "../engine/text.hpp"
#include "targeting.hpp"
#include <list>


class Burr: public NetworkObject{
    public:
        Burr();
        Burr(int x,int y,bool local,std::string name);

        void Update(float dt);
        void Render();
        bool IsDead();
        bool canForceRender(){return true;};
        bool canForceUpdate(){return true;};
        static std::list<Order> orders;
        void AddOrder(Order o,bool stackOrder=false);
        void ProcessMouse();
        bool NetworkClientUpdate(SocketMessage *msg);
        void ForceAttack(){attackDur = 10.0f;};
        void CreationFunction(SocketMessage *msg){
            msg->Add<int>(box.x);
            msg->Add<int>(box.y);
            msg->AddString(name.c_str());
        };
        void Kill(){health = 0;};
        void MakeCarry(int val);
    private:
        float attackDur;
        float animResetTimer;
        bool waitReset;
        std::string name;
        int health;
        Sprite spr;
        Sprite holdingObject;
        Animation ani;
        int dir;
        Text tname;
        int carry;

};
