#include "title.hpp"
#include <sstream>
#include <cmath>

#include "title.hpp"

#include "../luasystem/luacaller.hpp"
#include "../luasystem/luatools.hpp"
#include "../engine/renderhelp.hpp"


#include "../tests/testthread.hpp"
#include "../tests/testfiles.hpp"
#include "gamelevel.hpp"
#include "protocolserver.hpp"
#include "protocolclient.hpp"


Title::Title(){
    requestQuit = requestDelete = false;

    Camera::Initiate(Rect(0,0,SCREEN_SIZE_W,SCREEN_SIZE_H),128,200);
    Camera::speed = 0;
    needConnect = false;
    waitName = false;

}


void Title::Begin(){

    LuaCaller::LoadFile(LuaManager::L,"lua/room.lua");
    LuaCaller::Pcall(LuaManager::L);

    LuaCaller::CallGlobalField(LuaManager::L,"onLoad");
}

Title::~Title(){
    Pool.ErasePools();
}


void Title::Update(float dt){

    Input();

    Pool.Update(dt);
    Map.clear();
    Pool.PreRender(Map);


    Camera::Update(dt);

    UpdateWindowses(dt);


}

void Title::Render(){
    RenderHelp::DrawSquareColorA(0,0,SCREEN_SIZE_W,SCREEN_SIZE_H,255,255,255,255);


    RenderInstances();

    RenderWindowses();
    //Console::GetInstance().Render();


}
bool Title::Signal(SignalRef s){
    if (s.GetType() == SIGNAL_TYPE_INT){
        if (s.GetInt() == 777){
            waitName = true;
        }else if (s.GetInt() == 1){
            ReliableUdpServer *server = new ReliableUdpServer();
            if (server->Bind(8888)){
                Game::GetInstance()->AddState(new GameLevel(new ProtocolServer(static_cast<Connection*>(server)),name));
                return true;
            }else{
                delete[]server;
            }
        }
    }else if (s.GetType() == SIGNAL_TYPE_STR){
        if (waitName){
            name = s.GetStr();
            waitName = false;
            return true;
        }
        ReliableUdpClient *client = new ReliableUdpClient();
        if (client->Connect(s.GetStr(),8888,1000)){
            Game::GetInstance()->AddState(new GameLevel(new ProtocolClient(static_cast<Connection*>(client)),name));
            return true;
        }

    }

    return false;
}

void Title::Input(){
    if (InputManager::GetInstance().ShouldQuit()){
        Console::GetInstance().AddText("DEVE QITAR");
        requestQuit = true;
    }


    if( InputManager::GetInstance().KeyPress(SDLK_ESCAPE) ) {
        Console::GetInstance().AddText("SCAPE");
        requestDelete = true;
    }
}
