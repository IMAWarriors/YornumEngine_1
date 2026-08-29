#ifndef DEBUGMANAGER_H
#define DEBUGMANAGER_H

#include "../../Engine/Core/Rendering/Renderer.h"
#include "../../Engine/Core/Overhead/FrameStats.h"

#include <vector>

struct DebugData {
    std::string text;
    Vec2 position;
    int size;
    Color color;
};

struct DebugManager {

    std::vector<DebugData> queue;

    Renderer * renderer = nullptr;

    bool MODE_ONEFRAME = false;
    bool showAllInfo = false;
    bool showTileOutlines = false;
    bool showCameraClamps = false;
    bool showPhysicsBodyHitboxes = false;
    bool showHurtboxes = false;


    bool playerOverridesEnabled = true;
    bool playerGodmode = false;
    bool playerAvatarVisible = false;
    bool playerInvincible = false;
    bool playerUnexpirable = false;
    bool playerInfiniteAura = false;
    bool playerCanFly = false;

    /*ImGui::MenuItem("Avatar Visible", nullptr, & );
            ImGui::MenuItem("Invincible", nullptr, & );
            ImGui::MenuItem("Unexpirable", nullptr, & );
            ImGui::MenuItem("Infinite Aura", nullptr, & );
            ImGui::MenuItem("Can Fly", nullptr, & );*/
    
    int offset_x = 0;
    int offset_y = 0;

    void init(Renderer &_renderer) {
        renderer = &_renderer;
    }



    void push(DebugData data);
    void push(const std::string & text, Vec2 position);
    void push(const std::string & text, Vec2 position, int size);
    void push(const std::string & text, Vec2 position, Color color);

    void draw();
    void moveoffset (int x, int y);

        

};

extern DebugManager G_DEBUGGER;



#endif