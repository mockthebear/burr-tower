#include "targeting.hpp"
#include "gamelevel.hpp"
#include "tower.hpp"
#include "enemy.hpp"
#include "../engine/gamebase.hpp"
#include "../engine/renderhelp.hpp"
#include "../engine/gamebase.hpp"

uint32_t Order::orderId = 1;


bool Order::cancelAction = false;
void Order::Cancel(){
    cancelAction = true;

};


void Order::Proceed(GameObject *obj){
    objproceed = obj;

}
GameObject *Order::objproceed = nullptr;


void Order::Finish(){
    if (type == ORDER_TYPE_BUILD){
        if (other){
            if (!other->p.IsDone()){
                //other->NotifyDamage(actor,1);
            }
        }
    }
}
Order::Order(){
    type = ORDER_TYPE_EMPTY;
    finished = true;
    actor = other = nullptr;
    myId = 0;
    state = 0;
    idle = false;
    ordS = Sprite("data/cursor.png");
    ordS.SetClip(0,0,24,24);
    ignoreSprite = false;
    selectNew = false;
}

void Order::checkNewTarget(){
    for (auto &obj : Game::GetCurrentState().Pool.ForGroup(GameLevel::EnemyPoolGroup)){
        if (!obj->IsDead()) {
            if (obj->Is(TYPEOF(Enemy))){
               AttackObject(obj,coldown.GetDuration(),kId,true);
               return;
            }
        }
    }
    finished = true;
}

bool Order::IsAttacking(){
    if (type == ORDER_TYPE_ATTACK){
        Point pos = target.GetPosition();
        if (state >= 1){
            return !coldown.HasFinished();
        }
    }
    return false;
}

void Order::Update(float dt){
    if (!CheckStability())
        return;
    if (type == ORDER_TYPE_TARGET || type == ORDER_TYPE_ATTACK){
        Point pos = target.GetPosition();
        if (type == ORDER_TYPE_ATTACK){
            coldown.Update(dt);
            idle = !coldown.HasFinished();
            if ( (coldown.HasFinished() && pos.GetDistance(actor->box.GetCenter()) < sqrt(actor->box.w*actor->box.h)) || target.obj->IsDead()){
                if (state == 0){
                    if ((coldown.HasFinished() && pos.GetDistance(actor->box.GetCenter()) < sqrt(actor->box.w*actor->box.h))){
                        coldown.Restart();
                        state = 1;
                        return;
                    }
                }
                state++;

                if (coldown.HasFinished()){
                    coldown.Restart();
                    if (target.obj->IsDead()){
                        if (selectNew && Burr::orders.size() <= 1)
                            checkNewTarget();
                        else
                            finished = true;
                    }else{
                        if (GameLevel::IsHosting()){
                            target.obj->NotifyDamage(actor,kId);
                        }else{
                            GameLevel::instance->NotifyDamage((NetworkObject*)target.obj,kId);
                        }
                        if (target.obj->IsDead()){
                            if (selectNew && Burr::orders.size() <= 1)
                                checkNewTarget();
                            else
                                finished = true;
                        }
                    }

                }
            }

        }else{
            if (pos.GetDistance(actor->box.GetCenter()) < sqrt(actor->box.w*actor->box.h) ){
                finished = true;
            }
        }
    }else if (type == ORDER_TYPE_BUILD){
        if (state == 0){
            if (kId == TYPEOF(Tower)){
                where.GridSnap(GRID_SIZE);
                GameLevel::instance->RequestMakeTower(where.x,where.y,oType);
                other = nullptr;
            }
            idle = true;
            state = 1;
        }else{
            if (cancelAction){
                finished = true;
            }
            if (objproceed){
                other = (ProgressThings*)objproceed;
                objproceed = nullptr;
            }
            if (other){
                if (other->IsDead()){
                    finished = true;
                    return;
                }
                if (other->p.IsDone()){
                    finished = true;
                }
            }
        }

    }else if (type == ORDER_TYPE_TOUCH){
        target.obj->NotifyDamage(actor,0);
        finished = true;
    }
}

void Order::Render(Order lst,bool linesa){
    if (!CheckStability())
        return;
    if (type == ORDER_TYPE_TARGET || type == ORDER_TYPE_ATTACK){

        Point pos;
        Point pos2;
        if (lst.type == ORDER_TYPE_EMPTY || lst.myId == myId){
            //means main pos is the actor one
            pos2 = actor->box.GetPos();
            pos2.y += actor->box.h;
            pos2.x += actor->box.w/2;
            pos = target.GetPosition();
        }else if (lst.type == ORDER_TYPE_BUILD || lst.type == ORDER_TYPE_TOUCH){
            pos2 = lst.GetPosition();
            pos2.GridSnap(GRID_SIZE);
            pos2.x += GRID_SIZE/2;
            pos2.y += GRID_SIZE/2;
            pos = GetPosition();
        }else{
            pos2 = lst.GetPosition();
            pos = target.GetPosition();

        }
        if (target.IsDead()){
            return;
        }

        RenderHelp::DrawLineColor(pos2.x,pos2.y,pos.x,pos.y,200,200,200,200);


    }else if (type == ORDER_TYPE_BUILD || type == ORDER_TYPE_TOUCH){

        float dist = actor->box.GetCenter().GetDistance(GetPosition());
        if (dist < 32){
            float alp = 1.0-(dist/32.0f);
            ordS.SetAlpha(255 * alp);
        }else{
            ordS.SetAlpha(255);
        }
        Point auxwhere = GetPosition();


        if (type == ORDER_TYPE_TOUCH){
            ordS.SetClip(24 ,0,24,24);
        }else{
            auxwhere.GridSnap(GRID_SIZE);
            ordS.SetClip(48 ,0,24,24);
        }
        if (state == 0)
            ordS.Render(auxwhere.x,auxwhere.y,0);
        //RenderHelp::DrawSquareColorA(auxwhere.x,auxwhere.y,GRID_SIZE,GRID_SIZE,0,0,0,255,true);
    }



}

Point Order::GetPosition(){
    if (type == ORDER_TYPE_TARGET || type == ORDER_TYPE_ATTACK || type == ORDER_TYPE_TOUCH){
        return target.GetPosition();
    }else{
        return where;
    }
}


Point Order::GetDirection(){
    if (!CheckStability())
        return Point();
    if (type == ORDER_TYPE_TARGET){
        Point pos = target.GetPosition();
        float direction = pos.getDirection(actor->box.GetCenter());
        return Point(cos(direction) ,sin(direction));
    }else if (type == ORDER_TYPE_ATTACK){
        Point pos = target.GetPosition();
        Point dir(pos.getDirection(actor->box.GetCenter()));
        if (coldown.HasFinished() && pos.GetDistance(actor->box.GetCenter()) > sqrt(actor->box.w*actor->box.h)){
            return dir;
        }else{
            return Point();
        }
    }
    return Point();
}

bool Order::CheckStability(){
    if (!actor){
        return false;
    }
    if (actor->IsDead()){
        actor = nullptr;
        return false;
    }
    if (type == ORDER_TYPE_TOUCH){
        if (target.obj == nullptr || target.obj->IsDead()){
            finished = true;
            return false;
        }
    }
    return true;
}
