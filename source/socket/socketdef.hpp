#pragma once
#ifndef __EMSCRIPTEN__
#ifndef DISABLE_SOCKET
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <queue>
#include <map>
#include <functional>
#include <list>
#ifdef _WIN32
#include <winsock2.h>
typedef sockaddr_in SocketAddr;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
typedef struct sockaddr_in SocketAddr;
#endif // _WIN32


template <int MsgSize=1024> class SocketMessage_{
    public:
        SocketMessage_():m_pointer(0),m_readPointer(0){
            for (int i=0;i<MsgSize;i++){
                StaticStream[i] = 0;
            }
        };
        SocketMessage_(const char *c):SocketMessage_(){
            SetStream((char *)c);
        }
        uint32_t GetMaxBuffer(){
            return MsgSize;
        }
        void SetSize(uint16_t size){
            m_pointer = size;
            m_readPointer = 0;

        };
        void Clear(){
            m_readPointer = 0;
            m_pointer = 0;
        }
        bool SetStream(char *stream,uint16_t size=0){
            if (!stream){
                return false;
            }
            if (size >= MsgSize){
                return false;
            }
            if (size == 0){
                size = strlen(stream);
            }
            memcpy(StaticStream,stream,size);
            m_pointer = size;
            m_readPointer = 0;
            return true;
        };

        char *GetStream(uint16_t &size){
            size = m_pointer;
            return StaticStream;
        };

        char *GetStream(){
            return StaticStream;
        };


        bool Skip(uint16_t val=1){
            if (m_readPointer+val >= MsgSize){
                return false;
            }
            m_readPointer += val;
            return true;
        }

        char Peek(){
            return StaticStream[m_readPointer];
        }

        char ReadByte(){
            char ret = StaticStream[m_readPointer];
            m_readPointer++;
            return ret;
        }

        bool AddString(const char *c){
            if (m_pointer+strlen(c) >= MsgSize){
                return false;
            }
            for (int i=0;c[i] != '\0';i++){
                StaticStream[m_pointer] = c[i];
                m_pointer++;
            }
            StaticStream[m_pointer] = '\0';
            m_pointer++;
            return true;
        };
        std::string GetString(){
            std::string str;
            char c = ReadByte();
            while (c != '\0'){
                str.push_back(c);
                c = ReadByte();
                if (m_readPointer >= m_pointer){
                    break;
                }
            }
            str.push_back('\0');
            return str;
        };

        bool AddByte(char b){
            if (m_pointer >= MsgSize){
                return false;
            }
            StaticStream[m_pointer] = b;
            m_pointer++;
            return true;
        }

        template<typename T>bool Add(T obj){
            if (m_pointer+sizeof(T) >= MsgSize){
                return false;
            }
            T *addr = (T*)&StaticStream[m_pointer];
            m_pointer += sizeof(T);
            memcpy(addr,&obj,sizeof(T));
            return true;
        }

        template<typename T>T Get(){
            if (m_readPointer+sizeof(T) >= m_pointer){
                return T();
            }
            T ret = *((T*) (StaticStream + m_readPointer) );
            m_readPointer += sizeof(T);
            return ret;
        }


        uint16_t GetMessageSize(){
            return m_pointer;
        }
        uint16_t Seek(uint16_t pos = 0){
            uint16_t myPos = m_readPointer;
            m_readPointer = pos;
            return Seek;
        }
    private:
        uint16_t m_pointer,m_readPointer;
        char StaticStream[MsgSize];
};
typedef std::list<int> PeerList;

typedef SocketMessage_<1024> SocketMessage;

class Connection{
    public:
        virtual void Update(float dt) = 0;
        virtual bool Receive(SocketMessage *msg,int pid=-1) = 0;
        virtual bool ReceiveBytes(SocketMessage *msg,uint16_t amount,int pid=-1) = 0;
        virtual bool Send(SocketMessage *msg,int pid=-1) = 0;
        virtual bool IsConnected(){return false;};
        virtual uint32_t GetId(){ return m_id;};
        virtual void SetId(uint32_t id){ m_id = id;};
        virtual PeerList GetPeers(){ return peerIds;};
        virtual int GetPeersOnline(){ return peerIds.size();};
        virtual void SetDisconectFunction(std::function<void(int)> dc){ OnDisconect = dc;};
    protected:
        std::function<void(int)> OnDisconect;
        PeerList peerIds;
        uint32_t m_id;
};

class SocketClient: public Connection{
    public:
        #ifdef _WIN32
        SocketClient():m_SocketHandler(INVALID_SOCKET){};
        #else
        SocketClient():m_SocketHandler(-1){};
        #endif // _WIN32
        virtual bool Connect(std::string addr,uint16_t port,int duration) = 0;

    protected:
        #ifdef _WIN32
        SOCKET m_SocketHandler;
        #else
        int m_SocketHandler;
        #endif // _WIN32
        SocketAddr m_serverAddr;

};

class SocketHost: public Connection{
    public:
        SocketHost():m_lastPid(0){};
        virtual bool Bind(uint16_t port) = 0;
    protected:
        uint64_t m_lastPid;
        std::map<uint64_t,std::queue<SocketMessage>> m_messages;
};

class BaseSocket{
    public:
        static bool StartSocket();
};

#endif // DISABLE_SOCKET
#endif // __EMSCRIPTEN__
