#include "protocolclient.hpp"
#include "../socket/reliableudp.hpp"
#include "burr.hpp"
#include "tower.hpp"
#include "enemy.hpp"
#include "gamelevel.hpp"
#include "seeds.hpp"

void ProtocolClient::Update(NetworkState *s,float dt){
    m_sock->Update(dt);

    updateCounter -= dt;
    if (updateCounter <= 0){
        updateCounter = updateInterval;

        SocketMessage msg;
        msg.Clear();
        PeerList Peers = m_sock->GetPeers();
        currentState = s;
        for (auto &peer : Peers){
            m_Pid = peer;
            while (m_sock->Receive(&msg,peer)){
                ParsePacket(s,peer,&msg);
            }
        }
        ObjectMap mapp = GetState()->cmap.GetObjects();
        SocketMessage reply;
        ReliableUdpClient::SetSendMode(ENET_PACKET_FLAG_RELIABLE);
        for (auto &objEntry : mapp){
            if (objEntry.second->IsLocal()){
                if (objEntry.second->NetworkClientUpdate(&reply)){
                    reply.Add<int>(objEntry.first);
                    m_sock->Send(&reply,-1);
                    reply.Clear();
                }
            }
        }
    }
}

void ProtocolClient::ParsePacket(NetworkState *s,int peer,SocketMessage *msg){
    uint8_t firstByte = msg->Get<uint8_t>();
    switch (firstByte){
        case 0x01:
            ParseFirstPacket(msg); break;
        case 0x02:
            CreateObject(msg); break;
        case 0x07:
            ParseStorageChange(msg);break;
        case 0x08:
            ParseChangeHealth(msg);break;
        case 0x09:
            ParseAttack(msg);break;
        case 0x0a:
            ParseDestroy(msg);break;
        case 0x0e:
            ParseProceed(msg); break;
        case 0x0f:
            ParseCancel(msg); break;
        case 0x10:
            ParseBearAttack(msg); break;
        case 0x22:
            ParseEffect(msg); break;
        case 0xA3:
            ParseBearMove(msg); break;
        case 0xBE:
            ParseCarry(msg); break;
        default:
            bear::out << "Unknow packet " << firstByte << "\n";
    }
}

void ProtocolClient::ParseFirstPacket(SocketMessage *msg){
    m_myPid = msg->Get<int>(); // My id
    uint32_t objectCount = msg->Get<uint32_t>();
    for (uint32_t i=0;i<objectCount;i++){
        if (!CreateObject(msg)){
            continue;
        }
    }
}

void ProtocolClient::ParseCancel(SocketMessage *msg){
    Order::Cancel();
}
void ProtocolClient::ParseDestroy(SocketMessage *msg){
    int objId = msg->Get<int>();
    GameObject *obj = GetState()->cmap.GetCreatureById(objId);

    if (obj){
        GetState()->cmap.DestroyObject(objId);
        obj->Kill();
    }else{
        bear::out << "Attept to destroy "<<objId << ". id not found\n";
    }
}

void ProtocolClient::ParseBearAttack(SocketMessage *msg){
    int id = msg->Get<int>();
    Burr *bur = (Burr*)GetState()->cmap.GetCreatureById(id);
    if (bur){
        bur->ForceAttack();
    }
}

void ProtocolClient::ParseEffect(SocketMessage *msg){
    int x = msg->Get<int>();
    int y = msg->Get<int>();
    int dmg = msg->Get<int>();
    GameLevel::MakeEffect(Point(x,y),dmg);
}

void ProtocolClient::ParseAttack(SocketMessage *msg){
    int objId = msg->Get<int>();
    Point pos;
    pos.x = msg->Get<int>();
    pos.y = msg->Get<int>();
    GameObject *obj = GetState()->cmap.GetCreatureById(objId);
    if (obj && obj->Is(TYPEOF(Tower))){
        static_cast<Tower*>(obj)->AttackFunction(pos);
    }
}

void ProtocolClient::ParseChangeHealth(SocketMessage *msg){
    int objId = msg->Get<int>();
    int health = msg->Get<int>();
    GameObject *obj = GetState()->cmap.GetCreatureById(objId);
    if (obj && obj->Is(TYPEOF(Enemy))){
        static_cast<Enemy*>(obj)->health = health;
    }
}

void ProtocolClient::ParseStorageChange(SocketMessage *msg){
    int field = msg->Get<int>();
    int val = msg->Get<int>();
    bear::out << "changed storage to" << val << "\n";
    GameLevel::GameStorage[field] = val;
}

void ProtocolClient::ParseProceed(SocketMessage *msg){
    int objId = msg->Get<int>();
    GameObject *obj = GetState()->cmap.GetCreatureById(objId);
    if (obj)
        Order::Proceed(obj);
}

void ProtocolClient::RequestTower(int x,int y,int type){
    SocketMessage reply;
    reply.Add<uint8_t>(0x33);
    reply.Add<int>(x);
    reply.Add<int>(y);
    reply.Add<int>(type);
    ReliableUdpServer::SetSendMode(ENET_PACKET_FLAG_RELIABLE);
    m_sock->Send(&reply,-1);
}


bool ProtocolClient::CreateObject(SocketMessage *msg){
    int objId = msg->Get<int>();
    int objType = msg->Get<int>();
    int objOwner = msg->Get<int>();
    if (objType == 1){
        int x = msg->Get<int>();
        int y = msg->Get<int>();
        std::string name = msg->GetString();
        bear::out << "Created " << objId << " at " << x << ":"<<y<<" named "<<name<<"\n";
        if (GetState()->MakeObjectWithId<Burr>(objId,x,y,objOwner == m_myPid,name)){
            return true;
        }else{
            bear::out << "Failed to create object!!!!!\n";
        }
    }else if (objType == 2){
        int x = msg->Get<int>();
        int y = msg->Get<int>();
        int typ = msg->Get<int>();
        bear::out << "Created " << objId << " at " << x << ":"<<y<<"\n";
        if (GetState()->MakeObjectWithId<Tower>(objId,x,y,typ)){
            return true;
        }else{
            bear::out << "Failed to create object!!!!!\n";
        }
    }else if (objType == 3){
        int pid = msg->Get<int>();
        int maxhealth = msg->Get<int>();
        int health = msg->Get<int>();
        float spd = msg->Get<float>();
        int lktp = msg->Get<int>();
        int x = msg->Get<int>();
        int y = msg->Get<int>();
        if (GetState()->MakeObjectWithId<Enemy>(objId,pid,health,spd,lktp,false)){
            Enemy *obj = static_cast<Enemy*>(GetState()->cmap.GetCreatureById(objId));
            obj->box.x = x;
            obj->box.y = y;
            obj->maxHealth = maxhealth;
            return true;
        }else{
            bear::out << "Failed to create object!!!!!\n";
        }
    }else if (objType == 4){

        int x = msg->Get<int>();
        int y = msg->Get<int>();
        int value = msg->Get<int>();
        GetState()->MakeObjectWithId<Seeds>(objId,x,y,value);
        bear::out << "Made a seederino\n";
        return true;
    }
    bear::out << "Unknow object id "<<objId<<" typed "<<objType<<"\n";
    return false;
}

void ProtocolClient::ParseCarry(SocketMessage *msg){
    int id = msg->Get<int>();
    int val = msg->Get<int>();
    Burr *obj = (Burr *)GetState()->cmap.GetCreatureById(id);
    bear::out << "client carry\n";
    if (obj){
        bear::out << "vav: "<<val<<"\n";
        obj->MakeCarry(val);
    }
}
void ProtocolClient::ParseBearMove(SocketMessage *msg){
    int count = msg->Get<int>();
    for (int i=0;i<count;i++){
        int id = msg->Get<int>();
        int x = msg->Get<int>();
        int y = msg->Get<int>();
        NetworkObject *obj = GetState()->cmap.GetCreatureById(id);
        if (obj){
            obj->InterpolationBox.x = x;
            obj->InterpolationBox.y = y;
        }else{
            //bear::out << "Trying to move instance unknow "<<id<<"\n";
        }
    }
}
