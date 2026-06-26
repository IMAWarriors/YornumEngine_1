// GameTypes.h

#ifndef GAMETYPES_H
#define GAMETYPES_H


#include <string>
#include <cmath>

#include "raylib.h"


struct Vec2 {
    float x;
    float y;

    Vec2 operator+ (const Vec2& rhs) const {
        Vec2 result = {x + rhs.x, y + rhs.y};
        return result;
    }

    Vec2 rotate (float degrees) {
        const float radians = degrees * M_PI / 180.0f;
        const double cos_rot = std::cos(radians);
        const double sin_rot = std::sin(radians);

        return {(this->x * cos_rot) - (this->y * sin_rot),
                (this->x * sin_rot) + (this->y * cos_rot)};
    }
};

struct ColorRGBA {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};












#endif

