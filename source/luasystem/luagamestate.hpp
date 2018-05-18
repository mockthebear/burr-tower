#include "../engine/genericstate.hpp"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>
#include <list>

#ifndef __EMSCRIPTEN__

#pragma once

#include "../framework/poolmanager.hpp"

class LuaGameState: public GenericState{
    public:
        LuaGameState();
        ~LuaGameState();
        void Setup();
        void Update(float dt);
        void Render();
        void Begin();
        void End();
        void Resume(GenericState *);
        void Pause(GenericState *);
        PoolManager Pool;
        bool canClose;
        uint64_t other;
};

#endif // __EMSCRIPTEN__
