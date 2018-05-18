
#include "../settings/definitions.hpp"

#include "threadpool.hpp"
#include "../engine/light.hpp"
#include "../framework/utils.hpp"

#include "../settings/configmanager.hpp"
#include "../performance/console.hpp"
#include "../engine/gamebase.hpp"
#include <math.h>
#include <stdlib.h>
#ifndef DISABLE_THREADPOOL
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#endif
#include <iostream>





ThreadPool& ThreadPool::GetInstance(int threads){
    static ThreadPool teste(threads);
    return teste;
}

void ThreadPool::sleep(float milliseconds) // cross-platform sleep function
{
    clock_t time_end;
    time_end = clock() + milliseconds * CLOCKS_PER_SEC/1000;
    while (clock() < time_end){}
}

bool ThreadPool::Begin(int threads){
    if (started){
        bear::out << "[ThreadPool] Attempt to create thread when there are already threads!\n";
        return false;
    }
    #ifndef DISABLE_THREADPOOL
    UsePThreads=threads;
    thread_pool = new  pthread_t[UsePThreads];
    mutexes = new  sem_t [UsePThreads];
    runningM = new  sem_t [UsePThreads];
    Params = new  parameters[UsePThreads];

    started = false;
    pthread_mutex_init(&Critical,NULL);
    for (int i=0;i<UsePThreads;i++){
        Params[i].id = i;
        Params[i].me = (void*)this;
        Params[i].working = false;
        Params[i].mainThread = false;
        Params[i].Threads = UsePThreads;
        sem_init(&mutexes[i], 0,1);
        sem_init(&runningM[i], 0,1);
        sem_wait (&runningM[i]);
        //pthread_mutex_unlock(&mutexes[i]);
        //pthread_mutex_lock(&runningM[i]);
    }
    Locked = true;
    #else
    bear::out << "Threads are disabled.\n";
    UsePThreads=1;
    Params = new  parameters[UsePThreads];
    Params[0].id = 0;
    Params[0].me = (void*)this;
    Params[0].Threads = UsePThreads;
    #endif

    CreateThreads();
    return true;
}


ThreadPool::ThreadPool(int threads)
#ifdef USE_LOCK_FREE_STACK
    :Jobs(threads,16)
#endif
{
    Echo = false;
    DynamicJobs = false;
    scalonator = 0;
    Light::Startup();
    Begin(threads);

}
bool ThreadPool::CreateThreads(){
    if (!started){
        started = true;
        #ifndef DISABLE_THREADPOOL
        if (Echo)
            bear::out << "[ThreadPool] Creating "<<UsePThreads<<" threads.\n";
        for (int i=0;i<UsePThreads;i++){
            pthread_create(&thread_pool[i], NULL, &ThreadPool::thread_pool_worker, (void*)&Params[i]);
            //sem_wait(&runningM[i]);
        }

        Lock();
        if (Echo)
            bear::out << "[ThreadPool] Done.\n";
        #endif
        return true;
    }
    return false;
}
bool ThreadPool::KillThreads(){
    if (started){
        DynamicJobs = true;
        if (Echo)
            bear::out << "Killing threads\n";
        #ifndef DISABLE_THREADPOOL
        Job kill = Job(JOB_KILL);
        for (int i=0;i<UsePThreads;i++){
            AddJob_(kill);
        }
        Unlock();
        for (int e=0;e<UsePThreads;e++){
            pthread_mutex_lock(&Critical);
            if (Echo)
                bear::out << "Waiting "<<e<<"\n";
            pthread_mutex_unlock(&Critical);
            pthread_join(thread_pool[e], NULL);
        }
        #endif
        started = false;
        return true;
    }
    return false;
}

ThreadPool::~ThreadPool(){

    if (started){
        bear::out << "[ThreadPool] Closing threads!\n";
        KillThreads();
        bear::out << "[ThreadPool] Bye!\n";
    }

}

void ThreadPool::AddLambdaJob(std::function<void(int,int,void*)> F){
    Job newJ;
    newJ.lambda = F;
    newJ.Type = JOB_LAMBDA;
    AddJob_(newJ);
}

void ThreadPool::AddJobStack(Job &j){
    #ifdef USE_LOCK_FREE_STACK
    Jobs.Push(j);
    #else
    Jobs.push(j);
    #endif


}
void ThreadPool::AddJob_(Job &j,bool ignoreFast){

    AddJobStack(j);

}


void ThreadPool::AddParallelFor(std::function<void(int,int,void*)> F,int min,int max,int jobs){
    if (jobs <= 0){
        jobs = UsePThreads;
    }
    for (int i=0;i<jobs;i++){
       Job newJ(F,min + (((float)max/(float)jobs)*(float)i),min + (((float)max/(float)jobs)*(float)(i+1)) );
       AddJob_(newJ);
    }
}



void ThreadPool::ClearJobs(){

    //Jobs.clear();

}

bool ThreadPool::Help(){

    if (Jobs.empty()){
        return false;
    }
    bear::out << "Helping\n";

    parameters Param;
    Param.id = UsePThreads;
    Param.me = (void*)this;
    Param.working = true;
    Param.mainThread = true;
    Param.Threads = UsePThreads;
    thread_pool_worker(&Param);
    return true;
};

void ThreadPool::Lock(){
    if (!started){
        bear::out << "[ThreadPool]Lock/Unlock with no threads running.\n";
        return;
    }
    Locked = true;
    #ifndef DISABLE_THREADPOOL
    for (int e=0;e<UsePThreads;e++){
        sem_wait (&runningM[e]);
    }

    #endif
}
bool ThreadPool::TryLock(){
    if (!started){
        bear::out << "[ThreadPool]Lock/Unlock with no threads running.\n";
        return false;
    }
    #ifndef DISABLE_THREADPOOL


    //Locked = true;
    //bool done = false;





    #endif
    return true;
}
void ThreadPool::Unlock(){
    if (!started){
        bear::out << "[ThreadPool]Lock/Unlock with no threads running.\n";
        return;
    }
    Locked = false;
    #ifndef DISABLE_THREADPOOL
    for (int e=0;e<UsePThreads;e++){
        sem_post(&mutexes[e]);
    }
    #else
    ThreadPool::thread_pool_worker(&Params[0]);
    #endif

}

bool ThreadPool::IsDone(){
    bool finished = false;
    if (Jobs.empty()){
        finished = true;
    }
    if (finished)
        return true;
    return false;

}
void ThreadPool::Update(float dt){}

void ThreadPool::CriticalLock(){
    #ifndef DISABLE_THREADPOOL
    pthread_mutex_lock(&Critical);
    #endif // DISABLE_THREADPOOL
}
void ThreadPool::CriticalUnLock(){
    #ifndef DISABLE_THREADPOOL
    pthread_mutex_unlock(&Critical);
    #endif // DISABLE_THREADPOOL
}

bool ThreadPool::SpreadJobs(){
    #ifndef DISABLE_THREADPOOL
    pthread_mutex_lock(&Critical);

    while (Jobs.size() > 0){
        int t = scalonator%UsePThreads;
        scalonator++;
        Params[t].MyJobs.emplace(Jobs.top());
        Jobs.pop();
    }

    pthread_mutex_unlock(&Critical);
    #endif // DISABLE_THREADPOOL
    return true;
}

void *ThreadPool::thread_pool_worker(void *OBJ){
    parameters *P = (parameters*)OBJ;
    ThreadPool *This = (ThreadPool*)P->me;
    #ifndef GL_LIGHT
    Light *luz = Light::GetInstance();
    #endif // GL_LIGHT
    P->working = false;
    unsigned long int iteration = 0;
    while(true){
        //Call once
        #ifndef DISABLE_THREADPOOL
        if (!P->mainThread)
            sem_wait (&This->mutexes[P->id]);
        #endif
        P->working = true;

        while (P->working){
            Job &todo = P->todo;
            if (todo.Type == JOB_NOTHING){
                iteration++;
                if (P->MyJobs.size() > 0){
                    todo = P->MyJobs.top();
                    P->MyJobs.pop();
                    P->working = true;
                }else if (This->DynamicJobs){
                    #ifndef DISABLE_THREADPOOL
                    pthread_mutex_lock(&This->Critical);
                    #endif // DISABLE_THREADPOOL
                    if (todo.Type == JOB_NOTHING){
                        if (This->Jobs.size() > 0){
                            todo = This->Jobs.top();
                            This->Jobs.pop();

                            P->working = true;
                        }else{
                            P->working = false;
                            if (P->mainThread){
                                return nullptr;
                            }
                        }
                    }
                    #ifndef DISABLE_THREADPOOL
                    pthread_mutex_unlock(&This->Critical);
                    #endif // DISABLE_THREADPOOL
                }else{
                    P->working = false;
                }
            }

            if (todo.Type != JOB_NOTHING){
                if (todo.Type == JOB_LAMBDA){
                    todo.lambda(P->id,P->Threads,NULL);
                }else if (todo.Type == JOB_FOR){
                    todo.lambda(todo.min,todo.max,This);
                }else if (todo.Type == JOB_KILL){
                    #ifndef DISABLE_THREADPOOL
                    pthread_mutex_lock(&This->Critical);
                    if (This->Echo)
                        bear::out << "[ThreadPool]{Thread:"<< P->id << "} closing.\n";
                    if (!P->mainThread)
                        sem_post(&This->runningM[P->id]);
                    pthread_mutex_unlock(&This->Critical);
                    if (!P->mainThread)
                        pthread_exit(NULL);
                    else
                        return nullptr;
                    #endif // DISABLE_THREADPOOL
                }else if (todo.Type == JOB_SHADE){
                    #ifndef GL_LIGHT
                    luz->Shade(P,todo);
                    #endif // GL_LIGHT
                }else if (todo.Type == JOB_REDUCE){
                    #ifndef GL_LIGHT
                    luz->Reduce(P,todo);
                    #endif // GL_LIGHT
                }else if (todo.Type == JOB_GEN){
                    #ifndef GL_LIGHT
                    luz->Gen(P,todo);
                    #endif // GL_LIGHT
                }
                todo.Type = JOB_NOTHING;
            }else{
                #ifdef DISABLE_THREADPOOL
                    return NULL;
                #endif
            }
        }
        #ifndef DISABLE_THREADPOOL
        if (!P->mainThread){
            sem_post(&This->runningM[P->id]);
        }
        #endif
    }
    return NULL;
}

