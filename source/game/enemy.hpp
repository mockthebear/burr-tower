#include "../engine/object.hpp"
#include "../engine/sprite.hpp"
#include "progressbar.hpp"
#include "networkobject.hpp"

#pragma once
#include <stack>

class Enemy: public NetworkObject{
    public:
        void Update(float);
        ~Enemy();
        Enemy();
        Enemy(int pid,int health,float speed,int tp,bool local);
        void Render();
        void Activate(int pid,int health,float speed,int tp);
        bool IsDead();
        bool IsInScreen(Point& p){return true;};
        bool IsInScreen(){return true;};
        void NotifyCollision(GameObject *p);
        bool Is(int);
        void NotifyDamage(GameObject *bj,int n);
        bool isOnTop(){return false;};
        int getStep(){return pate.size();};
        void NotifyNewPath();
        int hasPerspective(){return eatMode ? 9999+box.y : 0;};
        void Kill(){health = 0;};
        virtual void CreationFunction(SocketMessage *msg){
            msg->Add<int>(pathId);
            msg->Add<int>(maxHealth);
            msg->Add<int>(health);
            msg->Add<float>(speed);
            msg->Add<int>(sprId);
            msg->Add<int>(box.x);
            msg->Add<int>(box.y);
        };
        int health,maxHealth;
    private:
        int sprId;
        std::string myspr;
        bool eatMode;
        int cycles;
        std::vector<Point> pt;
        std::stack<Point> pate;
        float speed;
        int step;
        int pathId;
        bool local;
        Sprite sp;
        ProgressBar pp;
};





