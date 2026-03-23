#ifndef RENDERER_H
#define RENDERER_H

#include "raylib.h"

#include "../Overhead/Config.h"
#include "../Overhead/GameTypes.h"

#include <cmath>
#include <string>
#include <cstdarg>
#include <vector>

class GameEngine;

class Renderer {


    private:

        Vec2 camera_position = {0.0f, 0.0f};

        Font debug_font;


    public:

        void present(RenderTexture2D & canvas, GameEngine & game, float alpha);

        void begin_texture_frame (RenderTexture2D & canvas);

        void end_texture_frame ();

        void init_canvas (RenderTexture2D & render_texture);

        

        Vec2 world_camera_transform (Vec2 world_coords);
        // void rdraw_sprite();

        void rdraw_circle (float _x, float _y, float _radius, Color _color);

        void rdraw_text(const std::string & _text, int _x, int _y, int _size, Color _color);

        void rdraw_sprite(Texture2D _image, Rectangle _crop, Rectangle _paste);

        std::string text(const char* fmt, ...);

        // =====================

        // Camera Mutators

        void set_camera_position(Vec2 position);
        Vec2 get_camera_position() const;













};


#endif