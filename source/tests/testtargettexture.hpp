
#include "../settings/definitions.hpp"
#include "../engine/genericstate.hpp"
#include "../engine/renderhelp.hpp"
#include "../engine/smarttileset.hpp"
#pragma once

class Test_TargetTexture: public State{
    public:
        Test_TargetTexture(){
            requestQuit = requestDelete = false;
            tset = nullptr;
            duration = 100.0f;
        };
        ~Test_TargetTexture(){
            if (tset){
                delete tset;
            }
        };
        void Begin(){
            m_shader.Compile("engine/vertex.glvs","engine/lens.glfs");
            tileN = 174;
            tileAnim = 1.0f;
            bear::out << "Test target\n";
            background = Assets.make<Sprite>("data/wall.jpg");
            targ.Generate(400,400);
            tset = new SmartTileset(Point(32,32),Point(15,15),2,Point(320,320));
            tset->SetSprite(Assets.make<Sprite>("data/tiles.png"));
            Camera::Initiate();
            tset->SetTile(0,0,0,110);
            tset->SetTile(0,2,0,111);
            tset->SetTile(0,0,3,112);
            tset->SetTile(0,0,4,113);
            tset->SetTile(0,0,5,110);
            tset->SetTile(0,0,6,110);
            tset->SetTile(0,0,7,110);
            tset->SetTile(0,1,7,120);
            tset->SetTile(0,2,7,174);
            tset->SetTile(0,3,7,120);
            tset->SetTile(1,0,8,110);
            tset->SetTile(0,18,8,110);
            tset->SetTile(1,0,7,117);

            tset->MakeMap();

        };

        void Update(float dt){
            tset->Update(dt);

            duration -= dt;
            tileAnim -= dt;
            if (tileAnim <= 0){
                tileAnim = 1.0f;
                tileN++;
                tset->SetTileDirect(0,2,7,tileN);
                if (tileN >= 174+5){
                    tileN = 174;
                }
            }

            if( InputManager::GetInstance().IsAnyKeyPressed() != -1 || duration <= 0 ) {


                requestDelete = true;
            }


        };
        void Render(){
            //background.Render(0,0,0);
            targ.Bind();
            background.Render(0,0,45);

            #ifdef RENDER_OPENGL
            glLoadIdentity();
            glTranslatef(4,4, 0.0f);

            glBegin( GL_QUADS );
                glColor4ub( 255,255,120,240 );
                glVertex2f( 0,0 );
                glVertex2f(  32, 0 );
                glVertex2f( 32,32 );
                glVertex2f( 0,  32 );
            glEnd();
            glPopMatrix();
            #endif // RENDER_OPENGL

            RenderHelp::DrawCircleColor(Point(400-64,400 - 64),32,255,0,255,255);
            targ.UnBind();

            m_shader.Bind();
            Point p = g_input.GetMouse();
            p.y = p.y/(float)SCREEN_SIZE_H;
            p.x = p.x/(float)SCREEN_SIZE_W;
            p.y = 1.0f - p.y;
            m_shader.SetUniform<Point>("Cent2d",p);
            targ.Render(Point(32,32));

            m_shader.Unbind();
            tset->RenderLayer(0);
            tset->RenderLayer(1);

        };
        void Input();
        void Resume(){};
        void End(){
            m_shader.Close();
        };
    private:
        Shader m_shader;
        SmartTileset *tset;
        Sprite background;
        float duration;
        float tileAnim;
        int tileN;
        TargetTexture targ;
};

