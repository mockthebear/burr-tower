#include "burr.hpp"
#include "gamelevel.hpp"
#include "tower.hpp"
#include "../input/inputmanager.hpp"
#include "../engine/gamebase.hpp"
#include "../engine/collision.hpp"
#include "../engine/light.hpp"

std::list<Order> Burr::orders;

Burr::Burr():NetworkObject(1){
    OBJ_REGISTER(Burr);
    health = 0;
}
Burr::Burr(int x,int y,bool local,std::string nm):NetworkObject(1){
    OBJ_REGISTER(Burr);
    health = 10;
    name = nm;
    m_local = local;
    dir = 0;
    spr = Sprite("data/burr.png");

    spr.SetScale(Point(0.5,0.5));
    ani.SetGridSize(100,100);
    box.x = x;
    box.y = y;
    box.w = box.h = 32;
    InterpolationBox = box;
    animResetTimer = 0;
    attackDur = 0;
    tname = Text(nm,16,{90,57,38,255});
    carry = 0;

    holdingObject = Sprite("data/drops.png");
    holdingObject.SetScale(Point(0.25,0.25));
    holdingObject.SetClip(0,0,100,100);
}


void Burr::AddOrder(Order o,bool stackOrder){
    if ( (g_input.IsKeyDown(SDLK_LSHIFT) && g_input.IsKeyDown(SDLK_RSHIFT) && !stackOrder) ){
        while (orders.size() > 0){
            orders.front().Finish();
            orders.pop_front();
        }
    }
    orders.push_back(o);
}

void Burr::MakeCarry(int val){
    carry = val;
    ani.SetAnimation(3,1,1.0f);
};

bool Burr::NetworkClientUpdate(SocketMessage *msg){
    msg->Add<uint8_t>(0xA3);
    msg->Add<int>(box.x);
    msg->Add<int>(box.y);
    return true;
}

void Burr::Update(float dt){
    ani.Update(dt);
    animResetTimer -= dt;
    Point speed;

    //Light::GetInstance()->AddLight(box.GetCenter()+Point(4,4),50.0f,1.0f);
    bool onAttack = false;
    if (m_local){
        if (carry != 0){
            Tower::BuildMode = 0;
        }
        if (orders.size() > 0){
            bool wasAttacking = orders.front().IsAttacking();
            orders.front().Update(dt);

            speed = orders.front().GetDirection();
            speed = speed * 18.0f;


            onAttack = orders.front().IsAttacking();
            if (!wasAttacking && onAttack){
                GameLevel::instance->NotifyAttack(this);
            }

            if (orders.front().IsDead()){
                orders.front().Finish();
                orders.pop_front();

            }
        }


        if (speed.x != 0 || speed.y != 0){
            if (carry == 0){
                if (ani.sprY != 1){
                    ani.SetAnimation(1,8,1.0f);
                }
            }else{
                if (ani.sprY != 4){
                    ani.SetAnimation(4,8,1.0f);
                }
            }
        }else{
            if (onAttack){
                if (ani.sprY != 2){
                    ani.SetAnimation(2,4,1.0f);
                }
            }else{
                if (carry == 0){
                    ani.SetAnimation(0,1,1.0f);
                }else{
                    ani.SetAnimation(3,1,1.0f);
                }
            }
        }



    }else{
        attackDur -= dt;
        if (attackDur > 0){
            onAttack = true;
        }
        Point p = box.GetCenter();
        Point p2 = InterpolationBox.GetCenter();

        speed = p.GetDirection(p2);

        if (p.GetDistance(p2) >= 24*dt){
            speed.x += speed.x * 24;
            speed.y += speed.y * 24;
        }else{
            speed = Point(0.0f,0.0f);
        }
        if (p.GetDistance(p2) > 18.0f * 2.0f){
            box.x = InterpolationBox.x;
            box.y = InterpolationBox.y;
            return;
        }




        if (speed.x != 0 || speed.y != 0){
            if (carry == 0){
                if (ani.sprY != 1){
                    ani.SetAnimation(1,8,1.0f);
                }
            }else{
                if (ani.sprY != 4){
                    ani.SetAnimation(4,8,1.0f);
                }
            }
            waitReset = false;
        }else{
            if (onAttack){
                if (ani.sprY != 2){
                    ani.SetAnimation(2,4,1.0f);
                }
            }else if (ani.sprY != 0){
                if (!waitReset){
                    waitReset = true;
                    animResetTimer = 1.0f;
                }else{
                    if (animResetTimer <= 0){
                        if (carry == 0){
                            ani.SetAnimation(0,1,1.0f);
                        }else{
                            ani.SetAnimation(3,1,1.0f);
                        }
                        waitReset = false;
                    }
                }
            }

        }

    }

    if (speed.x > 0){
        dir = 0;
    }else if (speed.x < 0){
        dir = 1;
    }

    box.x += speed.x * dt;
    box.y += speed.y * dt;
    if (m_local){
        ProcessMouse();
    }
    Circle c(SCREEN_SIZE_W/2,(SCREEN_SIZE_H+GRID_SIZE*2)/2,40);
    if (Collision::IsColliding(box,c) && carry > 0){
        if (GameLevel::IsHosting()){
            GameLevel::SetValue(1,carry,true);
        }
        carry = 0;

    }
    Light::GetInstance()->AddLightM(box.GetCenter().x,box.GetCenter().y,110);

}
void Burr::Render(){
    ani.Render(box.x,box.y,spr,dir);
    if (carry > 0){
        holdingObject.Render(Point(box.x+6,box.y-16),0);
    }

    tname.Render(PointInt(box.x+box.w/2 + 10,box.y -2),TEXT_RENDER_CENTER);

}
bool Burr::IsDead(){
    return health <= 0;
}


void Burr::ProcessMouse(){
    Rect usefulArea(0,0,SCREEN_SIZE_W,SCREEN_SIZE_H-80);
    if (g_input.IsHalted()){
        return;
    }
    if( InputManager::GetInstance().KeyPress(SDLK_ESCAPE) ) {
        if (Tower::BuildMode > 0){
            Tower::BuildMode = 0;
            return;
        }
        while (orders.size() > 0){
            orders.front().Finish();
            orders.pop_front();
        }
        return;
    }
    if (usefulArea.IsInside(InputManager::GetInstance().GetMouse())){

        if( InputManager::GetInstance().MousePress(3)){
            while (orders.size() > 0){
                orders.front().Finish();
                orders.pop_front();
            }
        }

        if( InputManager::GetInstance().MousePress(1)){



            Order ord;
            ord.SetActor(this);

            Point p = InputManager::GetInstance().GetMouse();
            Rect c(p.x,p.y,2,2);
            if (carry == 0){
                if (g_input.IsKeyDown(SDLK_d)){
                    for (auto &obj : Game::GetCurrentState().Pool.ForGroup(GameLevel::MapPoolGroup)){
                        if (!obj->IsDead()) {
                            if (Collision::IsColliding(obj->box,c) ){
                                ord.AttackObject(obj,5.0f,4,false);
                                AddOrder(ord);
                                return;
                            }
                        }
                    }
                }
                for (auto &obj : Game::GetCurrentState().Pool.ForGroup(GameLevel::EnemyPoolGroup)){
                    if (!obj->IsDead()) {
                        if (Collision::IsColliding(obj->box,c) ){
                            ord.AttackObject(obj,5.0f,4,true);
                            AddOrder(ord);
                            return;
                        }
                    }
                }
            }


            Point wherePos = InputManager::GetInstance().GetMouse();

            wherePos.GridSnap(GRID_SIZE);
            wherePos.x += GRID_SIZE/2;
            wherePos.y += GRID_SIZE/2;
            ord.FollowPosition(wherePos);
            ord.ignoreSprite = true;
            AddOrder(ord);
            if (carry == 0){
                for (auto &obj : Game::GetCurrentState().Pool.ForGroup(GameLevel::ItemPoolGroup)){
                    if (!obj->IsDead()) {
                        if (Collision::IsColliding(obj->box,c) ){
                            ord.TouchObject(obj,1);
                            AddOrder(ord);
                            return;
                        }
                    }
                }


                if (Tower::BuildMode > 0 && Tower::BuildMode < 10){
                    ord.ignoreSprite = false;

                    wherePos = InputManager::GetInstance().GetMouse();

                    ord.SetBuild(wherePos,TYPEOF(Tower),Tower::BuildMode);

                    AddOrder(ord,true);
                    Tower::BuildMode = 0;
                }else if (Tower::BuildMode == 10){
                    ord.ignoreSprite = false;

                    wherePos = InputManager::GetInstance().GetMouse();

                    ord.SetBuild(wherePos,TYPEOF(Tower),-(1+rand()%3) );

                    AddOrder(ord,true);
                    Tower::BuildMode = 0;
                }
            }


        }
    }
}

