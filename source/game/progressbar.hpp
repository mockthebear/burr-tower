#pragma once

#include "../engine/object.hpp"

class ProgressBar{
    public:
        ProgressBar();
        ProgressBar(float duration,float start = 0.0f,int fancy=0);
        void SetSize(int i){size = i ;};
        void SetMinMax(float min,float max);
        void SetValue(float v);

        void Finish(){
            progress = duration;
        }
        void Reset();
        bool IsDone();
        float GetPercent();

        void Update(float dt);
        void Render(GameObject *obj,Point p = Point(0,0));
    private:
        int size;
        int fancy;
        float mim,mam;
        bool onValue;
        float duration;
        float progress;

};
