#pragma once
#include "progressthings.hpp"
#include "../engine/object.hpp"
#include "../engine/sprite.hpp"
#include "../engine/timer.hpp"


enum TargetType{
    TARGET_TYPE_NOTHING,
    TARGET_TYPE_OBJECT,
    TARGET_TYPE_POINT,
};
class Target{
    public:
        Target(){
            Reset();
        };
        void Reset(){
            type = TARGET_TYPE_NOTHING;
            obj = nullptr;
        };
        bool IsDead(){
            if (type == TARGET_TYPE_OBJECT){
                if (!obj || obj->IsDead()){
                    return true;
                }
            }
            return false;
        }
        void SetPosition(Point p){
            pos = p;
            type = TARGET_TYPE_POINT;
        };
        void SetThing(GameObject *p){
            obj = p;
            type = TARGET_TYPE_OBJECT;
        };
        Point GetPosition(){
            if (type == TARGET_TYPE_OBJECT){
                if (!obj){
                    return Point(0,0);
                }
                return obj->box.GetCenter();
            }else if (type == TARGET_TYPE_POINT){
                return pos;
            }else{
                return Point();
            }
        }
        TargetType type;
        GameObject *obj;
        Point pos;
};

enum OrderType{
    ORDER_TYPE_EMPTY,
    ORDER_TYPE_TARGET,
    ORDER_TYPE_BUILD,
    ORDER_TYPE_ATTACK,
    ORDER_TYPE_TOUCH,
};

class Order{
    public:
        Order();
        void Finish();

        static bool cancelAction;
        static void Cancel();
        static void Proceed(GameObject *obj);
        static GameObject *objproceed;

        void SetBuild(Point p,int objType,int typee){
            type = ORDER_TYPE_BUILD;
            finished = false;
            myId = orderId++;
            kId = objType;
            oType = typee;
            where = p;
        }
        bool IsAttacking();
        void FollowPosition(Point p){
            Target t;
            t.SetPosition(p);
            SetTarget(t);
        }

        void AttackObject(GameObject *p,float delay,int damage,bool canSelectNew){
            Target t;
            t.SetThing(p);
            SetTarget(t);

            coldown = Timer(delay);
            coldown.Restart();
            kId = damage;
            coldown.Enable();
            coldown.Finish();;
            state = 0;
            type = ORDER_TYPE_ATTACK;
            selectNew = canSelectNew;
        }
        void TouchObject(GameObject *p,int val){
            Target t;
            t.SetThing(p);
            SetTarget(t);
            type = ORDER_TYPE_TOUCH;
            kId = val;
        }
        void FollowObject(GameObject *p){
            Target t;
            t.SetThing(p);
            SetTarget(t);
        }

        void SetTarget(Target t){
            target = t;
            type = ORDER_TYPE_TARGET;
            myId = orderId++;
            finished = false;
        }

        void SetActor(GameObject *obj){
            actor = obj;
        }
        bool IsDead(){
            return finished;
        }
        bool Idle(){return idle;}
        Point GetPosition();

        void Update(float dt);
        void Render(Order lst,bool lines);
        Point GetDirection();
        bool ignoreSprite;
    private:
        void checkNewTarget();
        static uint32_t orderId;
        uint32_t myId;
        uint32_t state;
        int kId,oType;
        bool CheckStability();
        bool finished,idle,selectNew;
        Timer coldown;
        Point where;
        Sprite ordS;
        GameObject *actor;
        ProgressThings *other;
        OrderType type;
        Target target;
};
