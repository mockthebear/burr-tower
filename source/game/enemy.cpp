#include "enemy.hpp"
#include "../engine/gamebase.hpp"
#include "../engine/camera.hpp"
#include "../engine/renderhelp.hpp"
#include "../engine/light.hpp"
#include "../engine/camera.hpp"
#include "../framework/utils.hpp"

#include "gamelevel.hpp"

#include "tower.hpp"
#include "seeds.hpp"

#include "../engine/camera.hpp"


Enemy::Enemy():NetworkObject(3){
    OBJ_REGISTER(Enemy);
    health = 0;
    box.h = GRID_SIZE;
    box.w = GRID_SIZE;
    pathId = 0;

}


Enemy::~Enemy(){

}
Enemy::Enemy(int pid,int health,float speed,int tp,bool isLocal):Enemy(){
    Activate(pid,health,speed,tp);
    local = isLocal;
    InterpolationBox = box;
}

void Enemy::NotifyNewPath(){
    std::stack<Point> aux = GameLevel::pfer.Find(GameLevel::outPositions[pathId],PointInt(box.x,box.y));
    if (aux.size() > 0){
        pate = aux;
        pt.clear();
        while (aux.size() > 0){
            pt.emplace_back(aux.top());
            aux.pop();
        }
    }
}

void Enemy::Activate(int pid,int hp,float spe,int tp){
    step = 0;
    pathId = pid;
    speed = spe;
    eatMode = false;
    sprId = tp;
    myspr = utils::format("data/enemymed%d.png",tp);
    sp = Sprite(myspr.c_str());


    pp = ProgressBar(0,0,hp/10.0f);
    pp.SetSize(40);
    pp.SetMinMax(0,hp);
    sp.SetGrid(100,100);
    sp.SetFrameCount(4);
    sp.SetFrameTime(2.0f - ((spe/16.0f)) );
    sp.SetFrame(0,1);
    sp.SetScale(Point(0.5,0.5));


    health = maxHealth = hp;
    pate = GameLevel::Paths[pid];
    std::stack<Point> aux = GameLevel::Paths[pid];
    if (aux.size() > 0){
        box.x = aux.top().x;
        box.y = aux.top().y;

        while (aux.size() > 0){
            pt.emplace_back(aux.top());
            aux.pop();
        }
    }
};
void Enemy::Update(float dt){
    if (pt.size() <= 0){
        return;
    }
    pp.SetValue(health);
    sp.Update(dt);
    if (!eatMode){
        Point Where = pate.top();
        float angle = pate.top().getDirection(box.GetPos());
        float distance = pate.top().getDistance(box.GetPos());
        if (distance <= 2){
            step++;
            box.x = Where.x;
            box.y = Where.y;
            pate.pop();
            pt.erase(pt.begin());
            if (pate.size() <= 0){
                eatMode = true;
                sp.SetFrameCount(4);
                sp.SetFrameTime(1.5 );
                sp.SetFrame(0,2);
            }
        }else{
            if (!GameLevel::IsHosting()){
                Point p = box.GetCenter();
                Point p2 = InterpolationBox.GetCenter();

                //angle = p.GetDirection(p2);
                float angle2 = p.GetDirection(p2);

                if (p.GetDistance(p2) > speed*2){
                    box.x = InterpolationBox.x;
                    box.y = InterpolationBox.y;
                    NotifyNewPath();
                    if (speed * cos(angle2) < 0 ){
                        sp.SetFlip(SDL_FLIP_HORIZONTAL);
                    }else{
                        sp.SetFlip(SDL_FLIP_NONE);
                    }
                    return;
                }


            }
            box.y += speed * sin(angle)* dt;
            box.x += speed * cos(angle)* dt;

        }
        if (speed * cos(angle) < 0 ){
            sp.SetFlip(SDL_FLIP_HORIZONTAL);
        }else{
            sp.SetFlip(SDL_FLIP_NONE);
        }

        if (local){
            Rect damage(0,0,1,1);
            damage.CenterRectIn(box);
            for (auto &obj : Game::GetCurrentState().Pool.ForGroup(GameLevel::MapPoolGroup)){
                if (!obj->IsDead()) {
                    if (Collision::IsColliding(obj->box,damage) ){
                        obj->NotifyDamage(this,1);
                    }
                }
            }


        }
        Circle c(SCREEN_SIZE_W/2,(SCREEN_SIZE_H+GRID_SIZE*1.5)/2,32);

        if (Collision::IsColliding(box,c) ){
            eatMode = true;
            sp.SetFrameCount(4);
            sp.SetFrameTime(1.5 );
            sp.SetFrame(0,2);
        }

    }else{
        if (sp.IsAnimationOver()){
            if (sp.GetFrame().y == 2){
                cycles = 0;
                if (GameLevel::IsHosting()){
                    GameLevel::SetValue(4,-1,true);
                    GameLevel::wibbly.Reset(2.6f, Geometry::toRad(180.0) );
                }
                //GameLevel::GameStorage[STORAGE_FOOD]--;
                //
                sp.SetFrameTime(1.5 );
                sp.SetFrame(0,3);
                sp.SetRepeatTimes(8);
            }else{
                sp.SetFrameCount(4);
                sp.SetFrameTime(1.5 );
                sp.SetFrame(0,2);
                sp.SetRepeatTimes(1);
            }
        }
    }

    Light::GetInstance()->AddBlock(Rect(box.x-16,box.y+4,32,32),10.0f);

}

void Enemy::Render(){
    if (IsDead())
        return;

    if (Collision::IsColliding(box,g_input.GetMouse()) ){
        Point prev(box.x,box.y);
        for (auto &it : pt){
            RenderHelp::DrawLineColor(prev.x+box.w/2,prev.y+box.h/2,it.x+box.w/2,it.y+box.h/2,0,0,255,255);
            prev = it;
        }
    }

    sp.Render(box.x-GRID_SIZE/2,box.y-GRID_SIZE/2,0);
    if (Collision::IsColliding(box,g_input.GetMouse()) ){
        RenderHelp::DrawSquareColorA(box,0,255,0,255,true);
    }
    pp.Render(this);
}

bool Enemy::Is(int is){
    return is == OBJ_ENEMY || is == GetHash_internal();
}
bool Enemy::IsDead(){
    return health <= 0;
}

void Enemy::NotifyCollision(GameObject *obj){

}
void Enemy::NotifyDamage(GameObject *bj,int n){
    health -= n;
    GameLevel::MakeEffect(Point(box.x+box.w/2,box.y-8),n);
    if (IsDead()){
        //Game::GetCurrentState().Pool.AddInstance(Body(box.x,box.y,myspr));
        Seeds* T = (Seeds*)Game::GetCurrentState().Pool.AddInstance(Seeds(box.x,box.y,std::max(1.0f,maxHealth/30.0f)));
        GameLevel::instance->GetSocket()->SendNewObject(T);
        //GameLevel::SetValue(1,1,true);
        GameLevel::instance->SendDestroyObject(this);
    }else{
        GameLevel::instance->SendHealth(this,health);
    }
}
