#ifndef TitleH
#define TitleH

#include "state.hpp"
#include <stack>
#include "../performance/console.hpp"
#include "../performance/graph.hpp"
#include "../engine/pathfind.hpp"

#include "../socket/reliableudp.hpp"
class Title: public State{
    public:
        Title();
        ~Title();
        void Update(float);
        void Render();
        void Input();
        void Begin();
        void Resume(GenericState *s){std::cout << "Resumed\n\n";};
        void End(){std::cout << "End\n\n";};
        void Pause(GenericState *s){std::cout << "Paused\n\n";};
        bool Signal(SignalRef s);

        /*static bool isServer;
        static ReliableUdpServer server;
        static ReliableUdpClient client;*/


    private:
        bool needConnect;
        std::string name;
        bool waitName;

};
#endif
