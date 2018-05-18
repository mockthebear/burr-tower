#include "progressbar.hpp"
#include "../engine/renderhelp.hpp"
#include "../framework/geometry.hpp"


ProgressBar::ProgressBar(){
    duration = 1.0f;
    progress = 0.0f;
    size = 32;
};
ProgressBar::ProgressBar(float duration,float start,int vfancy){
    this->duration = duration;
    this->progress = start;
    onValue = false;
    fancy = vfancy;
    size = 32;
}

void ProgressBar::Reset(){
    progress = 0;
};

void ProgressBar::SetValue(float mm){
    mim = mm;
}
void ProgressBar::SetMinMax(float m1,float m2){
    mim = m1;
    mam = m2;
    onValue = true;
}


bool ProgressBar::IsDone(){
    if (!onValue)
        return  (progress >= duration);
    else
        return false;

};

float ProgressBar::GetPercent(){
    if (!onValue)
        return std::min(progress/duration,1.0f);
    else
        return std::min(mim/mam,1.0f);
};

void ProgressBar::Update(float dt){
    if (!onValue)
        progress += dt;
};
void ProgressBar::Render(GameObject *obj,Point offset){
    if (obj && !IsDone()){
        Rect outline(0,0,size,8);
        Rect inside(0,0,size,8);
        outline.CenterRectIn(obj->box);
        inside.CenterRectIn(obj->box);
        float perc = GetPercent();

        inside.w = size * perc;

        inside.y  -= obj->box.h;
        outline.y -= obj->box.h;

        inside.y += offset.y;
        inside.x += offset.x;

        outline.y += offset.y;
        outline.x += offset.x;


        RenderHelp::DrawSquareColorA(inside,(1.0f-perc)*255,(perc)*255,0,255,false);
        RenderHelp::DrawSquareColorA(outline,0,0,0,255,true);
        if (fancy > 0){
            for (int i=0;i<fancy;i++){
                float posX = i/(float)fancy;
                Point pos( outline.x  + posX*outline.w,  outline.y );
                Point pos2( pos.x,  outline.y + outline.h );
                RenderHelp::DrawLineColor(pos,pos2,0,0,0,255);
            }
        }
    }
};
