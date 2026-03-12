// Renderer.h

#ifndef RENDERER_H
#define RENDERER_H

#include "raylib.h"
#include "../Core/Config.h"
#include <cmath>

class Renderer {


    public:

        void present(RenderTexture2D & canvas);

        void begin_texture_frame (RenderTexture2D & canvas);

        void end_texture_frame ();

        void init_canvas (RenderTexture2D & render_texture);

        

        // void rdraw_sprite();

        void rdraw_circle (float _x, float _y, float _radius, Color _color);

        



};







#endif



