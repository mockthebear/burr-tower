#include "seeds.hpp"
#include "burr.hpp"
#include "gamelevel.hpp"
#include "../engine/renderhelp.hpp"
#include "../input/inputmanager.hpp"



Seeds::Seeds():NetworkObject(4){
    OBJ_REGISTER(Seeds);
    health = 0;
}
Seeds::Seeds(int x,int y,int value):NetworkObject(4){
    OBJ_REGISTER(Seeds);
    health = 10;

    spr = Sprite("data/drops.png");
    spr.SetScale(Point(0.25,0.25));
    spr.SetClip(0,0,100,100);

    box.x = x;
    box.y = y;
    box.w = box.h = 24;
    InterpolationBox = box;
    this->value = value;
}

void Seeds::Update(float dt){

}
void Seeds::Render(){
    spr.Render(box.GetPos());
    if (Collision::IsColliding(box,g_input.GetMouse()) ){
        RenderHelp::DrawSquareColorA(box,0,0,0,255,true);
    }
}
bool Seeds::IsDead(){
    return health <= 0;
}
bool Seeds::NetworkClientUpdate(SocketMessage *msg){
    return false;
}

void Seeds::NotifyDamage(GameObject *bj,int n){

    if (bj && bj->Is(TYPEOF(Burr))){
        GameLevel::instance->SendCarry(static_cast<NetworkObject*>(bj),value,this);
        if (GameLevel::IsHosting()){
            static_cast<Burr*>(bj)->MakeCarry(value);
            GameLevel::instance->SendDestroyObject(this);
            health = 0;
        }

    }
}
