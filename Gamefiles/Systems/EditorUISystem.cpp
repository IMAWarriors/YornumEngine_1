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

#include <vector>

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
                ImGui::Text("Loaded Tilesets:");

                UIPos(10, 563, 10, 547);

                ImGui::Text("________________");

                //static std::vector<std::string> items = scenes.
                //static std::vector<std::string> itempaths = assets.GetTilesetPaths("Gamefiles/Assets/Sprites/Tilesets/");

                static int selectedIndex = -1;

                static bool tilesetToPreview = false;
                static Texture2D * path = nullptr;

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

                
                for (int i = 0; i < scene.loaded_atlases.size(); i++) {
                    if (ImGui::Selectable(scene.loaded_atlases[i].name.c_str(), selectedIndex == i)) {
                        if (selectedIndex == i) {
                            selectedIndex = -1;
                        } else {
                            selectedIndex = i;
                        }
                        tilesetToPreview = true;
                        path = (scene.loaded_atlases[i].image_sheet_source);
                    }
                }
                

                ImGui::EndChild();
                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar();

                
                // =============================
                // ( + ) Add Tileset Button
                // -- Tileset Manager Button
                // ............
                // .......

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f,0.4f,0,1)); // 
                UIPos(240, 595, 240, 600 - 35);
                if (fullscreen) {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 1*fullscreenScale.y));
                } else {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 0));
                }
                ImGui::Button("+", ImVec2(30*fullscreenScale.x,(25.0f*fullscreenScale.y)));
                
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();

                // ------------------------------> 

                // =============================
                // ( - ) Delete Tileset Button
                // -- Tileset Manager Button
                // ............
                // .......

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f,0.1f,0,1)); // 
                UIPos(240, 595.0f + 28.0f, 240, (600 - 35) + (30.0f));
                if (fullscreen) {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 1*fullscreenScale.y));
                } else {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 0));
                }
                ImGui::Button("-", ImVec2(30*fullscreenScale.x,(25.0f*fullscreenScale.y)));
                
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();





                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f,0.9f,0,1)); // 
                UIPos(240, 595.0f + 56.0f, 240, (600 - 35) + (60.0f));
                if (fullscreen) {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 1*fullscreenScale.y));
                } else {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 0));
                }
                ImGui::Button("<", ImVec2(30*fullscreenScale.x,(25.0f*fullscreenScale.y)));
                
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();



                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f,0.05f,0.8f,0.5f)); // 
                UIPos(240, 595.0f + 84.0f, 240, (600 - 35) + (90.0f));
                if (fullscreen) {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 1*fullscreenScale.y));
                } else {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 0));
                }
                ImGui::Button("%", ImVec2(30*fullscreenScale.x,(25.0f*fullscreenScale.y)));
                
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();






                


                // Tileset & Tileatlas Manager

                

                if (selectedIndex != -1) {
                    UIPos(290, 558, 290, 542);
                    ImGui::Text("%s", scene.loaded_atlases[selectedIndex].name.c_str());

                    UIPos(285, 563, 285, 547);
                    ImGui::Text("___________");

                }

                

                // Left side: scrollable list

                // (Small Screen) (Fullscreen)

                if (selectedIndex != -1) {
                    
                    // DRAW PREVIEW PANE
                    if (tilesetToPreview && path != nullptr) {

                        Texture2D & texture = *path;

                        // Custom UI cursor Positioning function I have where first two coords are position in non-fullscreen, and last two are for fullscreen
                        
                        ImVec2 size;

                        float img_aspect = (float)texture.height / (float)texture.width;

                        if (texture.width > texture.height && !(texture.height > 130)) {
                            Vec2 scale;
                            scale = {225.0f, 225.0f*img_aspect};
                            size = ImVec2(scale.x * fullscreenScale.x, scale.y * fullscreenScale.y);

                            UIPos(290, 695, 290, 695);
                            ImGui::Text("%.0fpx by %.0fpx", (float)texture.width, (float)texture.height);

                        } else {
                            // For weird vertical tilesets
                            // Or square
                            
                            Vec2 scale;
                            scale = {100.0f/img_aspect, 100.0f};
                            size = ImVec2(scale.x * fullscreenScale.x, scale.y * fullscreenScale.y);

                            UIPos(290, 695, 290, 695);
                            ImGui::Text("%.0fpx by %.0fpx", (float)texture.width, (float)texture.height);
                        }

                        UIPos(290, 595, 290, 565);
                        ImGui::Image((ImTextureID)texture.id, size);

                    }


                    UIPos(500, 558, 500, 542);
                    ImGui::Text("Settings:");

                    UIPos(490, 563, 490, 547);
                    ImGui::Text("________________");


                    ImGui::PushItemWidth(85 * fullscreenScale.x);
                    

                    UIPos(480, 600, 480, 600 - 30);
                    static int tilesize = 5;
                    ImGui::SliderInt("Tiles Size (px)", &tilesize, 1, 64);

                    UIPos(480, 640, 480, 640 - 30);
                    static int column = 5;
                    ImGui::SliderInt("Tiles Per Column", &column, 1, 32);

                    UIPos(480, 680, 480, 680 - 30);
                    static int row = 5;
                    ImGui::SliderInt("Tiles Per Row", &row, 1, 32);

                    ImGui::PopItemWidth();








                    // DESELECT BUTTON
                
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f,0.1f,0.9f,1)); // 
                    UIPos(1240, 685, 1240, 685);
                    if (fullscreen) {
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 1*fullscreenScale.y));
                    } else {
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 0));
                    }
                    if (ImGui::Button("*", ImVec2(30*fullscreenScale.x,(25.0f*fullscreenScale.y)))) {
                        selectedIndex = -1;
                    }
                    
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor();

                }















                
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Pallet")) {

                static int selectedTileIndex = -1;

                

                // Loaded tileset manager so we can swap pallets

                // Tileset & Tileatlas Manager
                
                UIPos(10, 558, 10, 542);
                ImGui::Text("Loaded Tilesets:");

                UIPos(10, 563, 10, 547);

                ImGui::Text("________________");

                //static std::vector<std::string> items = scenes.
                //static std::vector<std::string> itempaths = assets.GetTilesetPaths("Gamefiles/Assets/Sprites/Tilesets/");

                static int selectedAtlas = -1;

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

                
                for (int i = 0; i < scene.loaded_atlases.size(); i++) {
                    if (ImGui::Selectable(scene.loaded_atlases[i].name.c_str(), selectedAtlas == i)) {
                        if (selectedAtlas == i) {
                            selectedAtlas = -1;
                        } else {
                            selectedAtlas = i;
                        }
                    }
                }
                

                ImGui::EndChild();
                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar();

                // -----------------------------------------------------









                
                
                UIPos(240, 563, 240, 547);
                ImGui::Text("________");

                UIPos(240, 558, 240, 542);
                ImGui::Text("Pallet:         ");

                ImGui::SameLine();

                
                ImGui::PushItemWidth(125 * fullscreenScale.x);
                    
                static int tdisplaysize = 32;
                ImGui::SliderInt("Tiles Size (px)", &tdisplaysize, 4, 64);
                ImGui::PopItemWidth();


                


                // ----------------------------------->
                //
                //

                UIPos(240, 605, 240, 570);
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f,0.2f,0.2f,0.7f));
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0,1)); // black
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f); // thickness

                if (fullscreen) {
                    ImGui::BeginChild("TileGrid", ImVec2(800 * fullscreenScale.x, 135 * fullscreenScale.y), true);
                } else {
                    ImGui::BeginChild("TileGrid", ImVec2(800 * fullscreenScale.x, 110 * fullscreenScale.y), true);
                }

                if (selectedAtlas != -1) {

                    Texture2D & tileset_texture = *scene.loaded_atlases[selectedAtlas].image_sheet_source;
                    int total_cols = (int)(scene.loaded_atlases[selectedAtlas].tiles_per_row);
                    int total_rows = (int)(scene.loaded_atlases[selectedAtlas].tiles_per_col);
                    
                    int tile_drawsize = tdisplaysize * fullscreenScale.x;

                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

                    struct RectDraw {
                        ImVec2 min;
                        ImVec2 max;
                        int r;
                        int g;
                        int b;
                        int a;
                        float thickness;
                    };

                    std::vector<RectDraw> drawQueue;
                    std::vector<RectDraw> lowerQueue;

                    for (int row = 0; row < total_rows; row++) {
                        for (int col = 0; col < total_cols; col++) {

                            Rectangle tile_slice = scene.loaded_atlases[selectedAtlas].getRectCR(col, row);

                            ImVec2 uv0 = ImVec2(
                                tile_slice.x / (float)tileset_texture.width,
                                tile_slice.y / (float)tileset_texture.height
                            );

                            ImVec2 uv1 = ImVec2(
                                (tile_slice.x + tile_slice.width) / (float)tileset_texture.width,
                                (tile_slice.y + tile_slice.height) / (float)tileset_texture.height
                            );
                            
                            ImGui::PushID(row * total_cols + col);

                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0,0,0,0.3f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0,0,0,0.2f));
                            
                            if (ImGui::ImageButton("tile", (ImTextureID)tileset_texture.id, 
                            {(float)tile_drawsize, (float)tile_drawsize}, 
                            uv0, 
                            uv1)) {
                                selectedTileIndex = row * total_cols + col;
                            }
                           
                            lowerQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 0, 0, 0, 255, 2.0f});


                            if (ImGui::IsItemHovered()) {
                                drawQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 0, 0, 225, 255, 2.0f});   
                            }

                            if (selectedTileIndex == row * total_cols + col) {
                                drawQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 0, 0, 190, 255, 3.0f});
                            }
                            
                            ImGui::PopID();

                            if (ImGui::GetItemRectMax().x < 1000*fullscreenScale.x) {
                                ImGui::SameLine();
                            }





                            
                            ImGui::PopStyleColor(3);
                        }

                
                    }

                    for (RectDraw draw : lowerQueue) {
                        ImDrawList* rdraw = ImGui::GetWindowDrawList();
                        rdraw->AddRect(draw.min, draw.max, IM_COL32(draw.r, draw.g, draw.b, draw.a), 0.0f, 0, draw.thickness);
                    }

                    for (RectDraw draw : drawQueue) {
                        ImDrawList* rdraw = ImGui::GetWindowDrawList();
                        rdraw->AddRect(draw.min, draw.max, IM_COL32(draw.r, draw.g, draw.b, draw.a), 0.0f, 0, draw.thickness);
                    }

                    drawQueue.clear();
                    ImGui::PopStyleVar(2);

                }
                
                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar();

                ImVec2 childSize = ImGui::GetWindowSize();

                
                
                if (selectedTileIndex == -1) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f,0.1f,0,1));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0.745f,1)); 
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f); // thickness
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f,0.2f,0.2f,1));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0,1)); // black
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f); // thickness
                }
                if (fullscreen) {

                    ImGui::SetCursorPos(ImVec2(
                        childSize.x - 30.0*fullscreenScale.x - 10.0,
                        childSize.y - 25.0f*fullscreenScale.y - 10.0
                    ));
                    if (ImGui::Button("-", ImVec2(30*fullscreenScale.x,(25.0f*fullscreenScale.y)))) {
                        selectedTileIndex = -1;
                    }
                } else {
                    ImGui::SetCursorPos(ImVec2(
                        childSize.x - 30.0  *   fullscreenScale.x - 25.0,
                        childSize.y - 35.0f *   fullscreenScale.y - 5.0
                    ));
                    if (ImGui::Button("-", ImVec2(45*fullscreenScale.x,(35.0f*fullscreenScale.y)))) {
                        selectedTileIndex = -1;
                    }
                }
                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar();


                ImGui::EndChild();
                
                

                //ImGui::ImageButton( (void*)(intptr_t)index, textureID, ImVec2(32, 32), uv0, uv1);

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



