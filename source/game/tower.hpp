#include "../engine/object.hpp"
#include "../engine/sprite.hpp"
#include "state.hpp"

#pragma once
#include <stack>
#include "progressthings.hpp"
class ShootEffect{
    public:
    ShootEffect():pos(),duration(0.0f){};
    ShootEffect(Point p):ShootEffect(){pos = p;duration = 2.0f;};

    void Update(float dt);
    void Render(Point start);
    bool isValid();

    Point pos;
    float duration;

};

class Tower: public ProgressThings{
    public:
        void Update(float);
        static int BuildMode;
        ~Tower();
        Tower();
        Tower(int x,int y,int id);
        void Render();
        bool IsDead();
        bool IsInScreen(Point& p){return true;};
        bool IsInScreen(){return true;};
        void NotifyCollision(GameObject *p);
        void NotifyDamage(GameObject *bj,int n);
        int health;
        float GetRange(){ return Range;};
        void Touch(GameObject *,int par);
        int GetType(){ return touched; };
        void AttackFunction(Point pos);
        void Kill(){NotifyDamage(nullptr,999);};
        virtual void CreationFunction(SocketMessage *msg){
            msg->Add<int>(box.x);
            msg->Add<int>(box.y);
            msg->Add<int>(touched);
        };
    private:
        ShootEffect effc;
        Sprite sp;
        int touched;
        GameObject *target;
        float targetDistance;
        float coldown;
        float lum;
        float Range;
        WibblyWobble wibbly;
};





