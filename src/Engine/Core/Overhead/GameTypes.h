// GameTypes.h

#ifndef GAMETYPES_H
#define GAMETYPES_H


#include <string>

#include "raylib.h"


struct Vec2 {
    float x;
    float y;

    Vec2 operator+ (const Vec2& rhs) const {
        Vec2 result = {x + rhs.x, y + rhs.y};
        return result;
    }
};

struct ColorRGBA {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};












#endif

