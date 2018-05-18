#include "../engine/genericstate.hpp"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>
#include <list>
#include "networkobject.hpp"
#include "creaturemap.hpp"


#pragma once


class WibblyWobble{
    public:
    WibblyWobble(){};
    WibblyWobble(Point minX,float duration,float strenght,Point oScale=Point(1,1)){
        this->minX = minX;
        this->oScale = oScale;
        counter = duration;
        this->strenght = strenght;
        runner = 0;
    };
    void Reset(float duration,float runnerS = 0){
        counter = duration;
        runner = runnerS;
    };
    void Update(float dt,Sprite &sp){
        runner += strenght*dt;
        if (counter != -1337)
            counter -= dt;
        if (counter <= 0 && counter != -1337){
            sp.SetScale(oScale);
            return;
        }
        float sx = oScale.x + (minX.x) * (sin(runner));
        float sy = oScale.y + (minX.y) * (sin(runner));
        sp.SetScale(Point(sx,sy));
    };

    float counter,strenght,runner;

    Point minX;
    Point oScale;
};


enum SelectedOption{
    SELECTED_MOVE = 0,

    SELECTED_BUILD_BEGIN,
    SELECTED_BUILD_LAND,
    SELECTED_BUILD_BARRICADE,
    SELECTED_BUILD_LAST,

    SELECTED_BUILD_TOWER1,
    SELECTED_BUILD_TOWER2,
    SELECTED_BUILD_TOWER3,
    SELECTED_BUILD_TOWER4,
};

class State: public GenericState{
    public:
        State(){};
        virtual ~State(){};
        virtual void Update(float dt) =0;
        virtual void Render() =0;
        virtual void Begin() =0;


};

class NetworkState: public State{
    public:
        NetworkState(){};
        virtual ~NetworkState(){};
        template<typename T, typename ... Args> bool MakeObjectWithId(int id,Args...arg){
            T* obj = (T*)Pool.AddInstance(T(arg...));
            if (!obj){
                return false;
            }
            return cmap.RegisterFrom(obj,id);
        }

        template<typename T, typename ... Args> int MakeObject(Args...arg){
            T* obj = (T*)Pool.AddInstance(T(arg...));
            if (!obj){
                return -1;
            }
            int id = cmap.RegisterCreature(obj);
            return id;
        }
        CreatureMap cmap;
};




