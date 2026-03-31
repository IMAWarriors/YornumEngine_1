// EditorUISystem.cpp

#include "EditorUISystem.h"

#include "../../Engine/Core/Rendering/Renderer.h"
#include "../../Engine/Core/Overhead/Config.h"
#include "../../Engine/Core/Overhead/WindowStats.h"
#include "../../Gamefiles/World/Scene.h"
#include "../../Gamefiles/World/Tile.h"
#include "../../Gamefiles/World/Overhead/Gwconst.h"
#include "../../Gamefiles/Assets/AssetManager.h"
#include "../../Gamefiles/Assets/EditorAssets.h"

#include "../../External/rlimgui/rlImGui.h"
#include "../../External/imgui/imgui.h"



#include <vector>
#include <algorithm>

#include <cmath>
#include <cctype>
#include <cstdio>

#include <filesystem>



void EditorUISystem::update (Registry & registry, float deltatime) {

    Vec2 fullscreenScale = {
                        (float)GetScreenWidth() / (float)gwconst::SCREEN_WIDTH_GAMEPIXELS,
                        (float)GetScreenHeight() / (float)gwconst::SCREEN_HEIGHT_GAMEPIXELS
    };

    bool fullscreen = GetScreenWidth() > config::INIT_WINDOW_DISPLAY_WIDTH;

    static bool showTileAtlasEditor = false;
    static bool showLayerManager = true;

    static int selectedLayer = 0;

    static int selectedIndex = -1; // FOR line: if(ImGui::BeginTabItem("Tileset")) {...  // serves to give show selected tile atlas for tile atlas editor

    static bool animParamsMatch = true;

    static int phystab_selectedTileIndex = -1;

    static bool newTilesetSplitMatch = true;

    static int anim_frames = 1;
    static float frame_time = 0.0f;

    ImGui::GetStyle().WindowMinSize = ImVec2(2.0,2.0);
    ImGui::GetStyle().WindowPadding = ImVec2(0,0);

    auto UIPos = [&] (float small_x, float small_y, float fullscreen_x, float fullscreen_y) {

        if (fullscreen) {
            ImGui::SetCursorScreenPos({fullscreen_x * fullscreenScale.x, (fullscreen_y) * fullscreenScale.y});
        } else {
            ImGui::SetCursorScreenPos({small_x * fullscreenScale.x, (small_y) * fullscreenScale.y});
        }

    };

    // =========================================================================================
    //      Tile Atlas Editor: Resizable Movable Window
    // -----------------------------------------------------
    // ** FEATURES **
    //  - Adjusting tile collisions
    //  - Resplitting tile atlas
    // =========================================================================================


    auto DrawSceneSettings = [&] () {

        ImVec2 orig = ImGui::GetStyle().WindowPadding;
        ImGui::GetStyle().WindowPadding = ImVec2(4,4);



        // Anchor to top-right corner
            ImVec2 windowSize = ImVec2(160 * fullscreenScale.x, 80 * fullscreenScale.y);
            ImVec2 windowPos = ImVec2(
                GetScreenWidth() - windowSize.x - 350.0f,
                30.0f * fullscreenScale.y
            );

            ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

            ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.2f, 0.2f, 0.7f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.1f, 0.1f, 0.3f, 1.0f));

            if (ImGui::Begin(scene.loaded_scene_name.c_str(), nullptr,
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse)) {
                scene.uiCapturesMouse |= ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

                // =========================
                // Layer List
                // =========================
                
            }

            ImGui::End();
            ImGui::PopStyleColor(3);
            ImGui::GetStyle().WindowPadding = orig;

    };

    auto DrawLayerManager = [&] () {

        ImVec2 orig = ImGui::GetStyle().WindowPadding;
        ImGui::GetStyle().WindowPadding = ImVec2(4,4);



        // Anchor to top-right corner
            ImVec2 windowSize = ImVec2(220 * fullscreenScale.x, 300 * fullscreenScale.y);
            ImVec2 windowPos = ImVec2(
                GetScreenWidth() - windowSize.x - 10.0f,
                30.0f * fullscreenScale.y
            );

            ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

            ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.2f, 0.2f, 0.7f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.1f, 0.1f, 0.3f, 1.0f));

            if (ImGui::Begin("Layer Manager", nullptr,
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse)) 
            {
                scene.uiCapturesMouse |= ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

                // =========================
                // Layer List
                // =========================
                ImGui::Text("Layers:");
                ImGui::Separator();

                ImGui::BeginChild("LayerList", ImVec2(0, 180 * fullscreenScale.y), true);

                for (int i = 0; i < (int)scene.tile_layers.size(); i++) {

                    std::string label = "Layer " + std::to_string(i);

                    if (ImGui::Selectable(label.c_str(), selectedLayer == i)) {
                        selectedLayer = i;
                    }
                }

                ImGui::EndChild();

                // =========================
                // Buttons
                // =========================

                // Add Layer
                if (ImGui::Button("+ Add Layer", ImVec2(-1, 0))) {
                    scene.tiles_push_new_layer();
                    selectedLayer = (int)scene.tile_layers.size() - 1;
                }

                // Delete Layer
                if (ImGui::Button("- Delete Layer", ImVec2(-1, 0))) {
                    if (!scene.tile_layers.empty() && selectedLayer >= 0) {
                        scene.tile_layers.erase(scene.tile_layers.begin() + selectedLayer);

                        if (selectedLayer >= (int)scene.tile_layers.size()) {
                            selectedLayer = (int)scene.tile_layers.size() - 1;
                        }
                    }
                }

                // =========================
                // Info
                // =========================
                ImGui::Separator();
                ImGui::Text("Editing Layer: %d", selectedLayer);

                // 🔴 THIS IS IMPORTANT:
                // expose this to rest of editor
                scene.EDITOR_ONLY_SELECTED_LAYER = selectedLayer;
            }

            ImGui::End();
            ImGui::PopStyleColor(3);
            ImGui::GetStyle().WindowPadding = orig;
    };


    auto DrawTileAtlasEditor = [&] () {

        ImGui::SetNextWindowPos({240 * fullscreenScale.x, (50) * fullscreenScale.y}, ImGuiCond_Once); // below your top bar
        ImGui::SetNextWindowSize({900 * fullscreenScale.x, 350 * fullscreenScale.y}, ImGuiCond_Once);

        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0,0,0.745f,1)); // active color
        ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0,0,0.445f,1));

        if (ImGui::Begin("Tile Atlas Editor")) {

            // ********************* TILE ATLAS EDITOR WINDOW CODE ******************************

            scene.EDITOR_ONLY_ACTIVE_TAEDITOR = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

            ImVec2 window_size = ImGui::GetWindowSize();
            ImVec2 window_position = ImGui::GetWindowPos();

            ImVec2 content = ImGui::GetContentRegionAvail();

            // position relative inside window
            ImGui::SetCursorPos(ImVec2(
                content.x * 0.05f,
                content.y * 0.1f + 20.0f
            ));

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f,0.2f,0.2f,0.7f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0,1)); // black
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f); // thickness

            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImGui::BeginChild("TileGrid", ImVec2(avail.x * 0.5f, avail.y * 0.8f), true);

            if (selectedIndex != -1) {

                Texture2D & tileset_texture = *scene.loaded_atlases[selectedIndex].image_sheet_source;
                int total_cols = (int)(scene.loaded_atlases[selectedIndex].tiles_per_row);
                int total_rows = (int)(scene.loaded_atlases[selectedIndex].tiles_per_col);
                
                float tile_drawsize = 32.0f;

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

                        Rectangle tile_slice = scene.loaded_atlases[selectedIndex].getRectCR(col, row);

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
                        {tile_drawsize, tile_drawsize}, 
                        uv0, 
                        uv1)) {

                            // What to do when someone selects a tile
                            
                        }
                        
                        lowerQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 0, 0, 0, 255, 2.0f});


                        if (ImGui::IsItemHovered()) {
                            drawQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 0, 0, 225, 255, 2.0f});   
                        }

                        /*handle seelcting
                        if (selectedTileIndex == row * total_cols + col) {
                            drawQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 0, 0, 190, 255, 3.0f});
                        }
                            */
                        
                        ImGui::PopID();

                        if (col < total_cols-1) {
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

            ImGui::EndChild();








            // ***********************************************************************************


            

        }

        ImGui::PopStyleColor(3);

        ImGui::End();

    };


    // <------------------------------------------- Tile Atlas Editor
    

    // Upper Title Bar
    // ================================
    // ..........................
    // .............

    /*
    ImGui::SetNextWindowPos({0 * fullscreenScale.x, 0 * fullscreenScale.y});
    ImGui::SetNextWindowSize({1280 * fullscreenScale.x, 30 * fullscreenScale.y});

    ImGui::Begin("Scene Editor & Manager", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    
    UIPos(10, 0, 10, 10);
    ImGui::Text("Scene Editor & Manager");

    ImGui::End();
    */

    static std::string saveSceneNameInput;
    static int saveSceneSelectedIndex = -1;
    static bool saveSceneInitialized = false;

    static int openSceneSelectedIndex = -1;

    ImGui::GetStyle().FramePadding = ImVec2(4, 4);

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {}
            if (ImGui::MenuItem("Open")) {
                openSceneSelectedIndex = -1;
                ImGui::OpenPopup("Open Scene");
            }
            if (ImGui::MenuItem("Save")) {
                saveSceneInitialized = false;
                ImGui::OpenPopup("Save Scene");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y")) {}
            ImGui::EndMenu();
        }


        ImGui::EndMainMenuBar();

    }

    std::filesystem::create_directories("Gamefiles/Scenes/");

    // Save modal or wtv
    auto strip_ext = [](const std::string& s) {
        size_t pos = s.find_last_of('.');
        return (pos == std::string::npos) ? s : s.substr(0, pos);
    };

    ImVec2 saveSceneModalSize = {
        std::max(620.0f, GetScreenWidth() * 0.72f),
        std::max(420.0f, GetScreenHeight() * 0.76f)
    };


    ImGui::SetNextWindowSize(saveSceneModalSize, ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Save Scene", NULL, ImGuiWindowFlags_NoCollapse)) {

        std::vector<std::string> scenepaths = assets.GetFilepathsInDirectory("Gamefiles/Scenes/", "scene");
        std::vector<std::string> scenenames = assets.GetFilenamesInDirectory("Gamefiles/Scenes/", "scene");

        if (!saveSceneInitialized) {
            saveSceneNameInput = scene.loaded_scene_name;
            saveSceneSelectedIndex = -1;

            for (int i = 0; i < (int)scenenames.size(); i++) {
                if (strip_ext(scenenames[i]) == strip_ext(saveSceneNameInput)) {
                    saveSceneSelectedIndex = i;
                    break;
                }
            }
            saveSceneInitialized = true;
        }

        ImGui::Text("Choose filename of scene to create or overwrite:");
        ImGui::BeginChild("SaveSceneList", ImVec2(0, saveSceneModalSize.y * 0.28f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
        
        for (int i = 0; i < (int)scenepaths.size(); i++) {
            if (ImGui::Selectable(scenepaths[i].c_str(), saveSceneSelectedIndex == i, ImGuiSelectableFlags_DontClosePopups)) {
                saveSceneSelectedIndex = i;
                saveSceneNameInput = strip_ext(scenenames[i]);

            }
        }

        ImGui::EndChild();

        char filenameBuffer[128];
        std::snprintf(filenameBuffer, sizeof(filenameBuffer), "%s", saveSceneNameInput.c_str());
        if (ImGui::InputText("Scene Name", filenameBuffer, sizeof(filenameBuffer))) {
            saveSceneNameInput = filenameBuffer;
            saveSceneSelectedIndex = -1;
        }

        const bool existingFileSelected = (saveSceneSelectedIndex >= 0 && saveSceneSelectedIndex < (int)scenepaths.size());

        if (existingFileSelected) {
            ImGui::Text("Warning! Filename selected already exists in directory and will overwrite scene data.");
            ImGui::Text("(If you are intentionally saving updates on an old scene, ignore this message.)");
        }

        if (ImGui::Button("Confirm", ImVec2(120, 0))) {
            std::string finalPath;

            if (existingFileSelected) {
                finalPath = scenepaths[saveSceneSelectedIndex];
            } else {
                finalPath = "Gamefiles/Scenes/" + strip_ext(saveSceneNameInput) + ".scene";
            }

            if (!saveSceneNameInput.empty()) {
                scene.loaded_scene_name = strip_ext(saveSceneNameInput);
               
                bool success = scene.save_scene(finalPath);
                
                std::cout << (success ? "SAVE OK\n" : "SAVE FAILED\n");

                

                ImGui::CloseCurrentPopup();

                saveSceneInitialized = false;

            }

        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            saveSceneInitialized = false;
        }

        ImGui::EndPopup();
    }

    ImGui::SetNextWindowSize(saveSceneModalSize, ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Open Scene", NULL, ImGuiWindowFlags_NoCollapse)) {

        std::vector<std::string> scenepaths = assets.GetFilepathsInDirectory("Gamefiles/Scenes/", "scene");

        ImGui::Text("Choose a scene file to load:");
        ImGui::BeginChild("OpenSceneList", ImVec2(0, saveSceneModalSize.y * 0.36f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

        for (int i = 0; i < (int)scenepaths.size(); i++) {
            if (ImGui::Selectable(scenepaths[i].c_str(), openSceneSelectedIndex == i, ImGuiSelectableFlags_DontClosePopups)) {
                openSceneSelectedIndex = i;
            }
        }

        ImGui::EndChild();

        if (openSceneSelectedIndex >= 0 && openSceneSelectedIndex < (int)scenepaths.size()) {
            ImGui::Text("Selected: %s", scenepaths[openSceneSelectedIndex].c_str());
        } else {
            ImGui::Text("Selected: (none)");
        }

        if (ImGui::Button("Load", ImVec2(120, 0))) {
            if (openSceneSelectedIndex >= 0 && openSceneSelectedIndex < (int)scenepaths.size()) {
                if (scene.load_scene(scenepaths[openSceneSelectedIndex], assets)) {
                    scene.loaded_scene_name = strip_ext(std::filesystem::path(scenepaths[openSceneSelectedIndex]).filename().string());
                }
                ImGui::CloseCurrentPopup();
                openSceneSelectedIndex = -1;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            openSceneSelectedIndex = -1;
        }

        ImGui::EndPopup();

    }





    


    



    ImGui::GetStyle().FramePadding = ImVec2(4, 4);

    
    if (showTileAtlasEditor) {
        DrawTileAtlasEditor();
    }

    if (showLayerManager) {

        DrawLayerManager();

    }







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

                ImGui::Text("_______________");

                //static std::vector<std::string> items = scenes.
                //static std::vector<std::string> itempaths = assets.GetTilesetPaths("Gamefiles/Assets/Sprites/Tilesets/");

               

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

                // ***** Tileset splitting information *****

                static int tilesize = 5;
                static int split_columns = 5;   // horizontal tile count (tiles_per_row)
                static int split_rows = 5;      // vertical tile count (tiles_per_col)

                if (selectedIndex >= (int)scene.loaded_atlases.size()) {
                    selectedIndex = -1;
                    phystab_selectedTileIndex = -1;
                    path = nullptr;
                    tilesetToPreview = false;
                }


                // *** Draw list of all Tile Atlases ***

                for (int i = 0; i < scene.loaded_atlases.size(); i++) {

                    if (ImGui::Selectable(scene.loaded_atlases[i].name.c_str(), selectedIndex == i)) {
                        if (selectedIndex == i) {
                            selectedIndex = -1;
                        } else {
                            selectedIndex = i;
                            
                            if (selectedIndex != -1) {
                                tilesize    = scene.loaded_atlases[i].tile_size;
                                split_columns = (int)scene.loaded_atlases[i].tiles_per_row;
                                split_rows    = (int)scene.loaded_atlases[i].tiles_per_col;
                            }
                        }
                        phystab_selectedTileIndex = -1;
                        tilesetToPreview = true;
                        path = (scene.loaded_atlases[i].image_sheet_source);
                    }
                }

                size_t current_tilesize = 0;
                size_t current_tpc = 0;
                size_t current_tpr = 0;

                if (selectedIndex >= 0 && selectedIndex < scene.loaded_atlases.size()) {
                    auto & atlas = scene.loaded_atlases[selectedIndex];
                    current_tilesize = atlas.tile_size;
                    current_tpc = atlas.tiles_per_col;
                    current_tpr = atlas.tiles_per_row;
                }

                // *****************************************

                
                

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
                
                static int selectedTilesetToLoadIndex = -1;
                static Texture2D previewTilesetTexture = {0};
                static bool previewTilesetLoaded = false;
                static std::string selectedTilesetPathToLoad;
                static int new_tilesize = 16;
                static int new_split_columns = 1;
                static int new_split_rows = 1;
                static char new_tileset_name[128] = "";
                static bool loadTilesetNameError = false;
                
                
                if (ImGui::Button("+", ImVec2(30*fullscreenScale.x,(25.0f*fullscreenScale.y)))) {

                    selectedTilesetToLoadIndex = -1;
                    selectedTilesetPathToLoad.clear();
                    new_tilesize = 16;
                    new_split_columns = 1;
                    new_split_rows = 1;
                    new_tileset_name[0] = '\0';
                    loadTilesetNameError = false;
                    if (previewTilesetLoaded) {
                        UnloadTexture(previewTilesetTexture);
                        previewTilesetTexture = {0};
                        previewTilesetLoaded = false;
                    }
                    ImGui::OpenPopup("Load New Tileset...");

                }

                
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();




                ImVec2 loadTilesetModalSize = {
                    std::max(620.0f, GetScreenWidth() * 0.72f),
                    std::max(420.0f, GetScreenHeight() * 0.76f)
                };
                ImGui::SetNextWindowSize(loadTilesetModalSize, ImGuiCond_Appearing);
                if (ImGui::BeginPopupModal("Load New Tileset...", NULL, ImGuiWindowFlags_NoCollapse)) {

                    
                    ImGui::Separator();

                    std::vector<std::string> tileset_paths = assets.GetTilesetPaths("Gamefiles/Assets/Sprites/Tilesets/");

                    ImGui::Text("Choose source image:");
                    ImGui::BeginChild("TilesetSourceList", ImVec2(0, loadTilesetModalSize.y * 0.28f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

                    for (int i = 0; i < (int)tileset_paths.size(); i++) {
                        if (ImGui::Selectable(tileset_paths[i].c_str(), selectedTilesetToLoadIndex == i, ImGuiSelectableFlags_DontClosePopups)) {
                            if (selectedTilesetToLoadIndex == i) {
                                selectedTilesetToLoadIndex = -1;
                            } else {
                                selectedTilesetToLoadIndex = i;
                                
                                if (selectedTilesetToLoadIndex != -1) {
                                    
                                    selectedTilesetPathToLoad = tileset_paths[i];
                                    if (previewTilesetLoaded) {
                                        UnloadTexture(previewTilesetTexture);
                                        previewTilesetTexture = {0};
                                        previewTilesetLoaded = false;
                                    }
                                    previewTilesetTexture = LoadTexture(selectedTilesetPathToLoad.c_str());
                                    previewTilesetLoaded = IsTextureValid(previewTilesetTexture);
                                    if (previewTilesetLoaded) {
                                        new_split_columns = std::max(1, previewTilesetTexture.width / std::max(1, new_tilesize));
                                        new_split_rows = std::max(1, previewTilesetTexture.height / std::max(1, new_tilesize));
                                    } else {
                                        selectedTilesetPathToLoad.clear();
                                        selectedTilesetToLoadIndex = -1;
                                    
                                    } 

                                }
                            }
                            


                        }
                    }

                    ImGui::EndChild();

                    ImGui::Spacing();
                    ImGui::InputText("Tileset Name", new_tileset_name, sizeof(new_tileset_name));

                    std::string newAtlasNameTrimmed = std::string(new_tileset_name);
                    newAtlasNameTrimmed.erase(
                        newAtlasNameTrimmed.begin(),
                        std::find_if(newAtlasNameTrimmed.begin(), newAtlasNameTrimmed.end(), [] (unsigned char ch) {
                            return !std::isspace(ch);
                        })
                    );
                    newAtlasNameTrimmed.erase(
                        std::find_if(newAtlasNameTrimmed.rbegin(), newAtlasNameTrimmed.rend(), [] (unsigned char ch) {
                            return !std::isspace(ch);
                        }).base(),
                        newAtlasNameTrimmed.end()
                    );

                    bool uniqueName = !newAtlasNameTrimmed.empty();
                    for (const TileAtlas & atlas : scene.loaded_atlases) {
                        if (atlas.name == newAtlasNameTrimmed) {
                            uniqueName = false;
                            break;
                        }
                    }

                    const bool splitValid = previewTilesetLoaded
                        && TileAtlas().test_split_validity(previewTilesetTexture, new_tilesize, new_split_columns, new_split_rows);
                    const bool canConfirm = splitValid && uniqueName && !selectedTilesetPathToLoad.empty();

                    if (previewTilesetLoaded) {
                        ImGui::SliderInt("Tile Size", &new_tilesize, 1, 256);
                        ImGui::SliderInt("Tiles / Col", &new_split_columns, 1, 512);
                        ImGui::SliderInt("Tiles / Row", &new_split_rows, 1, 512);

                        
                        ImGui::Text("Preview: %dx%d", previewTilesetTexture.width, previewTilesetTexture.height);
                        ImGui::TextColored(splitValid ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1), splitValid ? "Valid split" : "Invalid split");

                        ImVec2 previewChildSize = ImVec2(0, loadTilesetModalSize.y * 0.42f);
                        ImGui::BeginChild("TilesetSplitPreviewChild", previewChildSize, true, ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);
                        ImDrawList * drawList = ImGui::GetWindowDrawList();
                        ImVec2 canvasStart = ImGui::GetCursorScreenPos();
                        const float padding = 8.0f;
                        const float maxPreviewWidth = std::max(320.0f, loadTilesetModalSize.x - 56.0f);
                        const float maxPreviewHeight = std::max(180.0f, loadTilesetModalSize.y * 0.35f);
                        const float imgScale = std::min(maxPreviewWidth / (float)previewTilesetTexture.width, maxPreviewHeight / (float)previewTilesetTexture.height);
                        
                        const float scaledTileSize = (float)new_tilesize * imgScale;
                        const int imageCols = std::max(1, (int)std::ceil((float)previewTilesetTexture.width / (float)new_tilesize));
                        const int imageRows = std::max(1, (int)std::ceil((float)previewTilesetTexture.height / (float)new_tilesize));
                        const int gridCols = std::max(imageCols, new_split_columns);
                        const int gridRows = std::max(imageRows, new_split_rows);
                        const ImVec2 previewSize = {
                            std::max(previewTilesetTexture.width * imgScale, gridCols * scaledTileSize),
                            std::max(previewTilesetTexture.height * imgScale, gridRows * scaledTileSize)
                        };

                        drawList->AddRectFilled(
                            {canvasStart.x, canvasStart.y},
                            {canvasStart.x + previewSize.x + padding, canvasStart.y + previewSize.y + padding},
                            IM_COL32(32, 32, 32, 255)
                        );
                        ImGui::Image((ImTextureID)previewTilesetTexture.id, ImVec2(previewTilesetTexture.width * imgScale, previewTilesetTexture.height * imgScale));

                        
                        ImU32 gridColorValid = IM_COL32(0, 255, 0, 220);
                        ImU32 gridColorInvalid = IM_COL32(255, 0, 0, 220);
                        
                        for (int r = 0; r < gridRows; r++) {
                            for (int c = 0; c < gridCols; c++) {
                                ImVec2 dmin = {
                                    canvasStart.x + (float)c * scaledTileSize,
                                    canvasStart.y + (float)r * scaledTileSize
                                };
                                ImVec2 dmax = {
                                    dmin.x + scaledTileSize,
                                    dmin.y + scaledTileSize
                                };
                                bool cellIncludedInSelection = c < new_split_columns && r < new_split_rows;
                                bool cellFitsImageBounds = ((c + 1) * new_tilesize <= previewTilesetTexture.width)
                                    && ((r + 1) * new_tilesize <= previewTilesetTexture.height);
                                ImU32 gridColor = (cellIncludedInSelection && cellFitsImageBounds) ? gridColorValid : gridColorInvalid;
                                drawList->AddRect(dmin, dmax, gridColor, 0.0f, 0, 1.5f);
                            }
                        }

                        ImGui::Dummy(ImVec2(previewSize.x + padding, previewSize.y + padding));
                        ImGui::EndChild();

                    }
                    

                    ImGui::Spacing();

                    // CONFIRM BUTTON
                    ImGui::BeginDisabled(!canConfirm);
                    if (ImGui::Button("Confirm", ImVec2(120, 0))) {

                        
                        loadTilesetNameError = !uniqueName;

                        if (canConfirm) {
                            Texture2D & importedTexture = assets.LoadTilesetTexture(selectedTilesetPathToLoad);
                            scene.load_new_tileset(newAtlasNameTrimmed, importedTexture, new_tilesize, new_split_columns, new_split_rows);
                            selectedIndex = (int)scene.loaded_atlases.size() - 1;
                            tilesize = new_tilesize;
                            split_columns = new_split_columns;
                            split_rows = new_split_rows;
                            phystab_selectedTileIndex = -1;
                            tilesetToPreview = true;
                            path = scene.loaded_atlases[selectedIndex].image_sheet_source;

                            if (previewTilesetLoaded) {
                                UnloadTexture(previewTilesetTexture);
                                previewTilesetTexture = {0};
                                previewTilesetLoaded = false;
                            }
                            ImGui::CloseCurrentPopup();
                        }
                    }

                    ImGui::EndDisabled();

                    ImGui::SameLine();

                    // CANCEL BUTTON
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                        if (previewTilesetLoaded) {
                            UnloadTexture(previewTilesetTexture);
                            previewTilesetTexture = {0};
                            previewTilesetLoaded = false;
                        }
                        ImGui::CloseCurrentPopup();
                    }

                    if (loadTilesetNameError || (!uniqueName && new_tileset_name[0] != '\0')) {
                        ImGui::TextColored(ImVec4(1, 0.25f, 0.25f, 1), "Tileset name must be non-empty and unique.");
                    }

                    ImGui::EndPopup();

                } else if (previewTilesetLoaded) {
                    UnloadTexture(previewTilesetTexture);
                    previewTilesetTexture = {0};
                    previewTilesetLoaded = false;

                }



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
                if (ImGui::Button("-", ImVec2(30*fullscreenScale.x,(25.0f*fullscreenScale.y)))) {
                    if (selectedIndex >= 0 && selectedIndex < (int)scene.loaded_atlases.size()) {
                        ImGui::OpenPopup("Delete Tileset Confirm");
                    }
                }
                
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();


                if (ImGui::BeginPopupModal("Delete Tileset Confirm", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    if (selectedIndex >= 0 && selectedIndex < (int)scene.loaded_atlases.size()) {
                        ImGui::Text("Delete tileset '%s'?", scene.loaded_atlases[selectedIndex].name.c_str());
                        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "This will clear placed tiles that use it.");
                    } else {
                        ImGui::Text("No tileset selected.");
                    }

                    ImGui::Separator();

                    if (ImGui::Button("Confirm", ImVec2(120, 0))) {
                        if (selectedIndex >= 0 && selectedIndex < (int)scene.loaded_atlases.size()) {
                            Texture2D * deletedTexture = scene.loaded_atlases[selectedIndex].image_sheet_source;

                            for (TileGrid & grid : scene.tile_layers) {
                                for (int y = gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE; y <= gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE; y++) {
                                    for (int x = gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE; x <= gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE; x++) {
                                        Tile & tile = grid.get_tile(x, y);
                                        if (tile.atlas_idx == selectedIndex) {
                                            tile = {-1, -1};
                                        } else if (tile.atlas_idx > selectedIndex) {
                                            tile.atlas_idx--;
                                        }
                                    }
                                }
                            }

                            scene.loaded_atlases.erase(scene.loaded_atlases.begin() + selectedIndex);

                            if (scene.EDITOR_ONLY_SELECTED_ATLAS == selectedIndex) {
                                scene.EDITOR_ONLY_SELECTED_ATLAS = -1;
                                scene.EDITOR_ONLY_SELECTED_PALLET_TILE = -1;
                            } else if (scene.EDITOR_ONLY_SELECTED_ATLAS > selectedIndex) {
                                scene.EDITOR_ONLY_SELECTED_ATLAS--;
                            }

                            bool atlasStillUsesTexture = false;
                            for (const TileAtlas & atlas : scene.loaded_atlases) {
                                if (atlas.image_sheet_source == deletedTexture) {
                                    atlasStillUsesTexture = true;
                                    break;
                                }
                            }
                            if (!atlasStillUsesTexture) {
                                assets.UnloadTilesetTexture(deletedTexture);
                            }

                            selectedIndex = -1;
                            phystab_selectedTileIndex = -1;
                            path = nullptr;
                            tilesetToPreview = false;
                        }
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }



                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f,0.9f,0,1)); // 
                UIPos(240, 595.0f + 56.0f, 240, (600 - 35) + (60.0f));
                if (fullscreen) {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 1*fullscreenScale.y));
                } else {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 0));
                }
                if (ImGui::Button("<", ImVec2(30*fullscreenScale.x,(25.0f*fullscreenScale.y)))) {
                    showTileAtlasEditor = !showTileAtlasEditor;
                }
                
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();


                static bool splitStagedAndValid = false;
                
                static bool splitConfirmationWindow = false;


                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f,0.05f,0.8f,0.5f)); // 
                UIPos(240, 595.0f + 84.0f, 240, (600 - 35) + (90.0f));
                if (fullscreen) {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 1*fullscreenScale.y));
                } else {
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 0));
                }

                if (splitStagedAndValid) {

                    if(ImGui::Button("%", ImVec2(30*fullscreenScale.x,(25.0f*fullscreenScale.y)))) {

                        ImGui::OpenPopup("Confirm Split");

                    }
                }

                if (ImGui::BeginPopupModal("Confirm Split", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

                    ImGui::Text("Are you sure you want to re-split this tileset?");
                    ImGui::Separator();

                    ImGui::Text("Tile Size: %d", tilesize);
                    ImGui::Text("Columns: %d", split_columns);
                    ImGui::Text("Rows: %d", split_rows);

                    ImGui::Spacing();

                    // CONFIRM BUTTON
                    if (ImGui::Button("Confirm", ImVec2(120, 0))) {

                        splitStagedAndValid = false;

                        // Delete tilegrid tiles that has these tile atlases

                        for (TileGrid & grid : scene.tile_layers) {  // ***HUGELY DANGEROUS****

                            for (int y = gwconst::WORLD_TILEGRID_Y_BOUND_MIN_TILE; y <= gwconst::WORLD_TILEGRID_Y_BOUND_MAX_TILE ; y++) {

                                for (int x = gwconst::WORLD_TILEGRID_X_BOUND_MIN_TILE; x <= gwconst::WORLD_TILEGRID_X_BOUND_MAX_TILE ; x++) {

                                    Tile& tile = grid.get_tile(x, y);

                                    if (tile.atlas_idx == selectedIndex) {
                                        tile = {-1, -1};
                                    }
                                    

                                }

                            }


                        }

                        scene.loaded_atlases[selectedIndex].split_tileset(*scene.loaded_atlases[selectedIndex].image_sheet_source, tilesize, split_columns, split_rows);

                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    // CANCEL BUTTON
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
                
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

                            UIPos(280, 695, 285, 695);
                            ImGui::Text("%.0fpx by %.0fpx", (float)texture.width, (float)texture.height);
                        }

                        UIPos(290, 595, 290, 565);
                        ImGui::Image((ImTextureID)texture.id, size);

                    }


                    UIPos(500, 558, 500, 542);
                    ImGui::Text("Settings:");

                    UIPos(490, 563, 490, 547);
                    ImGui::Text("___________");


                    ImGui::PushItemWidth(85 * fullscreenScale.x);

                    newTilesetSplitMatch = true;
                    
                    if (fullscreen) {
                        UIPos(480, 600, 480, 600 - 30);

                        if (tilesize != (int)current_tilesize) {
                            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(180, 180, 0, 255));       // yellow grab
                            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(255, 255, 0, 255));       
                            newTilesetSplitMatch = false;
                        } else {
                            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(0, 180, 0, 255));       
                            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(0, 255, 0, 255));       
                        }

                        ImGui::SliderInt("Tiles Size", &tilesize, 1, 64);

                        UIPos(480, 640, 480, 640 - 30);

                        if (split_columns != (int)current_tpr) {
                            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(180, 180, 0, 255));       // yellow grab
                            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(255, 255, 0, 255));     
                            newTilesetSplitMatch = false;  
                        } else {
                            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(0, 180, 0, 255));       
                            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(0, 255, 0, 255));       
                        }

                        ImGui::SliderInt("Tiles / C.", &split_columns, 1, 32);

                        UIPos(480, 680, 480, 680 - 30);

                        if (split_rows != (int)current_tpc) {
                            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(180, 180, 0, 255));       // yellow grab
                            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(255, 255, 0, 255));     
                            newTilesetSplitMatch = false;  
                        } else {
                            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(0, 180, 0, 255));       
                            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(0, 255, 0, 255));       
                        }

                        ImGui::SliderInt("Tiles / R.", &split_rows, 1, 32);
                    } else {
                        UIPos(480, 590, 480,590 - 30);

                        if (tilesize != (int)current_tilesize) {
                            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(180, 180, 0, 255));       // yellow grab
                            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(255, 255, 0, 255));       
                            newTilesetSplitMatch = false;
                        } else {
                            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(0, 180, 0, 255));       
                            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(0, 255, 0, 255));       
                        }
                        
                        ImGui::SliderInt("Size", &tilesize, 1, 64);

                        UIPos(480, 630, 480, 630 - 30);

                        if (split_columns != (int)current_tpr) {
                            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(180, 180, 0, 255));       // yellow grab
                            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(255, 255, 0, 255));       
                            newTilesetSplitMatch = false;
                        } else {
                            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(0, 180, 0, 255));       
                            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(0, 255, 0, 255));       
                        }
                        
                        ImGui::SliderInt("TPC", &split_columns, 1, 32);

                        UIPos(480, 670, 480, 670 - 30);

                        if (split_rows != (int)current_tpc) {
                            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(180, 180, 0, 255));       // yellow grab
                            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(255, 255, 0, 255));       
                            newTilesetSplitMatch = false;
                        } else {
                            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(0, 180, 0, 255));       
                            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(0, 255, 0, 255));       
                        }
                        
                        ImGui::SliderInt("TPR", &split_rows, 1, 32);
                    }

                    ImGui::PopItemWidth();
                    ImGui::PopStyleColor(6);



                    // ============ Physics Editor =============

                    

                    UIPos(690, 558, 690, 542);
                    ImGui::Text("Physics & Collisions:");

                    UIPos(660, 563, 660, 547);
                    ImGui::Text("___________________________");

                    UIPos(700, 600, 660, 570);

                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f,0.2f,0.2f,0.7f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0,1)); // black
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f); // thickness

                    if (fullscreen) {
                        ImGui::BeginChild("PhysicsTileGrid", ImVec2(260 * fullscreenScale.x, 135 * fullscreenScale.y), true);
                    } else {
                        ImGui::BeginChild("PhysicsTileGrid", ImVec2(260 * fullscreenScale.x, 110 * fullscreenScale.y), true);
                    }

                    if (selectedIndex >= 0) {

                        Texture2D & tileset_texture = *scene.loaded_atlases[selectedIndex].image_sheet_source;
                        int total_cols = (int)(scene.loaded_atlases[selectedIndex].tiles_per_row);
                        int total_rows = (int)(scene.loaded_atlases[selectedIndex].tiles_per_col);
                        
                        int tile_drawsize = 32 * fullscreenScale.x;

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
                            bool filled;
                        };

                        struct ImgDraw {
                            ImVec2 min;
                            ImVec2 max;
                            ImVec2 uv_0;
                            ImVec2 uv_1;
                            Texture2D& img;
                        };


                        std::vector<ImgDraw> imgDrawQueue;
                        std::vector<RectDraw> drawQueue;
                        std::vector<RectDraw> lowerQueue;

                        ImVec2 firstTileMin;
                        bool gotFirstTile = false;

                        for (int row = 0; row < total_rows; row++) {
                            for (int col = 0; col < total_cols; col++) {

                                Rectangle tile_slice = scene.loaded_atlases[selectedIndex].getRectCR(col, row);

                                ImVec2 uv0 = ImVec2(
                                    tile_slice.x / (float)tileset_texture.width,
                                    tile_slice.y / (float)tileset_texture.height
                                );

                                ImVec2 uv1 = ImVec2(
                                    (tile_slice.x + tile_slice.width) / (float)tileset_texture.width,
                                    (tile_slice.y + tile_slice.height) / (float)tileset_texture.height
                                );
                                
                                ImGui::PushID(row * total_cols + col);

                                CollisionType coll = scene.loaded_atlases[selectedIndex].tile_data[row * total_cols + col].collision_data;
                                Rectangle ico_slice = {-9999,0,0,0};

                                if (coll == CollisionType::COLL_EMPTY) {
                                    ico_slice = editorAssets.util_tileset_geticon(CollisionIcons::UTIL_EMPTY_COL_ICO );
                                } else if (coll == CollisionType::COLL_FULL_SOLID) {
                                    ico_slice = editorAssets.util_tileset_geticon(CollisionIcons::UTIL_FULL_COL_ICO );
                                } else if (coll == CollisionType::COLL_PSLOPE1_SOLID) {
                                    ico_slice = editorAssets.util_tileset_geticon(CollisionIcons::UTIL_PSLOPE1_COL_ICO );
                                } else if (coll == CollisionType::COLL_NSLOPE1_SOLID) {
                                    ico_slice = editorAssets.util_tileset_geticon(CollisionIcons::UTIL_NSLOPE1_COL_ICO );
                                }

                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0,0,0,0.3f));
                                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0,0,0,0.2f));

                                if (ImGui::ImageButton("tile", (ImTextureID)tileset_texture.id, {(float)tile_drawsize, (float)tile_drawsize}, uv0, uv1)) {
                                    phystab_selectedTileIndex = row * total_cols + col;

                                    anim_frames = scene.loaded_atlases[selectedIndex].tile_data[phystab_selectedTileIndex].anim_frame_count;
                                    frame_time = scene.loaded_atlases[selectedIndex].tile_data[phystab_selectedTileIndex].anim_frame_speed;

                                    animParamsMatch = true;
                                }
                                
                                if (scene.loaded_atlases[selectedIndex].tile_data[row * total_cols + col].anim_parent_index != -1) {

                                    drawQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 255, 255, 255, 40, 2.0f, true});   
                                
                                }

                                if (ico_slice.x != -9999) {
                                    ImVec2 uv2 = ImVec2(
                                        ico_slice.x / (float)editorAssets.util_tileset.width,
                                        ico_slice.y / (float)editorAssets.util_tileset.height
                                    );

                                    ImVec2 uv3 = ImVec2(
                                        (ico_slice.x + ico_slice.width) / (float)editorAssets.util_tileset.width,
                                        (ico_slice.y + ico_slice.height) / (float)editorAssets.util_tileset.height
                                    );

                                    imgDrawQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), uv2, uv3, editorAssets.util_tileset});

                                }

                                if (!gotFirstTile) {
                                    firstTileMin = ImGui::GetItemRectMin(); // THIS is what you want
                                    gotFirstTile = true;
                                }



                                


                            
                                lowerQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 0, 0, 0, 255, 2.0f, false});

                                if (animParamsMatch != true) {
                                    if (phystab_selectedTileIndex + anim_frames > row * total_cols + col && row * total_cols + col > phystab_selectedTileIndex) {
                                        drawQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 255, 0, 0, 40, 2.0f, true});   
                                    }
                                }

                                if ((row * total_cols + col) >= 0 && (row * total_cols + col) < (int)scene.loaded_atlases[selectedIndex].tile_data.size()) {
                                    if (scene.loaded_atlases[selectedIndex].tile_data[row * total_cols + col].is_anim_tile) {
                                        drawQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 150, 30, 0, 255, 2.0f, false});
                                    }
                                }

                                if (ImGui::IsItemHovered()) {
                                    drawQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 0, 0, 225, 255, 2.0f, false});   
                                }

                                if (phystab_selectedTileIndex == row * total_cols + col) {
                                    drawQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 0, 0, 190, 255, 3.0f, false});
                                }
                                
                                ImGui::PopID();

                                if (col < total_cols-1) {
                                    ImGui::SameLine();
                                }





                                
                                ImGui::PopStyleColor(3);
                            }

                    
                        }


                        // ******** EDIT HERE TO ADD TILESET SPLIT PREVIEW *************

                        if (!newTilesetSplitMatch && selectedIndex != -1) {

                            float actual_tile_size = (float)scene.loaded_atlases[selectedIndex].tile_size;

                            // how much 1 pixel in the texture = in ImGui space
                            float pixels_to_screen = tile_drawsize / actual_tile_size;

                            // now scale your NEW tilesize properly
                            float scaledSize = tilesize * pixels_to_screen;

                            ImVec2 dmin = firstTileMin;
                            ImVec2 dmax = ImVec2(dmin.x + scaledSize, dmin.y + scaledSize);

                            int cols_to_draw = split_columns;
                            int rows_to_draw = split_rows;

                            // =======================================================

                            ImDrawList* rdraw = ImGui::GetWindowDrawList();

                            for (int r = 0; r < rows_to_draw; r++) {
                                for (int c = 0; c < cols_to_draw; c++) {

                                    ImVec2 dmin = ImVec2(
                                        firstTileMin.x + c * scaledSize,
                                        firstTileMin.y + r * scaledSize
                                    );

                                    ImVec2 dmax = ImVec2(
                                        dmin.x + scaledSize,
                                        dmin.y + scaledSize
                                    );

                                    if (scene.loaded_atlases[selectedIndex].test_split_validity(*scene.loaded_atlases[selectedIndex].image_sheet_source, tilesize, split_columns, split_rows)) {
                                        
                                        // Green, meaning it is valid split
                                        
                                        rdraw->AddRect(dmin, dmax, IM_COL32(0, 255, 0, 255), 0.0f, 0, 2.0f);
                                        splitStagedAndValid = true;

                                    } else {

                                        rdraw->AddRect(dmin, dmax, IM_COL32(255, 0, 0, 255), 0.0f, 0, 2.0f);
                                        splitStagedAndValid = false;

                                    }
                                }
                            }

                        }

                        // ***************************************************************
                        

                        for (RectDraw draw : lowerQueue) {
                            ImDrawList* rdraw = ImGui::GetWindowDrawList();
                            rdraw->AddRect(draw.min, draw.max, IM_COL32(draw.r, draw.g, draw.b, draw.a), 0.0f, 0, draw.thickness);
                        }

                        for (ImgDraw imgdraw : imgDrawQueue) {
                            ImDrawList* dl = ImGui::GetWindowDrawList();

                            dl->AddImage( (ImTextureID)imgdraw.img.id, imgdraw.min, imgdraw.max, imgdraw.uv_0, imgdraw.uv_1 );
                        }

                        for (RectDraw draw : drawQueue) {
                            if (draw.filled) {
                                ImDrawList* rdraw = ImGui::GetWindowDrawList();
                                rdraw->AddRectFilled(draw.min, draw.max, IM_COL32(draw.r, draw.g, draw.b, draw.a));
                            } else {
                                ImDrawList* rdraw = ImGui::GetWindowDrawList();
                                rdraw->AddRect(draw.min, draw.max, IM_COL32(draw.r, draw.g, draw.b, draw.a), 0.0f, 0, draw.thickness);
                            }
                        }

                        

                        drawQueue.clear();
                        ImGui::PopStyleVar(3);
                        ImGui::PopStyleColor(2);
                        ImGui::EndChild();

                    }





                    UIPos(970, 600, 960, 570);

                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f,0.2f,0.2f,0.7f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0,1)); // black
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f); // thickness

                    if (fullscreen) {
                        ImGui::BeginChild("CollisionSelector", ImVec2(240 * fullscreenScale.x, 135 * fullscreenScale.y), true);
                    } else {
                        ImGui::BeginChild("CollisionSelector", ImVec2(240 * fullscreenScale.x, 110 * fullscreenScale.y), true);
                    }

                    // ** INSERT CODE HERE ** //


                    // ======================= COLLISION SELECTOR =======================

                    // safety: only operate if a tile is selected
                    bool validTile =
                        (selectedIndex >= 0) &&
                        (phystab_selectedTileIndex >= 0) &&
                        (phystab_selectedTileIndex < (int)scene.loaded_atlases[selectedIndex].tile_data.size());

                    CollisionType* currentCollision = nullptr;

                    if (selectedIndex >= 0 &&
                        phystab_selectedTileIndex >= 0 &&
                        phystab_selectedTileIndex < (int)scene.loaded_atlases[selectedIndex].tile_data.size()) {

                        currentCollision =
                            &scene.loaded_atlases[selectedIndex].tile_data[phystab_selectedTileIndex].collision_data;
                    }

                    

                    // -------- LEFT SIDE: CURRENT SELECTION DISPLAY --------
                    ImGui::BeginGroup();

                    ImGui::Text("Current:");

                    ImVec2 previewSize = ImVec2(40 * fullscreenScale.x, 40 * fullscreenScale.y);

                    Rectangle currentIcon = {-9999,0,0,0};

                    if (validTile) {
                        if (currentCollision && (*currentCollision == CollisionType::COLL_EMPTY))
                            currentIcon = editorAssets.util_tileset_geticon(CollisionIcons::UTIL_EMPTY_COL_ICO);
                        else if (currentCollision && (*currentCollision == CollisionType::COLL_FULL_SOLID))
                            currentIcon = editorAssets.util_tileset_geticon(CollisionIcons::UTIL_FULL_COL_ICO);
                        else if (currentCollision && (*currentCollision == CollisionType::COLL_PSLOPE1_SOLID))
                            currentIcon = editorAssets.util_tileset_geticon(CollisionIcons::UTIL_PSLOPE1_COL_ICO);
                        else if (currentCollision && (*currentCollision == CollisionType::COLL_NSLOPE1_SOLID))
                            currentIcon = editorAssets.util_tileset_geticon(CollisionIcons::UTIL_NSLOPE1_COL_ICO);
                    }

                    if (currentIcon.x != -9999) {
                        ImVec2 min = ImGui::GetCursorScreenPos();
                        ImVec2 max = {min.x + previewSize.x, min.y + previewSize.y};

                        // white background
                        ImGui::GetWindowDrawList()->AddRectFilled(min, max, IM_COL32(60,60,60,255));

                        ImVec2 uv0 = {
                            currentIcon.x / (float)editorAssets.util_tileset.width,
                            currentIcon.y / (float)editorAssets.util_tileset.height
                        };

                        ImVec2 uv1 = {
                            (currentIcon.x + currentIcon.width) / (float)editorAssets.util_tileset.width,
                            (currentIcon.y + currentIcon.height) / (float)editorAssets.util_tileset.height
                        };

                        ImGui::Image((ImTextureID)editorAssets.util_tileset.id, previewSize, uv0, uv1);
                    }

                    ImGui::EndGroup();

                    // spacing between left + right
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);

                    // -------- RIGHT SIDE: SELECTOR LIST --------
                    ImGui::BeginGroup();

                    struct CollisionOption {
                        CollisionType type;
                        CollisionIcons icon;
                        const char* label;
                    };

                    CollisionOption options[4] = {
                        {CollisionType::COLL_EMPTY,        CollisionIcons::UTIL_EMPTY_COL_ICO,   "Empty Collision"},
                        {CollisionType::COLL_FULL_SOLID,   CollisionIcons::UTIL_FULL_COL_ICO,    "Full Solid"},
                        {CollisionType::COLL_PSLOPE1_SOLID,CollisionIcons::UTIL_PSLOPE1_COL_ICO, "Positive Slope"},
                        {CollisionType::COLL_NSLOPE1_SOLID,CollisionIcons::UTIL_NSLOPE1_COL_ICO, "Negative Slope"}
                    };

                    ImVec2 buttonSize = ImVec2(28 * fullscreenScale.x, 28 * fullscreenScale.y);

                    for (int i = 0; i < 4; i++) {
                        

                        ImGui::PushID(i);

                        bool isSelected = currentCollision && (*currentCollision == options[i].type);

                        // highlight selected
                        if (isSelected) {
                            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(80, 120, 255, 255));
                        } else {
                            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(50, 50, 50, 255));
                        }

                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(90, 90, 90, 255));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(120, 120, 120, 255));

                        Rectangle ico = editorAssets.util_tileset_geticon(options[i].icon);

                        ImVec2 min = ImGui::GetCursorScreenPos();
                        ImVec2 max = {min.x + buttonSize.x, min.y + buttonSize.y};

                        // white background (IMPORTANT)
                        ImGui::GetWindowDrawList()->AddRectFilled(min, max, IM_COL32(255,255,255,255));

                        ImVec2 uv0 = {
                            ico.x / (float)editorAssets.util_tileset.width,
                            ico.y / (float)editorAssets.util_tileset.height
                        };

                        ImVec2 uv1 = {
                            (ico.x + ico.width) / (float)editorAssets.util_tileset.width,
                            (ico.y + ico.height) / (float)editorAssets.util_tileset.height
                        };

                        if (ImGui::ImageButton("colbtn", (ImTextureID)editorAssets.util_tileset.id, buttonSize, uv0, uv1)) {
                            if (currentCollision) {
                                *currentCollision = options[i].type;
                            }
                        }

                        ImGui::SameLine();
                        ImGui::Text("%s", options[i].label);

                        ImGui::PopStyleColor(3);
                        ImGui::PopID();
                    }

                    ImGui::EndGroup();


                    // *** Do not put code past here

                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(2);

                    ImGui::EndChild();

                    
                    // ========================== END OF PHYSICS WINDOW =================================






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
                            selectedTileIndex = -1;
                        } else {
                            selectedAtlas = i;
                            selectedTileIndex = -1;
                        }
                    }
                }

                scene.EDITOR_ONLY_SELECTED_ATLAS = selectedAtlas;
                

                

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

                UIPos(1050, 563, 1050, 547);
                ImGui::Text("________");

                UIPos(1050, 558, 1050, 542);
                ImGui::Text("Tile Settings:         ");

                

                // Below the text...?

                

                bool selectedAtlasValid = (selectedAtlas >= 0 && selectedAtlas < (int)scene.loaded_atlases.size());

                if (selectedTileIndex != -1 && selectedAtlasValid && selectedTileIndex < (int)scene.loaded_atlases[selectedAtlas].tile_data.size()) {

                    ImGui::PushItemWidth(80 * fullscreenScale.x);

                    UIPos(1050, 595, 1050, 565);
                    

                    if (anim_frames != scene.loaded_atlases[selectedAtlas].tile_data[selectedTileIndex].anim_frame_count) {
                        ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(0, 255, 0, 255));       // yellow grab
                        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(0, 255, 0, 255));       
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(255, 255, 0, 255));       // yellow grab
                        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(255, 255, 0, 255));    
                    }

                    ImGui::SliderInt("Anim Frms", &anim_frames, 1, 16);
                    ImGui::PopStyleColor(2);

                    if (anim_frames != 1) {
                        
                        

                        

                        UIPos(1050, 635, 1050, 605);
                        

                        if (frame_time != scene.loaded_atlases[selectedAtlas].tile_data[selectedTileIndex].anim_frame_speed) {
                            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(0, 255, 0, 255));       // yellow grab
                            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(0, 255, 0, 255));       
                        } else {
                            ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(255, 255, 0, 255));       // yellow grab
                            ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(255, 255, 0, 255));    
                        }

                        ImGui::SliderFloat("Frm Time", &frame_time, 0.00f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);

                        ImGui::PopStyleColor(2);
                    }

                    

                    ImGui::PopItemWidth();

                    if (frame_time == scene.loaded_atlases[selectedAtlas].tile_data[selectedTileIndex].anim_frame_speed && anim_frames == scene.loaded_atlases[selectedAtlas].tile_data[selectedTileIndex].anim_frame_count) {
                        animParamsMatch = true;
                        
                    } else {
                        animParamsMatch = false;
                        bool canApplyAnimation = scene.loaded_atlases[selectedAtlas].are_anim_params_valid(selectedTileIndex, anim_frames, frame_time);

                        UIPos(1050, 675, 1050, 655);
                        if (canApplyAnimation && ImGui::Button("Apply Changes", ImVec2(200*fullscreenScale.x,(35.0f*fullscreenScale.y)))) {
                            if (scene.apply_tile_animation(selectedAtlas, selectedTileIndex, anim_frames, frame_time)) {
                                anim_frames = scene.loaded_atlases[selectedAtlas].tile_data[selectedTileIndex].anim_frame_count;
                                frame_time = scene.loaded_atlases[selectedAtlas].tile_data[selectedTileIndex].anim_frame_speed;
                                animParamsMatch = true;
                            }
                        } else if (!canApplyAnimation) {
                            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Invalid animation parameters.");
                        }
                    }

                    

                    

                
                }


                


                // ----------------------------------->
                // Draw Tile window
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

                if (selectedAtlasValid) {

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
                        bool filled;
                    };


                    std::vector<RectDraw> drawQueue;
                    std::vector<RectDraw> lowerQueue;

                    for (int row = 0; row < total_rows; row++) {
                        for (int col = 0; col < total_cols; col++) {

                            if (scene.loaded_atlases[selectedAtlas].tile_data[row * total_cols + col].anim_parent_index != -1) {
                                continue;
                            }

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
                            
                            if (ImGui::ImageButton("tile", (ImTextureID)tileset_texture.id, {(float)tile_drawsize, (float)tile_drawsize}, uv0, uv1)) {
                                selectedTileIndex = row * total_cols + col;

                                anim_frames = scene.loaded_atlases[selectedAtlas].tile_data[selectedTileIndex].anim_frame_count;
                                frame_time = scene.loaded_atlases[selectedAtlas].tile_data[selectedTileIndex].anim_frame_speed;

                                animParamsMatch = true;
                            }


                           
                            lowerQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 0, 0, 0, 255, 2.0f, false});

                            if (animParamsMatch != true) {
                                if (selectedTileIndex + anim_frames > row * total_cols + col && row * total_cols + col > selectedTileIndex) {
                                    drawQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 255, 0, 0, 40, 2.0f, true});   
                                }
                            }

                            if ((row * total_cols + col) >= 0 && (row * total_cols + col) < (int)scene.loaded_atlases[selectedAtlas].tile_data.size()) {
                                if (scene.loaded_atlases[selectedAtlas].tile_data[row * total_cols + col].is_anim_tile) {
                                    drawQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 150, 30, 0, 255, 2.0f, false});
                                }
                            }

                            if (ImGui::IsItemHovered()) {
                                drawQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 0, 0, 225, 255, 2.0f, false});   
                            }

                            if (selectedTileIndex == row * total_cols + col) {
                                drawQueue.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), 0, 0, 190, 255, 3.0f, false});
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
                        if (draw.filled) {
                            ImDrawList* rdraw = ImGui::GetWindowDrawList();
                            rdraw->AddRectFilled(draw.min, draw.max, IM_COL32(draw.r, draw.g, draw.b, draw.a));
                        } else {
                            ImDrawList* rdraw = ImGui::GetWindowDrawList();
                            rdraw->AddRect(draw.min, draw.max, IM_COL32(draw.r, draw.g, draw.b, draw.a), 0.0f, 0, draw.thickness);
                        }
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
                        childSize.x - 35.0*fullscreenScale.x - 10.0,
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

                scene.EDITOR_ONLY_SELECTED_PALLET_TILE = selectedTileIndex;

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


    scene.uiCapturesMouse = ImGui::GetIO().WantCaptureMouse;

    ImGui::End();

    

}



