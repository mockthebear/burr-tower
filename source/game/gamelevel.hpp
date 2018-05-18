#pragma once
#include "state.hpp"
#include <stack>
#include "../socket/reliableudp.hpp"
#include "../performance/linegrap.hpp"
#include "burr.hpp"
#include "tower.hpp"
#include "protocol.hpp"
#include "creaturemap.hpp"
#include "../engine/pathfind.hpp"
#include "../engine/renderhelp.hpp"
#include "../engine/smarttexture.hpp"
#include "../engine/shadermanager.hpp"
#define GRID_SIZE 32



class GameLevel: public NetworkState{
    public:
        GameLevel(Protocol *p,std::string name);
        ~GameLevel();
        void Update(float);
        void Render();
        void Input();
        void Begin();
        void Resume(GenericState *s){std::cout << "Resumed\n\n";};
        void End(){std::cout << "End\n\n";};
        void Pause(GenericState *s){std::cout << "Paused\n\n";};
        bool Signal(SignalRef s);


        void RequestMakeTower(int x,int y,int id);
        void SendAttack(NetworkObject *obj,Point pos);
        void SendHealth(NetworkObject *obj,int health);
        void NotifyAttack(Burr *bur);
        void NotifyDamage(NetworkObject *obj,int dmg);


        static bool IsHosting();
        static bool ishosting;

        static void UpdatePaths();
        static PathFind pfer;
        static bool updateNeeded;
        static bool isBuildAble(Point p,bool checkPath=false);
        static int AttackablePoolGroup;
        static int EnemyPoolGroup;
        static int MapPoolGroup;
        static int ItemPoolGroup;
        static int BearPoolGroup;


        static std::stack<Point> LastValidPaths[4];
        void internalUpdatePaths();
        static std::stack<Point> Paths[4];
        static Point MakerPosition[4];
        static Point outPositions[4];

        static int ControllerId;
        static GameLevel *instance;
        bool MakeTower(int x,int y,int id,int from=-1);
        void SendDestroyObject(NetworkObject *obj);
        void SendCarry(NetworkObject *obj,int val,NetworkObject *seed);

        static void MakeEffect(Point pos,int dmg);

        static int GameStorage[255];
        static int SetValue(int a,int b,bool increment=false);

        void MakeEnemy();
        static WibblyWobble wibbly;

        Protocol *GetSocket(){return m_sock;};
    private:
        Shader sdds;
        Shader invrt;
        Shader blur;
        TargetTexture tr;

        std::string myName;

        TargetTexture *tar;
        float delayKek;
        LineGraph lg;

        Protocol *m_sock;

        float sc;
        Sprite bg;
        Sprite foodie;
        Sprite twr;

};
