#include "protocolserver.hpp"
#include "burr.hpp"
#include "enemy.hpp"
#include "gamelevel.hpp"
#include "seeds.hpp"
#include "../socket/reliableudp.hpp"
#include <algorithm>

ProtocolServer::ProtocolServer(Connection *m):Protocol(m){
    bearCount = 1;
    auto v = [=](int pid){
        if (Ownership[pid].size()){
            for (auto &it : Ownership[pid]){
                NetworkObject *o = (NetworkObject *)GetState()->cmap.GetCreatureById(it);
                if (o){
                    GameLevel::instance->SendDestroyObject(o);
                    o->Kill();
                }
            }
            Ownership[pid].clear();
        }
    };
    m_sock->SetDisconectFunction(v);
};
void ProtocolServer::Update(NetworkState *s,float dt){
    m_sock->Update(dt);

    updateCounter -= dt;
    if (updateCounter <= 0){
        updateCounter = updateInterval;

        SocketMessage msg;
        msg.Clear();
        currentState = s;
        PeerList Peers = m_sock->GetPeers();
        for (auto &peer : Peers){
            m_Pid = peer;
            while (m_sock->Receive(&msg,peer)){
                ParsePacket(s,peer,&msg);
            }
        }
        UpdateBears();
        UpdateEnemy();
    }
}
void ProtocolServer::SendCancel(int id){
    SocketMessage reply;
    reply.Add<uint8_t>(0x0f);
    ReliableUdpServer::SetSendMode(ENET_PACKET_FLAG_RELIABLE);
    m_sock->Send(&reply,id);
}

void ProtocolServer::SendProceed(int obj,int id){
    SocketMessage reply;
    reply.Add<uint8_t>(0x0e);
    reply.Add<int>(obj);

    ReliableUdpServer::SetSendMode(ENET_PACKET_FLAG_RELIABLE);
    m_sock->Send(&reply,id);
}

int ProtocolServer::SendNewObject(NetworkObject *obj,int id){
    SocketMessage reply;
    if (id == -1){
        id = GetState()->cmap.RegisterCreature(obj);
    }
    reply.Add<uint8_t>(0x02);
    reply.Add<int>(id);                              // object id
    reply.Add<int>(obj->GetObjType());               // object type
    reply.Add<int>(-1);                              // object owner  0 means the server owns it
    obj->CreationFunction(&reply);
    ReliableUdpServer::SetSendMode(ENET_PACKET_FLAG_RELIABLE);
    m_sock->Send(&reply,-1);
    return id;
}

void ProtocolServer::ParsePacket(NetworkState *s,int peer,SocketMessage *msg){
    uint8_t firstByte = msg->Get<uint8_t>();
    switch (firstByte){
        case 0x01:
            ParseFirstPacket(msg); break;
        case 0x10:
            ParseBearAttack(msg); break;
        case 0x11:
            ParseClientDamage(msg); break;
        case 0xA3:
            ParseBearMoved(msg); break;
        case 0xBE:
            ParseCarry(msg); break;
        case 0x33:
            ParseRequestedTower(msg); break;
        default:
            bear::out << "Unknow packet " << firstByte << "\n";
    }
}


void ProtocolServer::ParseFirstPacket(SocketMessage *msg){
    SocketMessage reply;
    ObjectMap& mapp = GetState()->cmap.GetObjects();

    reply.Add<uint8_t>(0x01);                           // pack id
    reply.Add<int>(GetPid());                           // user id
    reply.Add<uint32_t>(mapp.size());                   // object count in map
    for (auto &objEntry : mapp){                        //
        reply.Add<int>(objEntry.first);                 // object id
        reply.Add<int>(objEntry.second->GetObjType());  // object type
        reply.Add<int>(-1);                              // object owner  0 means the server owns it
        objEntry.second->CreationFunction(&reply);
    }
    //todo socket mode reliable
    ReliableUdpServer::SetSendMode(ENET_PACKET_FLAG_RELIABLE);
    m_sock->Send(&reply,GetPid());

    //Send his bear if he is not on spectate
    msg->ReadByte();
    std::string nm = msg->GetString();
    reply.Clear();
    int id = GetState()->MakeObject<Burr>(SCREEN_SIZE_W/2,SCREEN_SIZE_H/2,false,nm);
    Burr *bur = (Burr*)GetState()->cmap.GetCreatureById(id);
    reply.Add<uint8_t>(0x02);                           // protocol id
    reply.Add<int>(id);                                 // objId
    reply.Add<int>(bur->GetObjType());                  // obj type
    reply.Add<int>(GetPid());                           // obj owner
    bur->CreationFunction(&reply);                      // obj data
    m_sock->Send(&reply,-1);
    Ownership[GetPid()].emplace_back(id);

    for (int i=0;i<10;i++){
        UpdateStorage(i,GameLevel::GameStorage[i],GetPid());
    }

    bear::out << nm << " Logged in\n";




}
void ProtocolServer::UpdateEnemy(){
    ObjectMap mapp = GetState()->cmap.GetObjects();
    //NetworkObject *inst = nullptr;
    SocketMessage reply;
    ReliableUdpServer::SetSendMode(ENET_PACKET_FLAG_UNSEQUENCED);
    std::vector<Enemy*> en;
    std::vector<int> idss;
    for (auto &objEntry : mapp){
        if (!objEntry.second->IsDead() && objEntry.second->Is(TYPEOF(Enemy))){
            en.emplace_back((Enemy*)objEntry.second);
            idss.emplace_back(objEntry.first);
        }
    }
    float perPacket = 8.0f;
    for (int i=0;i<std::ceil(en.size()/perPacket);i++){
        reply.Clear();
        reply.Add<uint8_t>(0xA3);
        int position = i*perPacket;
        int count = perPacket;
        if ((i+1)*(uint32_t)perPacket > en.size()){
            position = (i)*perPacket;
            count = en.size()%(int)perPacket;
        }


        reply.Add<int>(count);

        for (int e=0;e<count;e++){

            reply.Add<int>(idss[position + e]);
            reply.Add<int>(en[position + e]->box.x);
            reply.Add<int>(en[position + e]->box.y);
        }

        m_sock->Send(&reply,-1);
    }

}
void ProtocolServer::UpdateBears(){
    ObjectMap mapp = GetState()->cmap.GetObjects();
    NetworkObject *inst = nullptr;
    SocketMessage reply;
    ReliableUdpServer::SetSendMode(ENET_PACKET_FLAG_UNSEQUENCED);
    for (auto &objEntry : mapp){
        if (objEntry.second->Is(TYPEOF(Burr))){
            inst = (NetworkObject*)objEntry.second;
            reply.Clear();
            reply.Add<uint8_t>(0xA3);                   // message header
            reply.Add<int>(1);                          // amount
            reply.Add<int>(objEntry.first);             // objId
            reply.Add<int>(inst->box.x);                // y pos
            reply.Add<int>(inst->box.y);                // x pos
            m_sock->Send(&reply,-1);                    // Send to evebody
        }
    }

}
void ProtocolServer::ParseRequestedTower(SocketMessage *msg){
    int x = msg->Get<int>();
    int y = msg->Get<int>();
    int type = msg->Get<int>();
    GameLevel::instance->MakeTower(x,y,type,GetPid());
}

void ProtocolServer::ParseBearAttack(SocketMessage *msg){
    int id = msg->Get<int>();
    Burr *bur = (Burr*)GetState()->cmap.GetCreatureById(id);
    if (bur){
        bur->ForceAttack();
    }
}

void ProtocolServer::ParseCarry(SocketMessage *msg){
    int id = msg->Get<int>();
    int val = msg->Get<int>();
    int id2 = msg->Get<int>();
    Burr *bur = (Burr*)GetState()->cmap.GetCreatureById(id);
    Seeds *sd = (Seeds*)GetState()->cmap.GetCreatureById(id2);

    if (bur && sd){
        bur->MakeCarry(val);
        ReliableUdpServer::SetSendMode(ENET_PACKET_FLAG_RELIABLE);
        GameLevel::instance->SendCarry(bur,val,sd);
        GameLevel::instance->SendDestroyObject(sd);
        sd->Kill();
    }
}

void ProtocolServer::ParseClientDamage(SocketMessage *msg){
    int id = msg->Get<int>();
    int dmg = msg->Get<int>();
    NetworkObject *obj = GetState()->cmap.GetCreatureById(id);
    if (obj){
        obj->NotifyDamage(nullptr,dmg);
    }
}


void ProtocolServer::ParseBearMoved(SocketMessage *msg){
    int x = msg->Get<int>();
    int y = msg->Get<int>();
    int id = msg->Get<int>();
    Burr *bur = (Burr*)GetState()->cmap.GetCreatureById(id);
    if (bur){
        auto lst = Ownership[GetPid()];
        if (std::find(lst.begin(), lst.end(), id) != lst.end()){
            bur->InterpolationBox.x = x;
            bur->InterpolationBox.y = y;
        }else{
            bear::out << GetPid() << " do not own " << id << "\n";
        }
    }else{
        bear::out << "No know bear id " << id << "\n";
    }
}
void ProtocolServer::SendHealth(int id,int health){
    SocketMessage reply;
    reply.Add<uint8_t>(0x08);
    reply.Add<int>(id);
    reply.Add<int>(health);
    ReliableUdpServer::SetSendMode(ENET_PACKET_FLAG_UNSEQUENCED);
    m_sock->Send(&reply,-1);
}


void ProtocolServer::UpdateStorage(int field,int val,int who){
    SocketMessage reply;
    reply.Add<uint8_t>(0x07);
    reply.Add<int>(field);
    reply.Add<int>(val);
    ReliableUdpServer::SetSendMode(ENET_PACKET_FLAG_RELIABLE);
    m_sock->Send(&reply,who);
}


void ProtocolServer::SendAttack(int id,Point pos){
    SocketMessage reply;
    reply.Add<uint8_t>(0x09);
    reply.Add<int>(id);
    reply.Add<int>(pos.x);
    reply.Add<int>(pos.y);
    ReliableUdpServer::SetSendMode(ENET_PACKET_FLAG_RELIABLE);
    m_sock->Send(&reply,-1);
}


void ProtocolServer::SendDestroyObject(NetworkObject *obj){
    SocketMessage reply;

    int id = GetState()->cmap.GetIdByCreature(obj);
    if (id != -1){
        GetState()->cmap.DestroyObject(id);
        reply.Add<uint8_t>(0x0a);
        reply.Add<int>(id);
        ReliableUdpServer::SetSendMode(ENET_PACKET_FLAG_RELIABLE);
        m_sock->Send(&reply,-1);
    }
}
