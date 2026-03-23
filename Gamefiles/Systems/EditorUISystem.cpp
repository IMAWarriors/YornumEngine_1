// EditorUISystem.cpp

#include "EditorUISystem.h"

#include "../../Engine/Core/Rendering/Renderer.h"
#include "../../Engine/Core/Overhead/Config.h"
#include "../../Engine/Core/Overhead/WindowStats.h"
#include "../../Gamefiles/World/Scene.h"
#include "../../Gamefiles/World/Tile.h"
#include "../../Gamefiles/World/Overhead/Gwconst.h"
#include "../../Gamefiles/Assets/AssetManager.h"

#include "../../External/rlimgui/rlImGui.h"
#include "../../External/imgui/imgui.h"

void EditorUISystem::update (Registry & registry, float deltatime) {

    Vec2 fullscreenScale = {
                        (float)GetScreenWidth() / (float)gwconst::SCREEN_WIDTH_GAMEPIXELS,
                        (float)GetScreenHeight() / (float)gwconst::SCREEN_HEIGHT_GAMEPIXELS
    };

    bool fullscreen = GetScreenWidth() > config::INIT_WINDOW_DISPLAY_WIDTH;

    ImGui::GetStyle().WindowMinSize = ImVec2(2.0,2.0);
    ImGui::GetStyle().WindowPadding = ImVec2(0,0);

    auto UIPos = [&] (float small_x, float small_y, float fullscreen_x, float fullscreen_y) {

        if (fullscreen) {
            ImGui::SetCursorScreenPos({fullscreen_x * fullscreenScale.x, (fullscreen_y) * fullscreenScale.y});
        } else {
            ImGui::SetCursorScreenPos({small_x * fullscreenScale.x, (small_y) * fullscreenScale.y});
        }

    };

    

    // Upper Title Bar
    // ================================
    // ..........................
    // .............

    ImGui::SetNextWindowPos({0 * fullscreenScale.x, 0 * fullscreenScale.y});
    ImGui::SetNextWindowSize({1280 * fullscreenScale.x, 30 * fullscreenScale.y});

    ImGui::Begin("Scene Editor & Manager", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    
    UIPos(10, 0, 10, 10);
    ImGui::Text("Scene Editor & Manager");

    ImGui::End();



    // Workspace Area
    // ================================
    // ..........................
    // .............

    ImGui::SetNextWindowPos({0 * fullscreenScale.x, (720 - 200) * fullscreenScale.y}); // below your top bar
    ImGui::SetNextWindowSize({1280 * fullscreenScale.x, 200 * fullscreenScale.y});

    if (ImGui::Begin("Workspace", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar )) {

        ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4                  (0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4           (0.33f, 0.33f, 0.33f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4            (0.20f, 0.20f, 0.20f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TabUnfocused, ImVec4         (0.08f, 0.08f, 0.08f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, ImVec4   (0.15f, 0.15f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4               (0,0,0,0));       // removes window borders

        // optional: kill border thickness entirely
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 20.0f);

        if (ImGui::BeginTabBar("WorkspaceTabs")) {

            if (ImGui::BeginTabItem("Tileset")) {

                // Tileset & Tileatlas Manager

                UIPos(10, 558, 10, 542);
                ImGui::Text("Tileset Manager");

                UIPos(10, 563, 10, 547);

                ImGui::Text("________________");

                static std::vector<std::string> items = assets.GetTilesetFilenames("Gamefiles/Assets/Sprites/Tilesets/");
                static std::vector<std::string> itempaths = assets.GetTilesetPaths("Gamefiles/Assets/Sprites/Tilesets/");

                static int selectedIndex = -1;

                bool tilesetToPreview = false;
                std::string path;

                // Left side: scrollable list

                // (Small Screen) (Fullscreen)
                UIPos(10, 595, 10, 565);
                ImGui::GetStyle().WindowPadding = ImVec2(6,6);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f,0.0f,0.0f,1.0f));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1,1,1,1));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0,1)); // black
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f); // thickness

                if (fullscreen) {
                    ImGui::BeginChild("ItemListPanel", ImVec2(220 * fullscreenScale.x, 145 * fullscreenScale.y), true);
                } else {
                    ImGui::BeginChild("ItemListPanel", ImVec2(220 * fullscreenScale.x, 110 * fullscreenScale.y), true);
                }

                for (int i = 0; i < items.size(); i++) {
                    if (ImGui::Selectable(items[i].c_str(), selectedIndex == i)) {
                        selectedIndex = i;
                        tilesetToPreview = true;
                        path = itempaths[i];
                    }
                }

                ImGui::EndChild();
                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar();




                // ------------------------------> 


                // Tileset & Tileatlas Manager

                UIPos(260, 558, 240, 542);
                ImGui::Text("Preview");

                UIPos(255, 563, 235, 547);

                ImGui::Text("________________");

                // Left side: scrollable list

                // (Small Screen) (Fullscreen)
                UIPos(260, 595, 240, 565);
                
                if (tilesetToPreview) {
                    
                    Texture2D & texture = assets.LoadTilesetTexture(path);

                    float scale = std::min(200.0f / texture.width, 200.0f / texture.height);
                    ImVec2 size = ImVec2(texture.width * scale, texture.height * scale);

                    ImGui::Image((ImTextureID)texture.id, size);

                }

                
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Layer")) {
                // EMPTY
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Spawndefs")) {
                // EMPTY
                ImGui::EndTabItem();
            }

            

            ImGui::EndTabBar();

            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(6);
        }
    }
    ImGui::End();

}



