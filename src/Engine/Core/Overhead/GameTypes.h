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

    Vec2 rotated (float degrees) const {
        const float radians = degrees * (float)M_PI / 180.0f;
        const float cos_rot = std::cos(radians);
        const float sin_rot = std::sin(radians);

        return {(x * cos_rot) - (y * sin_rot),
                (x * sin_rot) + (y * cos_rot)};
    }

    void rotate (float degrees) {
        *this = rotated(degrees);
    }
};

struct ColorRGBA {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};












#endif

