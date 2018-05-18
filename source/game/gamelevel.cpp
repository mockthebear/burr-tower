#include "title.hpp"
#include <sstream>
#include <cmath>

#include "gamelevel.hpp"
#include "tower.hpp"
#include "enemy.hpp"
#include "seeds.hpp"
#include "../engine/text.hpp"
#include "../engine/light.hpp"


#include "../luasystem/luacaller.hpp"
#include "../luasystem/luatools.hpp"
#include "../engine/renderhelp.hpp"

int GameLevel::GameStorage[255];
WibblyWobble GameLevel::wibbly;
PathFind GameLevel::pfer;
int GameLevel::ControllerId = 0;
bool GameLevel::updateNeeded = false;
GameLevel *GameLevel::instance = nullptr;

bool GameLevel::IsHosting(){
    return ishosting;
}
bool GameLevel::ishosting = false;


int GameLevel::AttackablePoolGroup = 0;
int GameLevel::EnemyPoolGroup = 0;
int GameLevel::MapPoolGroup = 0;
int GameLevel::BearPoolGroup = 0;
int GameLevel::ItemPoolGroup = 0;


std::stack<Point> GameLevel::LastValidPaths[4];
std::stack<Point> GameLevel::Paths[4];

Point GameLevel::MakerPosition[4] = {
    Point(0,SCREEN_SIZE_H/2),
    Point(SCREEN_SIZE_W,SCREEN_SIZE_H/2),

    Point(SCREEN_SIZE_W,0),



    Point(0,SCREEN_SIZE_H - 80),

};

Point GameLevel::outPositions[4] = {
    Point(SCREEN_SIZE_W/2,SCREEN_SIZE_H/2),
    Point(SCREEN_SIZE_W/2,SCREEN_SIZE_H/2),
    Point(SCREEN_SIZE_W/2,SCREEN_SIZE_H/2),
    Point(SCREEN_SIZE_W/2,SCREEN_SIZE_H/2)
};

void GameLevel::UpdatePaths(){
    GameLevel::updateNeeded = true;
}

GameLevel::GameLevel(Protocol *p,std::string name){
    for (int i=0;i<255;i++){
        GameStorage[i] = 0;
    }
    myName = name;
    m_sock = p;
    requestQuit = requestDelete = false;
    instance = this;
    Camera::Initiate(Rect(0,0,SCREEN_SIZE_W,SCREEN_SIZE_H),128,200);
    Camera::speed = 0;

    std::vector<PoolId> attackableGroup;
    std::vector<PoolId> enemyGroup;
    std::vector<PoolId> mapGroup;
    std::vector<PoolId> itemGroup;
    std::vector<PoolId> bearGroup;

    ParticlePool = new SPP<Particle>(1000);

    sdds.Compile("data/shade.glvs","data/lum.glfs");
    invrt.Compile("data/shade.glvs","data/inv.glfs");
    tr.Generate(SCREEN_SIZE_W,SCREEN_SIZE_H);


    bearGroup.emplace_back(Pool.Register<Burr>(8));
    mapGroup.emplace_back(Pool.Register<Tower>(250));
    enemyGroup.emplace_back(Pool.Register<Enemy>(100));
    itemGroup.emplace_back(Pool.Register<Seeds>(100));


    BearPoolGroup = Pool.CreatePoolGroup(bearGroup);
    MapPoolGroup = Pool.CreatePoolGroup(mapGroup);
    EnemyPoolGroup = Pool.CreatePoolGroup(enemyGroup);
    ItemPoolGroup = Pool.CreatePoolGroup(itemGroup);
    delayKek = 0;


}
/*
char get_line_intersection(Point p0, Point p1, Point p2, Point p3){


    Point s1,s2;

    //s1.x = p1_x - p0_x;
    s1.x = p1.x - p0.x;
    s1.y = p1.y - p0.y;
    s2.x = p3.x - p2.x;
    s2.y = p3.y - p2.y;

    float s, t;
    s = (-s1.y * (p0.x - p2.x) + s1.x * (p0.y - p2.y)) / (-s2.x * s1.y + s1.x * s2.y);
    t = ( s2.x * (p0.y - p2.y) - s2.y * (p0.x - p2.x)) / (-s2.x * s1.y + s1.x * s2.y);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        return 1;
    }

    return 0; // No collision
}*/

void GameLevel::Begin(){
    wibbly = WibblyWobble(Point(0.2,0.2),0,1.0f);

    bg = Sprite("data/background.png");
    lg = LineGraph(Point(400,150),80);
    bg.SetCenter(Point(0,0));
    bg.SetScale(Point(0.9f,0.9f));

    twr = Sprite("data/torres.png");
    twr.SetScale(Point(0.5,0.5));

    foodie = Sprite("data/food.png");
    foodie.SetScale(Point(0.9,0.9));

    //Light::Startup(Point(SCREEN_SIZE_W,SCREEN_SIZE_H),Point(0.25,0.25));

    GameStorage[1] = 10;
    GameStorage[4] = 20;

    if (m_sock->IsHosting()){
        ishosting = true;
        ControllerId = cmap.RegisterCreature((NetworkObject*)Pool.AddInstance(Burr(32,32,true,myName)));
    }else{
        ishosting = false;
        //Send greet packet
        SocketMessage msg;
        msg.Add<uint8_t>(0x01);
        msg.Add<uint8_t>(0x02);
        msg.AddString(myName.c_str());
        ReliableUdpClient::SetSendMode(ENET_PACKET_FLAG_RELIABLE);
        m_sock->Send(&msg,-1);
    }

    pfer.Close();
    pfer = PathFind(SCREEN_SIZE_W+GRID_SIZE,720,4,GRID_SIZE);
    internalUpdatePaths();
    LuaCaller::LoadFile(LuaManager::L,"lua/level.lua");
    LuaCaller::Pcall(LuaManager::L);

    LuaCaller::CallGlobalField(LuaManager::L,"onLoad");


    tar = new TargetTexture();
    tar->Generate(SCREEN_SIZE_W,SCREEN_SIZE_H);
    Point lpos(100,100);


    /*for (int x=0;x<200;x++){
        for (int y=0;y<200;y++){
            Point dis(x,y);
            bool isFree = true;
            for (auto &it : lines){
                if (get_line_intersection(lpos,dis,Point(it.x,it.y),Point(it.w,it.h) ) ){
                    isFree = false;
                    break;
                }
            }
            float dist = 1;
            if (isFree){
                dist = dis.getDistance(lpos)/100.0f;
                dist = std::max(0.0f,dist);
                dist = std::min(1.0f,dist);
            }
            sm->At(x,y) = RenderHelp::FormatRGBA(0,0,0,255*(dist));

        }
    }
    */

    ThreadPool::GetInstance().KillThreads();
    ThreadPool::GetInstance().Begin(4);

    //Light::GetInstance()->StartLights( Point(SCREEN_SIZE_W,SCREEN_SIZE_H) ,Point(64,64) ,4,2.5,90);
    //Light::GetInstance()->SetLightRaysCount(360);

    Light::GetInstance()->StartLights( Point(SCREEN_SIZE_W,SCREEN_SIZE_H) ,Point(160,160) ,4,6.5,90);
    Light::GetInstance()->SetLightRaysCount(360);

}

GameLevel::~GameLevel(){
    Pool.ErasePools();
    delete m_sock;
    delete ParticlePool;

}


void GameLevel::Update(float dt){
    Light::GetInstance()->Update(dt,LIGHT_BEGIN);
    wibbly.Update(dt,foodie);
    foodie.SetCenter(Point(34,34));
    Input();
    //Light::GetInstance()->AddLight(g_input.GetMouse(),3000.0f,1.0f);

    Light::GetInstance()->AddLightM(g_input.GetMouse().x,g_input.GetMouse().y,255);

    if (updateNeeded){
        updateNeeded = false;
        internalUpdatePaths();
    }
    ParticlePool->UpdateInstances(dt);


    Pool.Update(dt);
    Map.clear();
    Pool.PreRender(Map);
    ParticlePool->PreRender(Map);

    if (g_input.KeyPress(SDLK_1)){
        Tower::BuildMode = 1;
    }
    if (g_input.KeyPress(SDLK_2)){
        Tower::BuildMode = 2;
    }
    if (g_input.KeyPress(SDLK_3)){
        Tower::BuildMode = 3;
    }
    if (g_input.KeyPress(SDLK_4)){
        Tower::BuildMode = 4;
    }
    if (g_input.KeyPress(SDLK_b)){
        Tower::BuildMode = 10;
    }


    Camera::Update(dt);

    UpdateWindowses(dt);

    if (g_input.KeyPress(SDLK_e)){
        MakeEnemy();
    }

    Light::GetInstance()->Update(dt,LIGHT_SHADE);


    delayKek -= dt;
    if (delayKek <= 0){
        lg.AddData(ScreenManager::GetInstance().GetFps());
        delayKek = 5.0f;
    }




    m_sock->Update(this,dt);

}


int get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y,
    float p2_x, float p2_y, float p3_x, float p3_y, float *i_x, float *i_y)
{
    float s1_x, s1_y, s2_x, s2_y;
    s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
    s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

    float s, t;
    s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
    t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        // Collision detected
        if (i_x != NULL)
            *i_x = p0_x + (t * s1_x);
        if (i_y != NULL)
            *i_y = p0_y + (t * s1_y);
        return 1;
    }

    return 0; // No collision
}

void GameLevel::Render(){
    Light::GetInstance()->Update(0,LIGHT_REDUCE);
    RenderHelp::DrawSquareColorA(0,0,SCREEN_SIZE_W,SCREEN_SIZE_H,255,255,255,255);
    bg.Render(0,0);

    if (Burr::orders.size() > 0){
        Order last;
        for (auto &it : Burr::orders){
            it.Render(last,false);
            last = it;
        }
    }


    Rect r(0,0,68,68);
    r.CenterRectIn(Point(SCREEN_SIZE_W/2,(SCREEN_SIZE_H+GRID_SIZE*2)/2));
    int damageLevel =  (GameStorage[4]/5);
    foodie.SetClip((damageLevel-1)*68,0,68,68);

    foodie.Render(r.GetPos(),0);
    RenderInstances();


    for (auto &obj : Pool.ForGroup(MapPoolGroup)){
        if (Collision::IsColliding(obj->box,g_input.GetMouse()) ){
            float range = static_cast<Tower*>(obj)->GetRange();
            if (range > 0){
                RenderHelp::DrawCircleColor(obj->box.GetCenter(),range,100,255,200,30,90);
            }
        }
    }
    Light::GetInstance()->Update(0,LIGHT_GEN);
    if (Tower::BuildMode > 0 && Tower::BuildMode < 10){
        twr.SetClip((Tower::BuildMode-1)*48,0,48,48);
        Point pos = g_input.GetMouse();
        pos.GridSnap(GRID_SIZE);
        twr.SetAlpha(100);
        pos.x += 4;
        pos.y += 7;
        twr.Render(pos);
    }else if (Tower::BuildMode == 10){
        twr.SetClip(0,48,48,48);
        Point pos = g_input.GetMouse();
        pos.GridSnap(GRID_SIZE);
        twr.SetAlpha(100);
        pos.x += 8;
        pos.y += 8;
        twr.Render(pos);
    }


    if (Tower::BuildMode > 0){
        Point pos = g_input.GetMouse();
        pos.GridSnap(GRID_SIZE);
        Rect r;
        r.x = pos.x;
        r.y = pos.y;
        r.w = GRID_SIZE;
        r.h = GRID_SIZE;
        for (int y=-3;y<=3;y++)
            for (int x=-3;x<=3;x++){
                uint8_t alpha = 255;
                if (abs(x) == 3 || abs(y) == 3){
                    alpha = 20;
                }else if (abs(x) == 2 || abs(y) == 2){
                    alpha = 40;
                }else if (abs(x) == 1 || abs(y) == 1){
                    alpha = 127;
                }
                bool isColliding = false;
                Rect r2(r.x + x * r.w,r.y + y * r.h,r.w,r.h);
                if (!isBuildAble(r2.GetPos())){
                    isColliding = true;
                    alpha = std::min(alpha,(uint8_t)40);
                }

                RenderHelp::DrawSquareColorA(r2,isColliding ? 255 : 0,isColliding ? 0 : 255,0,alpha,!isColliding);
            }
    }


    /*sdds.Bind();

    sdds.SetUniform<std::vector<Point>>("lighter", {Point(0.5,0.5),Point(0.6,0.6)} );


    //RenderHelp::DrawCircleColor(Point(SCREEN_SIZE_W/2,SCREEN_SIZE_H/2),300,255,255,255,255);
    */

    tr.Bind();
    glDisable(GL_BLEND);
    RenderHelp::DrawSquareColor(Rect(0,0,SCREEN_SIZE_W,SCREEN_SIZE_H),0,0,0,0);
    glEnable(GL_BLEND);


    Point pt = g_input.GetMouse();
    std::vector<Rect> blocklines;

    blocklines.emplace_back(Rect(SCREEN_SIZE_W/2 + 34,SCREEN_SIZE_H/2,34,34));
    blocklines.emplace_back(Rect(SCREEN_SIZE_W/2 - 100,SCREEN_SIZE_H/2,32,32 ));




    //sdds.Bind();
    sdds.SetUniform<float>("minDark", 0.5f);
    sdds.SetUniform<Point>("centerLight",pt );
    sdds.SetUniform<Point>("screenSize",  Point(SCREEN_SIZE_W,SCREEN_SIZE_H)  );

    std::vector<Point> vert;

    int sides = 80;
    float radius = 190;
    Point3 adder[4];
    for (auto &it : blocklines){
        RenderHelp::DrawSquareColor(it,0,0,0,100);
        Point p1(it.x        - pt.x, it.y       - pt.y);
        Point p2(it.x + it.w - pt.x, it.y       - pt.y);
        Point p3(it.x + it.w - pt.x, it.y+it.h  - pt.y);
        Point p4(it.x        - pt.x, it.y+it.h  - pt.y);



        if (p1.getDistance(Point()) <= radius){
            vert.emplace_back(p1);
        }
        if (p2.getDistance(Point()) <= radius){
            vert.emplace_back(p2);
        }
        if (p3.getDistance(Point()) <= radius){
            vert.emplace_back(p3);
        }
        if (p4.getDistance(Point()) <= radius){
            vert.emplace_back(p4);
        }

    }

    glLoadIdentity();
    glTranslatef(pt.x, pt.y, 0.0f);

    glColor4ub( 255,0,0,200 );



    GLfloat angle;




    /*for (int i = 0; i <= sides; i++) {
         angle = i * 2.0f * Geometry::PI() / sides;
         Point a = pt;
         Point b = pt + Point(cos(angle) *  radius, sin(angle) *  radius);
         Point alt;

         for (auto &it : blocklines){
             Point block(it.x,it.y);
             Point block2(it.w,it.h);

             if (!get_line_intersection(a.x,a.y, b.x, b.y, block.x, block.y,  block2.x, block2.y, &alt.x, &alt.y)){
                vert.emplace_back(Point3(cos(angle) *  radius, sin(angle) *  radius, 0.0f));
             }else{
                 alt = alt-pt;
                 Point p1(block.x - pt.x, block.y - pt.y);
                 Point p2(block2.x - pt.x, block2.y - pt.y);
                 if ( p1.x > 0){
                    vert.emplace_back(Point3(p1.x, p1.y, 1.0f));
                    vert.emplace_back(Point3(p2.x, p2.y, 1.0f));
                 }else{
                    vert.emplace_back(Point3(p2.x, p2.y, 1.0f));
                    vert.emplace_back(Point3(p1.x, p1.y, 1.0f));
                 }
                 //vert.emplace_back(Point3(alt.x, alt.y, 1.0f));
             }

         }
    }*/
    //glBegin(GL_TRIANGLE_FAN);


    std::vector<Point> fvec;

    for (auto &it : vert){
        fvec.emplace_back(Point(it.x, it.y));
    }
    glBegin(GL_LINES);
    //glBegin(GL_TRIANGLE_FAN);
    //glVertex2f(0,0);

    for (auto &it : fvec){
        glVertex2f(0,0);
        glVertex2f(it.x, it.y);
    }

    /*for (int i = 0; i <= sides; i++) {
         angle = i * 2.0f * Geometry::PI() / sides;
         Point a = pt;
         Point b = pt + Point(cos(angle) *  radius, sin(angle) *  radius);
         Point alt;
         if (!get_line_intersection(a.x,a.y, b.x, b.y, block.x, block.y,  block2.x, block2.y, &alt.x, &alt.y)){
            glVertex2f(cos(angle) *  radius, sin(angle) *  radius);
            //RenderHelp::DrawLineColor(a,b,255,255,255,255);
         }else{

            glVertex2f(alt.x, alt.y);



            alt = alt-pt;

            float d1 = block.getDirection(pt);
            float d2 = block2.getDirection(pt);

            glVertex2f(cos(d1) *  radius, sin(d1) *  radius);
            glVertex2f(block.x - pt.x, block.y - pt.y);
            glVertex2f(block2.x - pt.x, block2.y - pt.y);
            glVertex2f(cos(d2) *  radius, sin(d2) *  radius);


            //glVertex2f(block2.x - pt.x, block2.y - pt.y);
            //glVertex2f(block.x - pt.x, block.y - pt.y);



            //

            //glVertex2f(alt.x, alt.y);
            //RenderHelp::DrawLineColor(a,alt,255,255,255,255);
         }
         //glVertex2f( cos(angle) *  radius, sin(angle) *  radius);
     }*/

     glEnd();

    glPopMatrix();


    //sdds.Unbind();
    tr.UnBind();
    //invrt.Bind();
    tr.Render(Point(0,0));
    //invrt.Unbind();

    ThreadPool::GetInstance().Lock();
    //Light::GetInstance()->Render();

    //if (g_input.IsKeyDown(SDLK_TAB))
        lg.Render(Point(32,32));
    RenderWindowses();

}



bool GameLevel::Signal(SignalRef s){

    return false;
}

void GameLevel::Input(){
    if (InputManager::GetInstance().ShouldQuit()){

        Console::GetInstance().AddText("DEVE QITAR");
        requestQuit = true;
    }

}


bool GameLevel::isBuildAble(Point p,bool cp){
    if (p.x < 0 || p.y < 0){
        return false;
    }
    p.GridSnap(GRID_SIZE);

    Rect r(p.x+1,p.y+1,GRID_SIZE-2,GRID_SIZE-2);
    for (auto &obj : Game::GetCurrentState().Pool){
        if (!obj->IsDead() && !obj->Is(TYPEOF(Burr))) {
            if (Collision::IsColliding(obj->box,r) ){
                return false;
            }
        }
    }
    Circle c(SCREEN_SIZE_W/2,(SCREEN_SIZE_H+GRID_SIZE*2)/2,40);
    if (Collision::IsColliding(r,c)){
        return false;
    }
    for (auto &it : MakerPosition){
        Rect r2(it.x,it.y,GRID_SIZE,GRID_SIZE);
        if (Collision::IsColliding(r2,r) ){
            return false;
        }
    }
    if (cp){
        GameLevel::pfer.AddBlock(p.x,p.y,true);
        std::stack<Point> lpth = GameLevel::pfer.Find(outPositions[0],PointInt(MakerPosition[0].x,MakerPosition[0].y));
        GameLevel::pfer.AddBlock(p.x,p.y,false);
        if (lpth.size() <= 0){
            return false;
        }
    }
    return true;
}

void GameLevel::internalUpdatePaths(){
    if (requestDelete || requestQuit)
        return;

    for (int i=0;i<4;i++){
        Paths[i] = GameLevel::pfer.Find(outPositions[i],PointInt(MakerPosition[i].x,MakerPosition[i].y));
        if (Paths[i].size() > 0){
            LastValidPaths[i] = Paths[i];
        }else{
            Paths[i] = LastValidPaths[i];
        }
    }
    for (auto &obj : Pool.ForGroup(EnemyPoolGroup)){
        if (obj && !obj->IsDead()){
           static_cast<Enemy*>(obj)->NotifyNewPath();
        }
    }
}

void GameLevel::MakeEnemy(){
    if (m_sock->IsHosting()){
        int hp = 10 + GameLevel::GameStorage[2]/2 + (rand()%(20+GameLevel::GameStorage[2]));
        float speed = 3.0f + GameLevel::GameStorage[3]/10.0f;
        bear::out << "The hp is : " << hp << "\n";
        bear::out << "The sp is : " << speed << "\n";
        Enemy* T = (Enemy*)Game::GetCurrentState().Pool.AddInstance(Enemy(rand()%4,hp, speed,1 + rand()%4,true));
        m_sock->SendNewObject(T);
    }
}


bool GameLevel::MakeTower(int x,int y,int id,int from){
    bool canMake = isBuildAble(Point(x,y),true);
    int cost = 1;
    switch (id){
        case 1:
            cost = 2;
            break;
        case 2:
            cost = 4;
            break;
        case 3:
            cost = 8;
            break;
        case 4:
            cost = 16;
            break;
    }
    if (GameLevel::GameStorage[1] < cost){
        canMake = false;
    }
    if (canMake){
        Tower* T = (Tower*)Game::GetCurrentState().Pool.AddInstance(Tower(x,y,id));
        int newId = m_sock->SendNewObject(T);
        GameLevel::SetValue(1,-cost,true);
        if (from == -1){
            Order::Proceed(T);
        }else{
            m_sock->SendProceed(newId,from);
        }
        return true;
    }else{
        if (from == -1){
            Order::Cancel();
        }else{
            m_sock->SendCancel(from);
        }
        return false;
    }
}
void GameLevel::RequestMakeTower(int x,int y,int id){
    if (m_sock->IsHosting()){
        MakeTower(x,y,id,-1);
    }else{
        m_sock->RequestTower(x,y,id);
    }
}

void GameLevel::SendDestroyObject(NetworkObject *obj){
    if (m_sock->IsHosting()){
        m_sock->SendDestroyObject(obj);

    }
}

void GameLevel::SendCarry(NetworkObject *obj,int val,NetworkObject *seed){
    int id = cmap.GetIdByCreature(obj);
    int id2 = cmap.GetIdByCreature(seed);
    if (id != -1 && id2 != -1){
        SocketMessage reply;
        reply.Add<uint8_t>(0xBE);
        reply.Add<int>(id);
        reply.Add<int>(val);
        if (!m_sock->IsHosting()){
            reply.Add<int>(id2);
        }
        ReliableUdpServer::SetSendMode(ENET_PACKET_FLAG_RELIABLE);
        m_sock->Send(&reply,-1);
    }
}

void GameLevel::SendHealth(NetworkObject *obj,int health){
    if (m_sock->IsHosting()){
        int id = cmap.GetIdByCreature(obj);
        if (id != -1){
            m_sock->SendHealth(id,health);
        }
    }
}

int GameLevel::SetValue(int field,int val,bool increment){
    if (field < 0 || field >= 255){
        return 0;
    }

    if (instance->m_sock->IsHosting()){
        if (increment){
            GameStorage[field] += val;
        }else{
            GameStorage[field] = val;
        }
        instance->m_sock->UpdateStorage(field,GameStorage[field]);
    }
    return GameStorage[field];
}

void GameLevel::MakeEffect(Point pos,int dmg){
    Particle *p = ParticleCreator::CreateAnimatedText(pos.x,pos.y,0,-2.0f,Text(utils::format("%d",dmg),14,{200,23,23,255}),10.0f);
    if (p){
        p->SetAlphaAsDuration();
        p->SetDepth(9999);
    }
    if (instance->m_sock->IsHosting()){
        SocketMessage reply;
        reply.Add<uint8_t>(0x22);
        reply.Add<int>(pos.x);
        reply.Add<int>(pos.y);
        reply.Add<int>(dmg);
        ReliableUdpServer::SetSendMode(ENET_PACKET_FLAG_UNSEQUENCED);
        instance->m_sock->Send(&reply,-1);
    }
}

void GameLevel::NotifyAttack(Burr *bur){

    int id = cmap.GetIdByCreature(bur);
    if (id != -1){
        SocketMessage reply;
        reply.Add<uint8_t>(0x10);
        reply.Add<int>(id);
        instance->m_sock->Send(&reply,-1);
    }

}

void GameLevel::NotifyDamage(NetworkObject *obj,int dmg){
    if (!instance->m_sock->IsHosting()){
        int id = cmap.GetIdByCreature(obj);
        if (id != -1){
            SocketMessage reply;
            reply.Add<uint8_t>(0x11);
            reply.Add<int>(id);
            reply.Add<int>(dmg);
            instance->m_sock->Send(&reply,-1);
        }
    }
}
void GameLevel::SendAttack(NetworkObject *obj,Point pos){
    if (m_sock->IsHosting()){
        int id = cmap.GetIdByCreature(obj);
        if (id != -1){
            m_sock->SendAttack(id,pos);
        }
    }
}
