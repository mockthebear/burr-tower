#include "../engine/object.hpp"
#include "networkobject.hpp"

#pragma once

typedef std::map<int,NetworkObject*> ObjectMap;

class CreatureMap{
    public:
        CreatureMap();
        int RegisterCreature(NetworkObject *obj);
        NetworkObject *GetCreatureById(int id);
        bool RegisterFrom(NetworkObject *obj,int id);
        ObjectMap& GetObjects(){ return ObjMap; }
        int GetIdByCreature(NetworkObject *obj);
        bool DestroyObject(int id);

    private:
        int counter;
        ObjectMap ObjMap;
};
