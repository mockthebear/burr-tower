#include "gamebehavior.hpp"

#include "title.hpp"
#include "gamelevel.hpp"
#include "../luasystem/luacaller.hpp"
#include "../luasystem/luainterface.hpp"
#include "../engine/defaultbehavior.hpp"
#include "../engine/gamebase.hpp"
#include "../performance/console.hpp"
#include "../framework/resourcemanager.hpp"
#include "../framework/threadpool.hpp"


#include <string>






GameBehavior& GameBehavior::GetInstance(){
    static GameBehavior teste;
    return teste;
}


GameBehavior::GameBehavior(){
    /* Create here the default */
    DefaultBehavior::GetInstance();


}

GameBehavior::~GameBehavior(){

}


bool GameBehavior::Begin(){
    TextureLoadMethod::DefaultLoadingMethod.mode = TEXTURE_TRILINEAR;
    return true;
}
bool GameBehavior::OnLoad(){
    PointInt P = ScreenManager::GetInstance().GetDisplaySize();
    PointInt P2 = ConfigManager::GetInstance().GetScreenSize();

    bear::out << "Supported main display is "<< P.x << " x "<< P.y << "\n";
    bear::out << "Started with screen "<< P2.x << " x "<< P2.y << "\n";

    bear::out << "Starting state.\n";
    GlobalMethodRegister::RegisterGlobalTable(LuaManager::L,"g_level");
    GlobalMethodRegister::RegisterGlobalTableMethod(LuaManager::L,"g_level","SetBuildMode",std::function<void(int)>([](int field)
    {
        Tower::BuildMode = field;
    }));

    GlobalMethodRegister::RegisterGlobalTableMethod(LuaManager::L,"g_level","GetStorage",std::function<int(int)>([](int field)
    {
        if (field < 0 || field >= 255){
            return 0;
        }
        return GameLevel::GameStorage[field];
    }));
    GlobalMethodRegister::RegisterGlobalTableMethod(LuaManager::L,"g_level","SetStorage",std::function<void(int,int)>([](int field,int value)
    {
        GameLevel::SetValue(field,value);
    }));
    //GameStorage[1]
    /*
    Game::GetInstance()->AddState(new Test_());
    */
    TextureLoadMethod::DefaultLoadingMethod.mode = TEXTURE_NEAREST;
    Game::GetInstance()->AddState(new Title());

    return DefaultBehavior::GetInstance().OnLoad();
}

void GameBehavior::OnClose(){
    ResourceManager::GetInstance().Erase("assets");
    DefaultBehavior::GetInstance().OnClose();
}



bool GameBehavior::OnResize(int newW,int newH){
    return DefaultBehavior::GetInstance().OnResize(newW,newH);
}


void GameBehavior::OnFocus(bool hasFocus){

    DefaultBehavior::GetInstance().OnFocus(hasFocus);
}

void GameBehavior::OnRestored(){

    DefaultBehavior::GetInstance().OnRestored();
}
void GameBehavior::OnMinimized(){

    DefaultBehavior::GetInstance().OnMinimized();
}
