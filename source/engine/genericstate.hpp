
#ifndef STATEH
#define STATEH
#include "object.hpp"
#include "../ui/base.hpp"
#include "../ui/window.hpp"
#include "../framework/basepool.hpp"
#include "../framework/poolmanager.hpp"
#include "assetmanager.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>
#include <list>

enum TypeSignal{
    SIGNAL_TYPE_EMPTY=0,
    SIGNAL_TYPE_INT,
    SIGNAL_TYPE_FLOAT,
    SIGNAL_TYPE_DOUBLE,
    SIGNAL_TYPE_UINT,
    SIGNAL_TYPE_UINT64,
    SIGNAL_TYPE_VOID,
    SIGNAL_TYPE_STR,
};

union SignalContainer{
    int i;
    float f;
    double lf;
    uint32_t ui;
    uint64_t uli;
    void *v;
    const char *str;
};

class SignalRef{
    public:
        SignalRef():type(SIGNAL_TYPE_EMPTY){scont.uli = 0;};



        int GetInt(){ return scont.i; };
        float GetFloat(){ return scont.f; };
        double GetDouble(){ return scont.lf; };
        uint32_t GetUint(){ return scont.ui; };
        uint64_t GetUint64(){ return scont.uli; };
        void* GetVoid(){ return scont.v; };
        const char *GetStr(){ return scont.str; };


        void SetInt(int var){
            type = SIGNAL_TYPE_INT;
            scont.i = var;
        }
        void SetFloat(float var){
            type = SIGNAL_TYPE_FLOAT;
            scont.f = var;
        }
        void SetDouble(double var){
            type = SIGNAL_TYPE_DOUBLE;
            scont.lf = var;
        }
        void SetUint(uint32_t var){
            type = SIGNAL_TYPE_UINT;
            scont.ui = var;
        }
        void SetUint64(uint64_t var){
            type = SIGNAL_TYPE_UINT64;
            scont.uli = var;
        }
        void SetVoid(void *var){
            type = SIGNAL_TYPE_VOID;
            scont.v = var;
        }
        void SetString(const char *var){
            type = SIGNAL_TYPE_STR;
            scont.str = var;
        }

        TypeSignal GetType(){ return type;};

    private:
        TypeSignal type;
        SignalContainer scont;
};

/**
 * @brief Basic game state
 *
 * The GenericState is the most basic game state you have
 *
 * What is a state?\n
 * An state is an class that will describe what the game will do
 * You can have an state for the title, another for the level 1, then one for
 * the boss fight. Its an simple way to divide your game in pieces, then you work
 * in small parts.
 *
 * The functions descibed here are IMPORTANT. you must have them in your state
 *
 * The states work on an stack. Every time you add an state, it will be stacking. Unless you set
 * to the current state to be deleted. When you delete the current state, the new current state will be the last one.
 * if the stack gets empty, the game close.
 *
 *
 * Simple game state that you can have:
 @code

Mystate::Mystate(){

    requestQuit = requestDelete = false;


}


void Mystate::Begin(){
    ResourceManager::GetInstance().Load("data/packedFiles.burr","assets");

    background = Sprite("assets:bg.png");

    Pool.Register<Projectile>(400);

    Pool.AddInstance(Projectile(32,32));

}

void Mystate::End(){
    Pool.ErasePools();
    ResourceManager::GetInstance().Erase("assets");
}

void Mystate::Update(float dt){
    if (InputManager::GetInstance().ShouldQuit()){
        requestQuit = true;
    }
    if( InputManager::GetInstance().KeyPress(SDLK_ESCAPE) ) {
        requestQuit = true;
    }

    UpdateWindowses(dt);
    Pool.Update(dt);
    Pool.PreRender(Map);

}

void Mystate::Render(){
    //Not necessary.
    SDL_SetRenderDrawColor(BearEngine->GetRenderer(), 0,0,0, 0);
    SDL_RenderClear( BearEngine->GetRenderer() );

    background->Render(0,0);

    RenderInstances();

    RenderWindowses();
}

 @endcode

 */

class GenericState{
    public:
        /**
            *The constructor. You dont have to make am empty constructor. But you of course will need one
            *Please check GenericState::Begin
            *
            *<b>Things that you MUST HAVE in your constructor</b>
            @code
                requestDelete = false;
                requestQuit = false;
                ParticlePool = new SPP<Particle>(100);
            @endcode
        */
        GenericState();
        /**
            *Delete everything here
        */
        virtual ~GenericState(){};
        /**
            *Here is where you will make the code that describes your game.
            *Update instances\n
            *Check input\n
            *Check collision\n
            *And so on\n
        */
        virtual void Update(float dt) =0;
        /**
            *Now everything will be shown on screen here.
        */
        virtual void Render() =0;
        /**
            *Now this is tricky. At least on the first state, some things may not have been started yet
            *Like an object pool
            *Its always better start objects here!
        */
        virtual void Begin() =0;
        virtual void End() =0;

        /**
            *Used by the engine. check if this state is dead;
            *When this flat is set to true, the engine will delete this state.
        */
        bool RequestedDeleted(){return requestDelete;};
        /**
            *Used by the engine. This is a flag to say to the engine "Hey, please, end the game".
        */
        bool RequestedQuit(){return requestQuit;};

        /**
            *Its an basic GameObject pool. SPP<T>
            *Its used by the class ParticleCreator
        */
        SPP<Particle> *ParticlePool;
        /**
            *Render map. It is used to make an "depth" inside the 2D world.
            *Its an map to order the rendering.
            *Lets say, you are in a topdown perspective, you can use the default
            *preRender function:
            *Pool->PreRender(Map);
            *This will fill the map with an ordened array on the Y axis
            *When the function of true Render will be called, the things on top screen
            *will be rendered first.
            *When rendering just use:
            @code
            for (auto it=Map.begin(); it!=Map.end(); ++it){
                for (auto &k : *it->second){
                    k->Render();
                }
                delete it->second;
            }
            @endcode
            *Also you can go to GameObject class, and change the function
            *GameObject::hasPerspective. The less the value, more on "bottom" it will be
        */
        std::map<int,std::vector<GameObject*>> Map;

        void RenderInstances();
        /**
            *Well, you dont need to use pools for the objects at all.
            *Here, there is an vector!
        */
        std::vector<std::unique_ptr<GameObject>> ObjectArray;

        /**
            *You dont want use the object array, and want use pools?\n
            *Dont know how to mess with our pool class SPP<T>?\n
            *Try using our PoolManager =]\n
        */
        PoolManager Pool;
        /**
            *Asset manager
        */
        AssetMannager Assets;
        int STATEID;
        bool gameStarted;
        bool requestDelete,requestQuit;

        void UpdateInstances(float dt);
        void UpdateWindowses(float dt);
        void RenderWindowses();
        void AddWindow(UIBase *b);

        virtual bool Signal(SignalRef s){ return false;};

        uint64_t GetTicks(){ return p_StateTicks;};
    protected:
        friend class Game;



        virtual void Pause(GenericState *){};
        virtual void Resume(GenericState *){};

        /**
            *You may need show some windows. Here's a vector to Window components
        */

        std::vector<std::unique_ptr<UIBase>> GameWindows;
    private:
        UIBase *storedUi;
        uint64_t p_StateTicks;

};
#endif // STATEH
