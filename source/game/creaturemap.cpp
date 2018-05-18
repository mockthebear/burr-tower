#include "creaturemap.hpp"


CreatureMap::CreatureMap(){
    counter = 0;
};

int CreatureMap::RegisterCreature(NetworkObject *obj){
    counter++;
    ObjMap[counter] = obj;
    return counter;
}
NetworkObject *CreatureMap::GetCreatureById(int id){
    if (ObjMap.find(id) == ObjMap.end()){
        return nullptr;
    }
    return ObjMap[id];
}

bool CreatureMap::RegisterFrom(NetworkObject *obj,int id){
    if (ObjMap.find(id) == ObjMap.end()){
        ObjMap[id] = obj;
        return true;
    }
    return false;
}

bool CreatureMap::DestroyObject(int id){
    if (ObjMap.find(id) != ObjMap.end()){
        ObjMap.erase(ObjMap.find(id));
    }
    return true;
}

int CreatureMap::GetIdByCreature(NetworkObject *obj){
    for (auto &it : ObjMap){
        if (it.second == obj){
            return it.first;
        }
    }
    return -1;
}
