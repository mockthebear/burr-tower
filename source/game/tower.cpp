#include "tower.hpp"
#include "../engine/camera.hpp"
#include "../engine/renderhelp.hpp"
#include "../engine/camera.hpp"
//#include "path.hpp"
#include "gamelevel.hpp"
#include "burr.hpp"
//#include "projectile.hpp"
#include "enemy.hpp"
//#include "effects.hpp"
#include "../engine/camera.hpp"
#include "../engine/light.hpp"
#include "../engine/gamebase.hpp"


void ShootEffect::Update(float dt){
     duration -= dt;
};
void ShootEffect::Render(Point start){
    if (isValid())
        RenderHelp::DrawLineColor(start,pos,255,0,0,100,2);
};
bool ShootEffect::isValid(){
    return duration > 0;
};


int Tower::BuildMode = 0;

Tower::Tower():ProgressThings(2){
    OBJ_REGISTER(Tower);
    health = 0;
    box.h = GRID_SIZE;
    box.w = GRID_SIZE;
    solid = false;
    target = nullptr;
    coldown = 0;
}


Tower::Tower(int x,int y,int id):Tower(){

    box.h = GRID_SIZE;
    box.w = GRID_SIZE;
    box.x = x;
    box.y = y;
    solid = false;
    target = nullptr;
    coldown = 0;

    touched = id;
    p = ProgressBar(10.0f,0);
    bear::out << "Made with id: " << id << "\n";


    sp = Sprite("data/torres.png");


    sp.SetGrid(48,48);
    sp.SetFrameCount(1);
    sp.SetFrame(0,0);
    sp.SetScale(Point(0.5,0.5));

    wibbly = WibblyWobble(Point(-0.1,0.2),0.0f,1.0f,Point(0.7,0.7));
    Range = 96 - 11 * id;
    if (id < 0){
        Range = 0;
    }
    Range = std::max(Range,0.0f);
    ball = Circle(x,y,Range);
    health = 1;
    GameLevel::UpdatePaths();
    lum = 0;

}

Tower::~Tower(){

}

void Tower::Touch(GameObject *,int par){
    touched = par;
}

void Tower::Update(float dt){

        Point p2 = box.GetCenter();
        bool wasDone = p.IsDone();
        p.Update(dt);
        if (!wasDone && p.IsDone()){
            GameLevel::pfer.AddBlock(box.x,box.y,true);
            sp.SetScale(Point(0.5,0.5));
            GameLevel::UpdatePaths();
            wibbly.Reset(2.6f, 0);
        }
        if (touched > 0 ){
            if (p.IsDone() && GameLevel::IsHosting()){
                if (target == nullptr || target->IsDead() || !Collision::IsColliding(target->box,ball)){
                    targetDistance = 0;
                    for (auto &obj : Game::GetCurrentState().Pool.ForGroup(GameLevel::EnemyPoolGroup)){
                        if (!obj->IsDead()) {
                            if (Collision::IsColliding(obj->box,ball) ){
                                Enemy *enem = (Enemy*)obj;
                                if (enem->getStep() >= targetDistance){
                                    targetDistance = enem->getStep();
                                    target = obj;
                                }
                            }
                        }
                    }
                }else{
                    if (!Collision::IsColliding(target->box,ball) ){
                        target = nullptr;
                    }else{
                        if (coldown <= 0){
                            Point p = box.GetCenter();
                            Point pDir = Point(p.GetDirection(target->box.GetCenter()));
                            pDir = pDir * 10.0f;
                            coldown = 10.0f;
                            target->NotifyDamage(this,2 + std::pow(touched,2) );
                            GameLevel::instance->SendAttack(this,target->box.GetPos());
                            AttackFunction(target->box.GetCenter());
                        }
                    }
                }
            }

        }

    lum += dt * 0.1;

    //Light::GetInstance()->AddLight(box.GetCenter(),13.0f,1.0f);

    Light::GetInstance()->AddLightM(p2.x-16,p2.y-8,90 + 8.0 * sin(lum));

    coldown -= dt;
    effc.Update(dt);
    wibbly.Update(dt,sp);

}
void Tower::AttackFunction(Point to){
    wibbly.Reset(2.6f, 0 );
    Particle *ps;
    int count = 4;
    Point3 rgb(255,255,255);
    switch (touched){
        case 1:
            count = 4;
            break;
        case 2:
            count = 8;
            rgb.y = 0;
            break;
        case 3:
            count = 16;

            break;
        case 4:
            count = 16;
            rgb.x = 0;
            rgb.y = 0;
            break;
    }
    for (int i=0;i<count;i++){
        Point dir(1,1);
        float angle = Geometry::toRad(i/(float)count * 360.0f);
        dir.x *= cos(angle) * 8.0f;
        dir.y *= sin(angle) * 8.0f;
        ps = ParticleCreator::CreateAnimatedSprite(to.x,to.y,dir.x,dir.y,"data/particle1.png",4,1.2f,0);
        if (ps){
            ps->SetDepth(9999);
            ps->GetSprite().ReBlend(rgb.x,rgb.y,rgb.z);
        }
    }

    effc = ShootEffect(to);
}


void Tower::Render(){

    if (touched){
        if (touched < 0){
            sp.SetClip(48 * (-(touched+1)),48,48,48);
        }else{
            sp.SetClip(48 * (touched-1),0,48,48);
        }
        if (!p.IsDone()){
            sp.SetScale(Point(p.GetPercent()*0.5,p.GetPercent()*0.5));
        }
        Point sc = sp.GetScale();
        Rect r2(box.x+4,box.y+8,box.w,box.h);

        sp.Render(r2.x - (sc.x * GRID_SIZE )  +GRID_SIZE/2.0,r2.y - (sc.y * GRID_SIZE)   + (GRID_SIZE/2.0) - 4,0);
        if (g_input.IsKeyDown(SDLK_d)){
            if (Collision::IsColliding(box,g_input.GetMouse()) ){
                RenderHelp::DrawSquareColorA(box,0,255,0,255,true);
            }
        }

        p.Render(this);
    }
    effc.Render(box.GetCenter());
}


bool Tower::IsDead(){
    return health <= 0;
}
void Tower::NotifyCollision(GameObject *obj){

}
void Tower::NotifyDamage(GameObject *bj,int n){

        health = 0;

        Point p = box.GetCenter();
        Point pDir;
        /*for (int i=0;i<16;i++){
            pDir = Point(Geometry::toRad(360.0 * i/16.0f ));
            pDir = pDir * 4.0f;
            //ParticleCreator::CreateCustomParticle(p.x,p.y,pDir.x,pDir.y,EffectManager::GetInstance().GetEffect(1),8,1.8,2);
        }*/
        GameLevel::pfer.AddBlock(box.x,box.y,false);
        GameLevel::UpdatePaths();
        GameLevel::instance->SendDestroyObject(this);



}
