#include "../engine/object.hpp"
#include "progressbar.hpp"
#include "networkobject.hpp"

#pragma once
#include <stack>

class ProgressThings: public NetworkObject{
    public:
        ProgressThings(int i):NetworkObject(i){};
        virtual void Touch(GameObject *,int p)=0;
        ProgressBar p;
        int health;
};
