// EditorUISystem.cpp

#include "EditorUISystem.h"

#include "../../Engine/Core/Rendering/Renderer.h"
#include "../../Engine/Core/Window/Window.h"
#include "../../Engine/Core/Overhead/Config.h"
#include "../../Engine/Core/Overhead/WindowStats.h"
#include "../../Gamefiles/World/Scene.h"
#include "../../Gamefiles/World/Tile.h"
#include "../../Gamefiles/World/Overhead/Gwconst.h"
#include "../../Gamefiles/Assets/EditorAssets.h"

#include "../../Tooling/Debug/DebugManager.h"


// ========================================================================================================
//            MAIN EDITOR SCRIPT
// <======================================>
//
// --------------------->
//    This shit is more or less a nightmare to debug so I'm beginning a documentation or at least catch all
// For important notation
//
// ======================>
//
//  ***** TODO LIST *****
//   --> For all main things to be done by @IMAWarriors, refer to search in TODO
//      ->  : // @IMAWarriors TODO
//
//
//
//
//

#include <vector>
#include <algorithm>

#include <cmath>
#include <cctype>
#include <cstdio>

#include <filesystem>
#include <functional>



void EditorUISystem::update (Registry & registry, float deltatime) {

    Vec2 fullscreenScale = {
                        (float)GetScreenWidth() / (float)gwconst::SCREEN_WIDTH_GAMEPIXELS,
                        (float)GetScreenHeight() / (float)gwconst::SCREEN_HEIGHT_GAMEPIXELS
    };




    static bool WORKSPACE_WINDOW_DRAW = true;


    const int INTENDED_FPS = (int)config::GAME_WORLD_FPS;



    bool fullscreen = GetScreenWidth() > config::INIT_WINDOW_DISPLAY_WIDTH;

    scene.EDITOR_ONLY_BACKGROUND_TAB_SELECTED = false;
    scene.EDITOR_ONLY_ACTIVE_BACKGROUND_EDITOR = false;


    static bool showTileAtlasEditor = false;
    static bool showLayerManager = true;

    static int selectedLayer = 0;
    static int selectedClamp = -1;

    static int selectedIndex = -1; // FOR line: if(ImGui::BeginTabItem("Tileset")) {...  // serves to give show selected tile atlas for tile atlas editor

    static bool animParamsMatch = true;

    static int phystab_selectedTileIndex = -1;

    static int selectedTileIndex = -1;

    static int selectedBackgroundLayer = -1;
    static int selectedBackgroundNode = -1;
    static bool backgroundPainterMode = false;
    static int backgroundSeatX = 0;
    static int backgroundSeatY = 0;
    static std::string backgroundStatusMessage;
    static bool backgroundStatusIsError = false;

    static bool newTilesetSplitMatch = true;

    static int anim_frames = 1;
    static float frame_time = 0.0f;
    static int palette_tiles_per_page = 256;
    static int palette_page_columns = 12;
    static int palette_page_index = 0;
    static int atlas_tiles_per_page = 512;
    static int atlas_page_columns = 16;
    static int atlas_page_index = 0;
    static int physics_tiles_per_page = 128;
    static int physics_page_columns = 8;
    static int physics_page_index = 0;

    static bool dcaeJustOpened = false;

    // Not wired to real joint system yet


    
    


    std::filesystem::create_directories(BACKGROUNDIMAGEDIR);

    ImGui::GetStyle().WindowMinSize = ImVec2(2.0,2.0);
    ImGui::GetStyle().WindowPadding = ImVec2(6,6);

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

                if (scene.tile_layers.empty()) {
                    selectedLayer = -1;
                } else if (selectedLayer >= (int)scene.tile_layers.size()) {
                    selectedLayer = (int)scene.tile_layers.size() - 1;
                } else if (selectedLayer < -1) {
                    selectedLayer = -1;
                }

            }

            ImGui::End();
            ImGui::PopStyleColor(3);
            ImGui::GetStyle().WindowPadding = orig;

    };

    auto strip_ext = [](const std::string& s) {
        size_t pos = s.find_last_of('.');
        return (pos == std::string::npos) ? s : s.substr(0, pos);
    };
        

    auto DistancePointToSegment = [](ImVec2 p, ImVec2 a, ImVec2 b) {
        ImVec2 ab = { b.x - a.x, b.y - a.y };
        ImVec2 ap = { p.x - a.x, p.y - a.y };

        float ab_len2 = ab.x * ab.x + ab.y * ab.y;
        if (ab_len2 == 0.0f) return sqrtf(ap.x * ap.x + ap.y * ap.y);

        float t = (ap.x * ab.x + ap.y * ab.y) / ab_len2;
        t = std::max(0.0f, std::min(1.0f, t));

        ImVec2 closest = {
            a.x + ab.x * t,
            a.y + ab.y * t
        };

        float dx = p.x - closest.x;
        float dy = p.y - closest.y;

        return sqrtf(dx * dx + dy * dy);
    };

    

    std::function<void()> DrawCreateAvatarEditor = [&] () {

        static bool lock_window_drag = false;
        static int jointselected = -1;

        static bool sterilizeJointForPopup = false;
        static bool sterilizeJointForAnimPopup = false;

        auto get_idx_from_jlayer_id = [&](Avatar& avatar, int layer) -> int {

            if (layer >= 0 && layer < avatar.default_frame.joints.size())
                return layer;
            return -1;

        };
        
        ImVec2 orig = ImGui::GetStyle().WindowPadding;
        ImGui::GetStyle().WindowPadding = ImVec2(6,6);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f,0.09f,0.12f,1));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f,0.12f,0.16f,1));
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f,0.5f,0.7f,0.6f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f,0.7f,0.9f,0.8f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f,0.4f,0.6f,0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f,0.6f,0.9f,1.0f));

        ImVec2 windowSize = ImVec2(800 * fullscreenScale.x, 500 * fullscreenScale.y);
        ImVec2 windowPos = ImVec2(80 * fullscreenScale.x, 50 * fullscreenScale.y);

        if (dcaeJustOpened) {
            ImGui::SetNextWindowPos(windowPos);
            ImGui::SetNextWindowSize(windowSize);
            dcaeJustOpened = false;
        }

        ImGuiWindowFlags flags = 0;

        if (lock_window_drag) {
            flags |= ImGuiWindowFlags_NoMove;
        }


        static int avatar_selected_idx = -1;
        static Avatar* avatar_selected = nullptr;
        static int anim_selected = -1;
        static bool confirm_delete_avatar_popup = false;
        static bool confirm_delete_animation_popup = false;

        static int selected_anim_frame = -1;

        static bool editing_anim_w_interpolation = false;

        static int timings_preview_frame = -1;


        static bool onion_frame_anch = false;
        static bool onion_frame_prev = false;

        static bool show_preview_joints = true;

        static bool play_preview_animation = false;

        static float preview_anim_accumulated_ms = 0;

        if (ImGui::Begin("Create Avatar...", nullptr, flags)) {
            //@@@@

            if (avatarMenu == AvatarCreatorMenu::AVATAR_SELECTION) {


                float availHeaderWidth = ImGui::GetContentRegionAvail().x - 3.0f;

                // LEFT HEADER
                ImGui::BeginChild("LeftHeader", ImVec2((availHeaderWidth/2.0f)-3.0f, 28), true);
                ImGui::Text("Select an Avatar anchor base to open the editor with");
                ImGui::EndChild();


                ImGui::SameLine();

                // RIGHT HEADER
                ImGui::BeginChild("RightHeader", ImVec2((availHeaderWidth/2.0f)-3.0f, 28), true);

                if (avatar_selected_idx != -1) {
                    ImGui::Text("Select an Animation to open Avatar/Animation editor with");
                }

                ImGui::EndChild();





                float availChildWidth = ImGui::GetContentRegionAvail().x;
                int children = 4; // Number of children
                float childWidth = (availChildWidth / (float)children) - 6.0f;

                // CHILD 1:
                // ==========
                // Select Avatar Pane


                int SAP_buttonCount = 5; // Number of buttons
                float SAP_buttonWidth = (childWidth / (float)SAP_buttonCount) - 8.5f;

                ImGui::BeginChild("SelectAvatarPane", ImVec2(childWidth, ImGui::GetContentRegionAvail().y), true);
            
                if (ImGui::Button("New", ImVec2(SAP_buttonWidth, 28))) {
                    avatars.push_back(Avatar("Untitled Avatar"));
                }

                ImGui::SameLine();
                
                if (ImGui::Button("Copy", ImVec2(SAP_buttonWidth, 28)) && avatar_selected_idx >= 0 && avatar_selected_idx < (int)avatars.size()) {
                    Avatar copied = avatars[avatar_selected_idx];
                    copied.name += " Copy";
                    avatars.push_back(copied);
                    avatar_selected_idx = (int)avatars.size() - 1;
                    avatar_selected = &avatars[avatar_selected_idx];
                }

                ImGui::SameLine();

                if (ImGui::Button("Delete", ImVec2(SAP_buttonWidth, 28)) && avatar_selected_idx >= 0 && avatar_selected_idx < (int)avatars.size()) {
                    const std::string filepath = AVATARDIR + avatars[avatar_selected_idx].name + ".avr";
                    if (std::filesystem::exists(filepath)) {
                        confirm_delete_avatar_popup = true;
                    } else {
                        avatars.erase(avatars.begin() + avatar_selected_idx);
                        avatar_selected_idx = -1;
                        avatar_selected = nullptr;
                        anim_selected = -1;
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button("Move ^", ImVec2(SAP_buttonWidth, 28)) && avatar_selected_idx > 0 && avatar_selected_idx < (int)avatars.size()) {
                    std::swap(avatars[avatar_selected_idx], avatars[avatar_selected_idx - 1]);
                    avatar_selected_idx--;
                    avatar_selected = &avatars[avatar_selected_idx];
                }

                ImGui::SameLine();

                if (ImGui::Button("Move v", ImVec2(SAP_buttonWidth, 28)) && avatar_selected_idx >= 0 && avatar_selected_idx < (int)avatars.size() - 1) {
                    std::swap(avatars[avatar_selected_idx], avatars[avatar_selected_idx + 1]);
                    avatar_selected_idx++;
                    avatar_selected = &avatars[avatar_selected_idx];
                }

                ImGui::Separator();

                ImGui::Text("General Avatar Files:");

                ImGui::Separator();

                for (int i = 0; i < avatars.size(); i++) {
                    ImGui::PushID(i);
                    bool sel = (i == avatar_selected_idx);
                    if (ImGui::Selectable(avatars.at(i).name.c_str(), sel)) {
                        avatar_selected_idx = (sel) ? -1 : i;
                        if (avatar_selected_idx != -1)
                            avatar_selected = &avatars[avatar_selected_idx];
                    }

                    ImGui::PopID();
                }




                ImGui::EndChild();



                // CHILD 2:
                // ==========
                // Avatar Information Overview 

               

                // >>> Rename the Edited Avatar

                if (avatar_selected_idx != -1) {

                    ImGui::SameLine();
                    ImGui::BeginChild("AvatarInfoOverview", ImVec2(childWidth, ImGui::GetContentRegionAvail().y), true);

                
                    ImGui::Text("Rename Selected Avatar File:");

                    ImGui::Separator();

                    static int lastAvatarSelected = -1;
                    static char avatarNameBuffer[128] = "Untitled Avatar";

                    if (avatar_selected_idx != lastAvatarSelected) {
                        // Update avatarNameBuffer to contain the field with the avatar name change
                        memset(avatarNameBuffer, 0, sizeof(avatarNameBuffer));
                        strncpy(avatarNameBuffer, avatars[avatar_selected_idx].name.c_str(), sizeof(avatarNameBuffer) - 1);
                        avatarNameBuffer[sizeof(avatarNameBuffer) - 1] = '\0';
                        lastAvatarSelected = avatar_selected_idx;
                    }
                    

                    ImGui::InputText("##rename", avatarNameBuffer, IM_ARRAYSIZE(avatarNameBuffer));
                    avatars[avatar_selected_idx].name = std::string(avatarNameBuffer);

                    ImGui::Separator();

                    ImGui::Text("Avatar Info Preview:");

                    ImGui::Separator();

                    ImGui::EndChild();
                    
                    
                } else {
                    
                }

                
                


                // CHILD 3:
                // ==========
                // Select Animation Pane

                

                if (avatar_selected_idx != -1) {

                    ImGui::SameLine();
                    ImGui::BeginChild("SelectAnimationPane", ImVec2(childWidth, ImGui::GetContentRegionAvail().y), true);

                    
                    if (animations.size() > 0) {
                        ImGui::Text("General Animations");

                        ImGui::Separator();
                    }

                    

                    if (ImGui::Button("New Animation", ImVec2(-1, 28))) {
                        animations.push_back(Animation(std::string("Untitled Animation " + std::to_string(animations.size() + 1))));
                        animations[animations.size()-1].frames.clear();
                    }

		            ImGui::SameLine();
                    if (ImGui::Button("Copy Animation", ImVec2(-1, 28)) && anim_selected >= 0 && anim_selected < (int)animations.size()) {
                        Animation copied = animations[anim_selected];
                        copied.name += " Copy";
                        animations.push_back(copied);
                        anim_selected = (int)animations.size() - 1;
                    }
                    if (ImGui::Button("Delete Animation", ImVec2(-1, 28)) && anim_selected >= 0 && anim_selected < (int)animations.size()) {
                        const std::string filepath = ANIMATIONDIR + animations[anim_selected].name + ".anim";
                        if (std::filesystem::exists(filepath)) {
                            confirm_delete_animation_popup = true;
                        } else {
                            animations.erase(animations.begin() + anim_selected);
                            anim_selected = -1;
                            selected_anim_frame = -1;
                            play_preview_animation = false;
                        }
                    }
                    if (ImGui::Button("Move Animation Up", ImVec2(-1, 28)) && anim_selected > 0 && anim_selected < (int)animations.size()) {
                        std::swap(animations[anim_selected], animations[anim_selected - 1]);
                        anim_selected--;
                    }
                    if (ImGui::Button("Move Animation Down", ImVec2(-1, 28)) && anim_selected >= 0 && anim_selected < (int)animations.size() - 1) {
                        std::swap(animations[anim_selected], animations[anim_selected + 1]);
                        anim_selected++;
                    }

                    ImGui::Separator();

                    for (int i = 0; i < animations.size(); i++) {

                        bool anim_compatible = true;

                        if (animations.at(i).frames.size() > 0) {
                            if (animations.at(i).frames[0].joints.size() != (*avatar_selected).default_frame.joints.size()) {
                                anim_compatible = false;
                            }
                        }

                        ImGui::BeginDisabled(!anim_compatible);

                        ImGui::PushID(i+999);
                        bool sel = (i == anim_selected);
                        if (ImGui::Selectable(animations.at(i).name.c_str(), sel)) {
                            anim_selected = (sel) ? -1 : i;

                        }
                        ImGui::PopID();

                        ImGui::EndDisabled();


                    }

                    ImGui::Separator();

                    ImGui::EndChild();


                }

                

                // CHILD 4:
                // ==========
                // Animation Overview Pane (should show compatibility with selected Avatar)

                

                if (avatar_selected_idx != -1) {

                    if (anim_selected != -1 && avatar_selected_idx != -1 && avatar_selected != nullptr) {

                        ImGui::SameLine();
                        ImGui::BeginChild("AnimationOverviewPane", ImVec2(childWidth, ImGui::GetContentRegionAvail().y), true);

                        // >>> Rename the Edited Animation
                        static int lastAnimSelected = -1;
                        static char animNameBuffer[128] = "Untitled Animation 1";

                        if (avatar_selected_idx != -1) {

                            ImGui::Text("Rename Selected Animation:");

                            ImGui::Separator();
                            
                            if (anim_selected != lastAnimSelected) {
                                // Update animNameBuffer to contain the field with the avatar name change
                                memset(animNameBuffer, 0, sizeof(animNameBuffer));
                                strncpy(animNameBuffer, animations[anim_selected].name.c_str(), sizeof(animNameBuffer) - 1);
                                animNameBuffer[sizeof(animNameBuffer) - 1] = '\0';
                                lastAnimSelected = anim_selected;                                
                            }

                            ImGui::InputText("##renameanim", animNameBuffer, IM_ARRAYSIZE(animNameBuffer));
                            animations[anim_selected].name = std::string(animNameBuffer);

                        } else {
                            
                        }

                        const std::string avString = std::string("Edit " + animations[anim_selected].name +  " w/ --> " + (*avatar_selected).name);

                        if (ImGui::Button(avString.c_str(), ImVec2(-1, 28))) {

                            lastAnimSelected = -1;
                            (*avatar_selected).LoadInternalJointTextures(assets);
                            avatarMenu = AvatarCreatorMenu::KEYFRAME_EDITOR;
                        }

                        ImGui::EndChild();
                    }
                }

                if (confirm_delete_avatar_popup) {
                    ImGui::OpenPopup("Confirm Delete Avatar");
                    confirm_delete_avatar_popup = false;
                }

                if (confirm_delete_animation_popup) {
                    ImGui::OpenPopup("Confirm Delete Animation");
                    confirm_delete_animation_popup = false;
                }

                if (ImGui::BeginPopupModal("Confirm Delete Avatar", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Delete selected avatar and saved .avr file (if it exists)?");
                    if (ImGui::Button("Delete Avatar", ImVec2(130, 0))) {
                        if (avatar_selected_idx >= 0 && avatar_selected_idx < (int)avatars.size()) {
                            const std::string filepath = AVATARDIR + avatars[avatar_selected_idx].name + ".avr";
                            if (std::filesystem::exists(filepath)) {
                                std::filesystem::remove(filepath);
                            }
                            avatars.erase(avatars.begin() + avatar_selected_idx);
                            avatar_selected_idx = -1;
                            avatar_selected = nullptr;
                            anim_selected = -1;
                        }
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(100, 0))) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                if (ImGui::BeginPopupModal("Confirm Delete Animation", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Delete selected animation and saved .anim file (if it exists)?");
                    if (ImGui::Button("Delete Animation", ImVec2(130, 0))) {
                        if (anim_selected >= 0 && anim_selected < (int)animations.size()) {
                            const std::string filepath = ANIMATIONDIR + animations[anim_selected].name + ".anim";
                            if (std::filesystem::exists(filepath)) {
                                std::filesystem::remove(filepath);
                            }
                            animations.erase(animations.begin() + anim_selected);
                            anim_selected = -1;
                            selected_anim_frame = -1;
                            play_preview_animation = false;
                        }
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(100, 0))) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                
            
            } else if (avatarMenu == AvatarCreatorMenu::KEYFRAME_EDITOR) {

                bool saveAvrPopup = false;
                bool saveAnimPopup = false;
                bool leaveAvrEditorPopup = false;

                const std::string editKind = std::string((*avatar_selected).name + ": " + animations[anim_selected].name);

                if (ImGui::Button("<<")) {

                    leaveAvrEditorPopup = true;
                    
                }

                ImGui::SameLine();

                ImGui::Text(editKind.c_str());

                ImGui::SameLine();

                if (ImGui::Button("Save Avatar (.avr)")) {
                    if (avatar_selected != nullptr) {
                        saveAvrPopup = true;
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button("Save Animation (.anim)")) {
                    if (anim_selected != -1) {
                        saveAnimPopup = true;
                    }
                }

                ImGui::SameLine();
                
                ImGui::Checkbox("Lock Window (no drag)", &lock_window_drag);


                static int lastJointSelected = jointselected;
                static char jointnamebuffer[128] = "Untitled joint";
                static int next_joint_unique_id = 0;

                if (avatar_selected_idx != -1) 
                    avatar_selected = &avatars[avatar_selected_idx];
                if (avatar_selected != nullptr) {
                    for (const auto& joint : avatar_selected->default_frame.joints) {
                        next_joint_unique_id = std::max(next_joint_unique_id, joint.unique_id + 1);
                    }
                }


                // BEGINNING OF PANE


                ImGui::BeginChild("LeftPanel", ImVec2(220, 0), true); // EndChild --> 

                

                // Animation Frame Editing for DEFAULT AVATAR JOINTS

                static int temp_query_selected_anim_frame = -1;

                float availWidth = ImGui::GetContentRegionAvail().x;
                int buttonCount = 5; // Number of buttons in the row
                float buttonWidth = availWidth / buttonCount - ImGui::GetStyle().ItemSpacing.x; // subtract spacing

                bool frame_control_disabled = false;

                if (avatar_selected_idx != -1 && anim_selected != -1) {

                    for (KeyAnimFrame frame : animations[anim_selected].frames) {

                        // If there is a mismatch in joint count between an animation being checked and an avatars' default frame,
                        // frame_control_disabled should be set to true immediately so we can deal with it
                        if ( avatars[avatar_selected_idx].default_frame.joints.size() != frame.joints.size() ) {
                            frame_control_disabled = true;
                            break;
                        }
                    }

                    if (avatars[avatar_selected_idx].default_frame.joints.size() < 1)
                        frame_control_disabled = true;

                    if (frame_control_disabled) {

                        // TODO @IMAWarriors
                        // ----------------------------
                        // Add a flag that basically notices there is an issue that the animation frames
                        // defined do not match with the setup joints of the Avatar, and if flag is noticed,
                        // pop-up a menu that says the animations and avatar do not match, would you like to 
                        // PRESERVE ANIMATION and exit the avatar editor to open different avatar + animation?
                        // or PRESERVE AVATAR and purge all frames whose joints do not align,
                        //      (or just purge all animation frames and start over)
                        // ==================================

                        // Notes for why there would be a mismatch?:
                        // -------------------------------------------------------
                        //  --> This could either be a small mismatch coming from accidental
                        //      deletion or such
                        //  --> OR this could be a mismatch in that if the animation has more joints than the Avatar's default frame,
                        //      ----// those animation joints should be purged (ESPECIALLY IF THERE IS MISMATCH BETWEEN ANIMATION FRAMES, NOT JUST DEFAULT FRAME)
                        //      ----// but if animation can match Avatar's frames by simply ignoring a few joints,
                        //             those joints should NOT be purged, but just ignored
                        //  --> OR ELSE this could be a large mismatch and this animation is not intended for this Avatar

                        // PRACTICAL SOLUTION 
                        // (to also like keep this from happened and harden the Avatar frame vs. Animation frames editor distinction:
                        // (because thats the greatest weakness of this application right now for sure, is the Avatar vs. Animation conflation;
                        //  it is organized in theory but its easy to forget about some principle that must be preserved for it to continue flawlessly)
                        // --------------------------------
                        // This is all complicated error handling, but, to keep things simple, it is probably best
                        // to first just ensure an Avatar is saved and its joint count is known before an animation
                        // is attempting to be edited for it-----animations with 0 animations are special;
                        // Avatars cannot be opened without an animation selected, so while an animation does have to be chosen,
                        // an animation with 0 frames will create its frames correctly based on the selected avatar's default frame joints.
                        // This should be made clear somehow in creation tab for animations for first first animation or something like
                        // -----
                        // HEY YOURE ESTABLISHING AN ANIMATION RIG BASED ON YOUR AVATAR RIG, AND THE ANIMATINO WILL BE BOUND TO RIGS THAT HAVE
                        // OVERLAPPING JOINT DESCRIPTIONS, REALLY ONLY WORKING BEST IF THEY HAVE ALL THE SAME JOINT DESCRIPTIONS AS AVATAR BASE FRAME DESCS,
                        // IS THAT OKAY?
                        // Prolly more formal than that but we do need to handle this somehow eventually....

                        


                    }

                } 

                // ============================================================
                // TIMELINE SETTINGS --> Have to establish in this scope rather than in Timings tab
                //                       so that the Canvas can read interpolations between frames and such
                // ============================================================


                static int currentFrame = 0;

                // Get the proper length of the animation in ticks
                int animation_tick_frame_length = 0;

                for (const KeyAnimFrame& kframe : animations[anim_selected].frames) {
                    animation_tick_frame_length += kframe.time_to_next + 1;
                }

                // If the preview setting is open and we are playing the animation,
                // go ahead and move the current Frame ticker forward

                if (anim_selected != -1 && play_preview_animation) {

                    preview_anim_accumulated_ms += (float)(1.0f / INTENDED_FPS) * (1000.0f);

                    if (preview_anim_accumulated_ms > animations[anim_selected].ms_per_tick_frame) {
                        preview_anim_accumulated_ms -= animations[anim_selected].ms_per_tick_frame;
                        currentFrame++;

                        if (currentFrame > animation_tick_frame_length - 1) {
                            currentFrame = 0;
                        }
                    }
                } else {
                    preview_anim_accumulated_ms = 0;
                }
                

                // THIS is the important part
                static float timelineScroll = 0.0f;

                // zoom level for time spacing
                static float pixelsPerFrame = 18.0f;

                // ----------------------------------------------------------------

                static bool manual_anim_frame_switch = false;
                static bool force_open_timing_tab = false;

                ImGui::BeginDisabled(frame_control_disabled);
                
                if (ImGui::Button("+", ImVec2(buttonWidth, 0))) {

                    if (avatar_selected_idx != -1) 
                        animations[anim_selected].new_frame(avatars[avatar_selected_idx]);

                    temp_query_selected_anim_frame = animations[anim_selected].frames.size() - 1; // Shouold return -1 if the size is 0, so default frame
                    manual_anim_frame_switch = true;


                }

                ImGui::SameLine();

                if (ImGui::Button("-", ImVec2(buttonWidth, 0))) {

                    if (temp_query_selected_anim_frame != -1) {
                        animations[anim_selected].frames.erase(animations[anim_selected].frames.begin() + temp_query_selected_anim_frame);
                        temp_query_selected_anim_frame = -1;
                        manual_anim_frame_switch = true;
                    }

                    animations[anim_selected].sync_frame_order_seq_id();

                }

                ImGui::SameLine();

                if (ImGui::Button("^", ImVec2(buttonWidth, 0))) {
                    if (temp_query_selected_anim_frame != -1 && temp_query_selected_anim_frame > 0 && temp_query_selected_anim_frame < animations[anim_selected].frames.size()) {
                        std::swap(animations[anim_selected].frames[temp_query_selected_anim_frame], animations[anim_selected].frames[temp_query_selected_anim_frame - 1]);
                        temp_query_selected_anim_frame--;
                        animations[anim_selected].sync_frame_order_seq_id();
                        manual_anim_frame_switch = true;
                    }
                }

                ImGui::SameLine();


                if (ImGui::Button("v", ImVec2(buttonWidth, 0))) {
                    if (temp_query_selected_anim_frame != -1 && temp_query_selected_anim_frame >= 0 && temp_query_selected_anim_frame < animations[anim_selected].frames.size()-1) {
                        std::swap(animations[anim_selected].frames[temp_query_selected_anim_frame], animations[anim_selected].frames[temp_query_selected_anim_frame + 1]);
                        temp_query_selected_anim_frame++;
                        animations[anim_selected].sync_frame_order_seq_id();
                        manual_anim_frame_switch = true;
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button("*", ImVec2(buttonWidth, 0))) {
                    // Duplicate this animation frame and switch to it

                    if (selected_anim_frame != -1) {

                        // Call the constructor to copy another frame, still need an avatar to link it to for Original joint draw order
                        if (avatar_selected_idx != -1) 
                            animations[anim_selected].new_frame(avatars[avatar_selected_idx], animations[anim_selected].frames[selected_anim_frame]);

                        temp_query_selected_anim_frame = animations[anim_selected].frames.size() - 1; // Shouold return -1 if the size is 0, so default frame
                        manual_anim_frame_switch = true;

                    } else {

                        // Just build a new frame with the default anchor positioning

                        if (avatar_selected_idx != -1) 
                            animations[anim_selected].new_frame(avatars[avatar_selected_idx]);

                        temp_query_selected_anim_frame = animations[anim_selected].frames.size() - 1; // Shouold return -1 if the size is 0, so default frame
                        manual_anim_frame_switch = true;

                    }
                }

                ImGui::EndDisabled();


                ImGui::Separator();

                //$$$
                if (selected_anim_frame == -1) {

                    // selection window
                    ImGui::Text("All Joints (Anchor)");

                    static int moveDX = 0;
                    static int moveDY = 32;
                    ImGui::DragInt("DX", &moveDX, 1.0f, -64, 64);
                    ImGui::DragInt("DY", &moveDY, 1.0f, -64, 64);

                    if (ImGui::Button("Move Anchor")) {
                        for (JointFramePosition& joint : (*avatar_selected).default_frame.joints) {
                            joint.origin.x += (float)moveDX;
                            joint.origin.y += (float)moveDY;
                        }
                    }

                    ImGui::Separator();

                    // frame control
                    if (!frame_control_disabled) {

                        float bwidth = (ImGui::GetContentRegionAvail().x / 2.0) - 5.0f;

                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f,0.1f,0.1f,0.8f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f,0.2f,0.2f,0.8f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f,0.3f,0.3f,0.8f));

                        if (ImGui::Button("Open Editor", ImVec2(bwidth, 24))) {

                            if (!frame_control_disabled && !animations[anim_selected].frames.empty()) {
                                selected_anim_frame = 0;
                                currentFrame = 0;
                                play_preview_animation = false;
                                force_open_timing_tab = (jointselected == -1);
                            }

                        }
                        
                        ImGui::PopStyleColor(3);

                        ImGui::SameLine();

                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f,0.3f,0.3f,0.8f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f,0.6f,0.6f,0.8f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f,0.1f,0.1f,0.8f));
                        
                        if (ImGui::Button("Play Anim", ImVec2(bwidth, 24))) {

                            if (!frame_control_disabled && !animations[anim_selected].frames.empty()) {
                                selected_anim_frame = 0;
                                currentFrame = 0;
                                play_preview_animation = true;
                                jointselected = -1;
                                force_open_timing_tab = (jointselected == -1);
                                
                            }
                        
                        }

                        ImGui::PopStyleColor(3);

                        ImGui::Separator();

                        if (frame_control_disabled) {
                            
                            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Animation editor unavailable:");
                            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "selected avatar anchor");
                            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "joints are incompatible with");
                            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "this animation.");

                        }

                    }

                } else {

                    ImGui::Text("All Joints (Frame)");

                    static int moveDX = 0;
                    static int moveDY = 32;

                    ImGui::DragInt("DX", &moveDX, 1.0f, -64, 64);
                    ImGui::DragInt("DY", &moveDY, 1.0f, -64, 64);

                    if (ImGui::Button("Move Animation")) {
                        for (AnimJointAdjustmentFrame& joint : animations[anim_selected].frames[selected_anim_frame].joints) {
                            joint.origin.x += (float)moveDX;
                            joint.origin.y += (float)moveDY;
                        }
                    }

                    ImGui::Separator();
                    //

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.2f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.45f, 0.15f, 1.0f)); // Slightly darker green on hover
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.05f, 0.3f, 0.05f, 1.0f));  // Even darker green on active
                    if (ImGui::Button("Return to Anchor Frame")) {

                        selected_anim_frame = -1;
                        currentFrame = 0;
                        play_preview_animation = false;
                        
                    }
                    ImGui::PopStyleColor(3);

                }

                if (anim_selected != -1) {

                    // Push the background color style for the selectable item
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.5f, 0.2f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.15f, 0.45f, 0.15f, 1.0f)); // Slightly darker green on hover
                    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.05f, 0.3f, 0.05f, 1.0f));  // Even darker green on active

                    if (selected_anim_frame != -1) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.6f));
                    }

                    // Draw the "Anchor Frame" selectable as the first item
                    if (ImGui::Selectable("Anchor Frame", (selected_anim_frame == -1))) {
                        temp_query_selected_anim_frame = -1;
                        manual_anim_frame_switch = true;
                        play_preview_animation = false;
                    }

                    if (selected_anim_frame != -1) {
                        ImGui::PopStyleColor();
                    }

                    // --------------->

                    // Pop the style colors to revert back to default for the rest of the list
                    ImGui::PopStyleColor(3);

                    if (selected_anim_frame == -1) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.6f));
                    }

                    // Draw the rest of the animation frames
                    for (int i = 0; i < animations[anim_selected].frames.size(); i++) {
                        ImGui::PushID(i);
                        bool sel = (i == selected_anim_frame);

                        ImGui::BeginDisabled(frame_control_disabled);

                        const std::string framename = std::string("Fr. #") + std::to_string(i);

                        if (ImGui::Selectable(framename.c_str(), sel)) {
                            temp_query_selected_anim_frame = i;
                            manual_anim_frame_switch = true;
                        }

                        ImGui::EndDisabled();

                        ImGui::PopID();
                    }

                    if (selected_anim_frame == -1) {
                        ImGui::PopStyleColor();
                    }

                }

                ImGui::Separator();


                // Query to avoid breaking stuff?

                if (manual_anim_frame_switch && temp_query_selected_anim_frame != selected_anim_frame) {
                    selected_anim_frame = temp_query_selected_anim_frame;
                    manual_anim_frame_switch = false;
                }

                if (selected_anim_frame == -1) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
                    ImGui::Text("Selected Anim Frame Index: -1 (Avatar Anchor)");
                    ImGui::PopStyleColor();
                } else {
                    ImGui::Text(std::string("Selected Anim Frame Index: " + std::to_string(selected_anim_frame)).c_str());
                }

                ImGui::Separator();

                // Joint Operations on WHOLE AVATAR, which should ONLY
                // affect the
                // ANCHOR FRAME

                if (selected_anim_frame == -1) {
                    auto syncAvatarJointIdsAndLayering = [&](Avatar& avatar) {
                        avatar.assign_unique_anchor_ids();
                        next_joint_unique_id = (int)avatar.default_frame.joints.size();
                    };

                    ImGui::Text("Default Frame Joints");

                    ImGui::Separator();

                    if (ImGui::Button("Add Joint", ImVec2(-1, 28))) {

                        int num1 = randInt(20,32);
                        int num2 = randInt(20,32);
                        int num3 = 32;
                        int num4 = 0;

                        if (avatar_selected != nullptr && avatar_selected_idx != -1) {
                        
                            // Create a new instance of the structure for the rendering of a joint (image, crop, offset, ...)
                            // and Create a new instance of the structure for the placement of a joint (position, direction, layerid)
                            avatars[avatar_selected_idx].default_texturing.joints.push_back(AvatarJoint("Untitled Joint"));
                            JointFramePosition new_pos({(float)num1, (float)num2}, {(float)num3, (float)num4});
                            new_pos.unique_id = next_joint_unique_id++;
                            avatars[avatar_selected_idx].default_frame.joints.push_back(new_pos);
                            syncAvatarJointIdsAndLayering(avatars[avatar_selected_idx]);
                        }

                    }

                    if (ImGui::Button("Delete Joint", ImVec2(-1, 28))) {
                        if (avatar_selected != nullptr) {
                            
                            if (jointselected >= 0 && jointselected < (*avatar_selected).default_texturing.joints.size()) {
                                (*avatar_selected).default_texturing.joints.erase((*avatar_selected).default_texturing.joints.begin() + jointselected);
                                (*avatar_selected).default_frame.joints.erase((*avatar_selected).default_frame.joints.begin() + jointselected);
                                syncAvatarJointIdsAndLayering(*avatar_selected);
                            }
                            jointselected = -1;
                            lastJointSelected = -1;
                            jointnamebuffer[0] = '\0';
                        }

                    }


                    if (ImGui::Button("* Duplicate Joint", ImVec2(-1, 28))) {

                        if (avatar_selected != nullptr && avatar_selected_idx != -1) {

                            if (jointselected != -1) {

                                AvatarJoint dupeText = (*avatar_selected).default_texturing.joints[jointselected];
                                JointFramePosition dupePos = (*avatar_selected).default_frame.joints[jointselected];
                                
                                (*avatar_selected).default_texturing.joints.push_back(AvatarJoint(dupeText.name + " cpy"));
                                JointFramePosition copied_pos({dupePos.origin.x + 20.0f, dupePos.origin.y + 20.0f}, {dupePos.direction.x, dupePos.direction.y});
                                copied_pos.unique_id = next_joint_unique_id++;
                                (*avatar_selected).default_frame.joints.push_back(copied_pos);
                                syncAvatarJointIdsAndLayering(*avatar_selected);

                                jointselected = (*avatar_selected).default_texturing.joints.size()-1;

                                if (dupeText.texture != nullptr)
                                    (*avatar_selected).default_texturing.joints[jointselected].load_texture_from_path(assets, dupeText.texturePath);

                                (*avatar_selected).default_texturing.joints[jointselected].crop_max = dupeText.crop_max;
                                (*avatar_selected).default_texturing.joints[jointselected].crop_min = dupeText.crop_min;
                                (*avatar_selected).default_texturing.joints[jointselected].rotation = dupeText.rotation;
                                (*avatar_selected).default_texturing.joints[jointselected].scale = dupeText.scale;
                                (*avatar_selected).default_texturing.joints[jointselected].offset = dupeText.offset;
                                
                            }
                        }

                    }
                
                    if (ImGui::Button("^ Joint Up Layer", ImVec2(-1, 28))) {

                        if (avatar_selected != nullptr  && avatar_selected_idx != -1) {

                            if (jointselected != -1 && (*avatar_selected).default_texturing.joints.size() >= 2 && jointselected >= 1 && jointselected < (*avatar_selected).default_texturing.joints.size()) {
                                
                                int idx1 = get_idx_from_jlayer_id(*avatar_selected, jointselected);
                                int idx2 = get_idx_from_jlayer_id(*avatar_selected, jointselected-1);
                                
                                
                                
                                AvatarJoint tempText = (*avatar_selected).default_texturing.joints[idx1];
                                JointFramePosition tempPos = (*avatar_selected).default_frame.joints[idx1];

                                (*avatar_selected).default_texturing.joints[idx1] = (*avatar_selected).default_texturing.joints[idx2];
                                (*avatar_selected).default_texturing.joints[idx2] = tempText;

                                (*avatar_selected).default_frame.joints[idx1] = (*avatar_selected).default_frame.joints[idx2];
                                (*avatar_selected).default_frame.joints[idx2] = tempPos;

                                jointselected = idx2;
                                syncAvatarJointIdsAndLayering(*avatar_selected);
                                
                                jointnamebuffer[0] = '\0';
                            }
                        }

                        
                    }

                    if (ImGui::Button("v Joint Down Layer", ImVec2(-1, 28))) {

                        if (avatar_selected != nullptr && avatar_selected_idx != -1) {

                            if (jointselected != -1 && (*avatar_selected).default_texturing.joints.size() >= 2 && jointselected >= 0 && jointselected < (*avatar_selected).default_texturing.joints.size()-1) {
                                
                                int idx1 = get_idx_from_jlayer_id(*avatar_selected, jointselected);
                                int idx2 = get_idx_from_jlayer_id(*avatar_selected, jointselected+1);
                                
                                AvatarJoint tempText = (*avatar_selected).default_texturing.joints[idx1];
                                JointFramePosition tempPos = (*avatar_selected).default_frame.joints[idx1];

                                (*avatar_selected).default_texturing.joints[idx1] = (*avatar_selected).default_texturing.joints[idx2];
                                (*avatar_selected).default_texturing.joints[idx2] = tempText;

                                (*avatar_selected).default_frame.joints[idx1] = (*avatar_selected).default_frame.joints[idx2];
                                (*avatar_selected).default_frame.joints[idx2] = tempPos;

                                jointselected = idx2;
                                syncAvatarJointIdsAndLayering(*avatar_selected);
                            }
                        }
                    }
                }


                

                ImGui::Separator();

                

                // List of joints
                for (int i = 0; i < (*avatar_selected).default_texturing.joints.size(); i++) {
                    ImGui::PushID(i);

                    std::string disp = std::string((*avatar_selected).default_texturing.joints.at(get_idx_from_jlayer_id((*avatar_selected), i)).name + " | Unique JID: " + std::to_string((*avatar_selected).default_frame.joints.at(get_idx_from_jlayer_id((*avatar_selected), i)).unique_id));

                    bool sel = (i == jointselected);
                    if (ImGui::Selectable( disp.c_str(), sel )) {
                        if (!sterilizeJointForPopup && !sterilizeJointForAnimPopup) {
                            jointselected = (sel) ? -1 : get_idx_from_jlayer_id((*avatar_selected), i);
                        }
                    }
                    ImGui::PopID();
                }

                ImGui::Separator();


                if (editing_anim_w_interpolation) {
                    ImGui::Text("Previewer:");

                    ImGui::Checkbox("Show Joints", &show_preview_joints);

                }




                ImGui::EndChild();


                

                ImGui::SameLine();

                float factor = 1.0f;

                if (jointselected != -1 || selected_anim_frame != -1) {
                    factor = 0.75f;
                }

                ImGui::BeginChild("CenterView", ImVec2(ImGui::GetContentRegionAvail().x * factor, ImGui::GetContentRegionAvail().y), true);


                ImVec2 canvasPos = ImGui::GetCursorScreenPos();
                ImVec2 canvasSize;


                // controller_bar info
                // ---------------------
                // Animation Preview Controller
                // --> For tweaking the position of the scrubber,
                //     and the current animation state so that animation
                //     and transitions can be previewed and edited 
                //     accordingly
                // --> DVD Player esq control options
                // ============================
                // Button Options:
                //
                // [ |< ] [ << ] [ |> / || ] [ >> ] [ >| ] [ # ]
                //
                // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                // --> [ |> ] / [ || ] : Play / Pause the animation playing
                //
                //


                // Frame Animation Editor
                // ========================
                //
                // 
                //


                float controller_bar_height = 165.0f;

                
                if (editing_anim_w_interpolation) {
                    canvasSize = {ImGui::GetContentRegionAvail().x, (ImGui::GetContentRegionAvail().y - controller_bar_height) };
                } else {
                    canvasSize = {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y};
                }


                ImDrawList* draw = ImGui::GetWindowDrawList();
                
                // CANVAS

                static Vec2 local_canvas_scroll = {-80.0f, -250.0f};
                static float local_canvas_zoom = 1.5f;

                static bool draggingLastFrame = false;
                static ImVec2 drag_start;
                static ImVec2 mouse_start;
                

                auto WorldToScreen = [&](ImVec2 pos) {
                    return ImVec2(
                        canvasPos.x + (pos.x - local_canvas_scroll.x) * local_canvas_zoom,
                        canvasPos.y + (pos.y - local_canvas_scroll.y) * local_canvas_zoom
                    );
                };




                
                // ============================================
                // Draw joints in Avatar/Joint Editor Code
                // ***********************
                //
                // ============================================

                // Some helpful helper functions...

                auto RotNewDirectionVec = [](Vec2 input_direction, float rotation_deg) -> Vec2 {

                    // Using code, take in a 2D Vector for direction and, given a rotation,
                    // preserve the magnitude of the vector while rotating the vector by rotation_deg (degrees),
                    // Outputting the new 2D vector

                    // Convert degrees to radians
                    float rotation_rad = rotation_deg * 3.14159265358979323846f / 180.0f;
                    float cos_theta = cos(rotation_rad);
                    float sin_theta = sin(rotation_rad);

                    // Rotate vector preserving magnitude
                    float new_x = input_direction.x * cos_theta - input_direction.y * sin_theta;
                    float new_y = input_direction.x * sin_theta + input_direction.y * cos_theta;

                    return Vec2{new_x, new_y};
                };

                auto RotNewPositionVec = [&RotNewDirectionVec](Vec2 input_position, Vec2 input_direction, float rotation_deg) -> Vec2 {

                    // Same thing but output the absolute position, not the direction which functions
                    // as a vector from the previous vector;
                    // --> I want a vector to output absolute vector of new final position of node

                    // Rotate the direction vector
                    Vec2 rotated_direction = RotNewDirectionVec(input_direction, rotation_deg);

                    // Add rotated direction vector to input position to get new absolute position
                    return Vec2{input_position.x + rotated_direction.x, input_position.y + rotated_direction.y};
                    
                };

                auto RotDegBetweenPoints = [](Vec2 orig_position, Vec2 dir_position) -> float {
                    // Calculate the vector from orig_position to dir_position
                    float dx = dir_position.x - orig_position.x;
                    float dy = dir_position.y - orig_position.y;

                    // Calculate the angle in radians between the vector and the positive X-axis
                    float angle_rad = atan2(dy, dx);

                    // Convert radians to degrees
                    float angle_deg = angle_rad * 180.0f / 3.14159265358979323846f;

                    // Optional: Normalize angle to [0, 360) degrees
                    if (angle_deg < 0) {
                        angle_deg += 360.0f;
                    }

                    return angle_deg;
                };

                auto WrapDeg180 = [](float angle) -> float {
                    float wrapped = fmodf(angle, 360.0f);
                    if (wrapped > 180.0f) wrapped -= 360.0f;
                    if (wrapped <= -180.0f) wrapped += 360.0f;
                    return wrapped;
                };


                ImU32 canv_col = IM_COL32(20,20,30,255);

                if (selected_anim_frame == -1) {
                    canv_col = IM_COL32(10,16,12,255);
                }

                // Draw canvas base

                draw->AddRectFilled(canvasPos, ImVec2(canvasPos.x+canvasSize.x, canvasPos.y + canvasSize.y), canv_col);

                // Get mouse position
                ImVec2 mpos = ImGui::GetIO().MousePos;


                // DRAW JOINTS and IMAGE TEXTURES in LOOP
                // **************************************

                // onion_frame_anch...
                
                
                // If selected_anim_frame == -1,
                // Then that means the ANCHOR FRAME is selected
                //
                // Any other selected frame indicates there is
                // a valid animation and a frame of that animation is chosen
            
                if (selected_anim_frame == -1) {

                    // --> ANCHOR FRAME: DRAW
                    for (int i = 0; i < (*avatar_selected).default_texturing.joints.size(); i++) {

                        int idx = get_idx_from_jlayer_id((*avatar_selected), i);

                        auto& j = (*avatar_selected).default_frame.joints[idx];
                        auto& j_text = (*avatar_selected).default_texturing.joints[idx];

                        ImVec2 a = WorldToScreen(ImVec2(j.origin.x, j.origin.y));
                        ImVec2 b = WorldToScreen(ImVec2(j.origin.x + j.direction.x,
                                                    j.origin.y + j.direction.y));

                        float dist = DistancePointToSegment(mpos, a, b);

                        bool hovered = dist < 8.0f;
                        bool selected = (idx == jointselected);

                        ImU32 color;

                        if (selected) {
                            color = IM_COL32(0, 200, 255, 255);   // bright cyan selected
                        }
                        else if (hovered) {
                            color = IM_COL32(80, 170, 255, 255);  // blue hover
                        }
                        else {
                            color = IM_COL32(255, 255, 255, 120); // default
                        }

                        float thickness = selected ? 6.0f : (hovered ? 5.0f : 3.0f);

                        draw->AddLine(a, b, color, thickness);

                        if (j_text.texture != nullptr) {
                            ImVec2 center = WorldToScreen(ImVec2(j.origin.x, j.origin.y));

                            float width  = j_text.texture->width  * j_text.scale.x * local_canvas_zoom;
                            float height = j_text.texture->height * j_text.scale.y * local_canvas_zoom;

                            // Direction → angle
                            ImVec2 dir = { j.direction.x, j.direction.y };
                            float baseAngle = atan2f(dir.y, dir.x);
                            float angle = baseAngle + j_text.rotation;

                            float cosA = cosf(angle);
                            float sinA = sinf(angle);

                            // Offset (in world space --> scaled)
                            ImVec2 offset = {
                                j_text.offset.x * local_canvas_zoom,
                                j_text.offset.y * local_canvas_zoom
                            };

                            // Define local quad (centered)
                            ImVec2 half = { width * 0.5f, height * 0.5f };

                            ImVec2 corners[4] = {
                                {-half.x, -half.y},
                                { half.x, -half.y},
                                { half.x,  half.y},
                                {-half.x,  half.y}
                            };

                            ImVec2 rotated[4];

                            for (int k = 0; k < 4; k++) {
                                float x = corners[k].x + offset.x;
                                float y = corners[k].y + offset.y;

                                rotated[k].x = center.x + (x * cosA - y * sinA);
                                rotated[k].y = center.y + (x * sinA + y * cosA);
                            }

                            ImVec2 uv0 = { j_text.crop_min.x, j_text.crop_min.y };
                            ImVec2 uv1 = { j_text.crop_max.x, j_text.crop_max.y };

                            draw->AddImageQuad(
                                (ImTextureID)(intptr_t)j_text.texture->id,
                                rotated[0],
                                rotated[1],
                                rotated[2],
                                rotated[3],
                                ImVec2(uv0.x, uv0.y),
                                ImVec2(uv1.x, uv0.y),
                                ImVec2(uv1.x, uv1.y),
                                ImVec2(uv0.x, uv1.y)
                            );
                        }

                        // CLICK SELECT
                        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                            if (!sterilizeJointForPopup && !sterilizeJointForAnimPopup) {
                                jointselected = idx;
                            }
                        }

                
                    }

                } else {


                    // --> First, if onion layering is on for the anchor frame,
                    //      TINT IT GREEN, MAKE IT TRANSPARENT, DISPLAY IT

                    if (onion_frame_anch) {
                        
                        

                        for (int i = 0; i < (*avatar_selected).default_texturing.joints.size(); i++) {

                            int idx = get_idx_from_jlayer_id((*avatar_selected), i);

                            auto& j = (*avatar_selected).default_frame.joints[idx];
                            auto& j_text = (*avatar_selected).default_texturing.joints[idx];

                            ImVec2 a = WorldToScreen(ImVec2(j.origin.x, j.origin.y));
                            ImVec2 b = WorldToScreen(ImVec2(j.origin.x + j.direction.x,
                                                        j.origin.y + j.direction.y));

                            float dist = DistancePointToSegment(mpos, a, b);
                            float thickness = 5.0f;

                            ImU32 color = IM_COL32(200, 200, 200, 60);

                            draw->AddLine(a, b, color, thickness);

                            if (j_text.texture != nullptr) {
                                ImVec2 center = WorldToScreen(ImVec2(j.origin.x, j.origin.y));

                                float width  = j_text.texture->width  * j_text.scale.x * local_canvas_zoom;
                                float height = j_text.texture->height * j_text.scale.y * local_canvas_zoom;

                                // Direction → angle
                                ImVec2 dir = { j.direction.x, j.direction.y };
                                float baseAngle = atan2f(dir.y, dir.x);
                                float angle = baseAngle + j_text.rotation;

                                float cosA = cosf(angle);
                                float sinA = sinf(angle);

                                // Offset (in world space --> scaled)
                                ImVec2 offset = {
                                    j_text.offset.x * local_canvas_zoom,
                                    j_text.offset.y * local_canvas_zoom
                                };

                                // Define local quad (centered)
                                ImVec2 half = { width * 0.5f, height * 0.5f };

                                ImVec2 corners[4] = {
                                    {-half.x, -half.y},
                                    { half.x, -half.y},
                                    { half.x,  half.y},
                                    {-half.x,  half.y}
                                };

                                ImVec2 rotated[4];

                                for (int k = 0; k < 4; k++) {
                                    float x = corners[k].x + offset.x;
                                    float y = corners[k].y + offset.y;

                                    rotated[k].x = center.x + (x * cosA - y * sinA);
                                    rotated[k].y = center.y + (x * sinA + y * cosA);
                                }

                                ImVec2 uv0 = { j_text.crop_min.x, j_text.crop_min.y };
                                ImVec2 uv1 = { j_text.crop_max.x, j_text.crop_max.y };

                                ImU32 ghostTint = IM_COL32(200, 255, 200, 90);


                                draw->AddImageQuad(
                                    (ImTextureID)(intptr_t)j_text.texture->id,
                                    rotated[0],
                                    rotated[1],
                                    rotated[2],
                                    rotated[3],
                                    ImVec2(uv0.x, uv0.y),
                                    ImVec2(uv1.x, uv0.y),
                                    ImVec2(uv1.x, uv1.y),
                                    ImVec2(uv0.x, uv1.y),
                                    ghostTint
                                );
                            }
                        }
                    }



                    // --> Second, if there are more than 1 frames in animation and it is supposed to be displayed
                    //      MAKE IT TRANSPARENT, DISPLAY IT

                    if (onion_frame_prev && anim_selected != -1) {

                        auto& anim = animations[anim_selected];

                        if (anim.frames.size() > 1) {
                            // ------------------->
                            // Code for drawing previous onion frame

                            int adj_anim_frame = selected_anim_frame - 1;

                            if (adj_anim_frame < 0) {
                                adj_anim_frame = anim.frames.size()-1;
                            }

                            for (int i = 0; i < (*avatar_selected).default_texturing.joints.size(); i++) {
                                
                                int idx = get_idx_from_jlayer_id((*avatar_selected), i);
                                auto& anchor_j = (*avatar_selected).default_frame.joints[idx];
                                auto& anim_j = animations[anim_selected].frames[adj_anim_frame].joints[idx];
                                auto& j_text = (*avatar_selected).default_texturing.joints[idx];

                                // Get accurate texture info
                                const ResolvedJointTexture resolved_texture = j_text.resolve_texture(anim_j.anim_texture_idx);
                                Texture2D* actual_texture = resolved_texture.texture;
                                float actual_texture_cmin_x = resolved_texture.crop_min.x;
                                float actual_texture_cmin_y = resolved_texture.crop_min.y;
                                float actual_texture_cmax_x = resolved_texture.crop_max.x;
                                float actual_texture_cmax_y = resolved_texture.crop_max.y;
                                float actual_texture_scale_x = resolved_texture.scale.x;
                                float actual_texture_scale_y = resolved_texture.scale.y;
                                float actual_texture_offset_x = resolved_texture.offset.x;
                                float actual_texture_offset_y = resolved_texture.offset.y;
                                float actual_texture_rotation = resolved_texture.rotation;
                                
                                float direction_point_x = 0.0f;
                                float direction_point_y = 0.0f;
                                direction_point_x = RotNewPositionVec({anchor_j.origin.x + anim_j.origin.x, anchor_j.origin.y + anim_j.origin.y}, anchor_j.direction, anim_j.rotation).x;
                                direction_point_y = RotNewPositionVec({anchor_j.origin.x + anim_j.origin.x, anchor_j.origin.y + anim_j.origin.y}, anchor_j.direction, anim_j.rotation).y;
                                ImVec2 a = WorldToScreen(ImVec2(anchor_j.origin.x + anim_j.origin.x, anchor_j.origin.y + anim_j.origin.y));
                                ImVec2 b = WorldToScreen(ImVec2(direction_point_x, direction_point_y));
                                float dist = DistancePointToSegment(mpos, a, b);
                                ImU32 color = IM_COL32(255, 255, 255, 80); // default
                                float thickness = 4.0f;
                                draw->AddLine(a, b, color, thickness);

                                if (actual_texture != nullptr) {
                                    ImVec2 center = WorldToScreen(ImVec2(anchor_j.origin.x + anim_j.origin.x, anchor_j.origin.y + anim_j.origin.y));
                                    float width  = actual_texture->width  * actual_texture_scale_x * local_canvas_zoom;
                                    float height = actual_texture->height * actual_texture_scale_y * local_canvas_zoom;
                                    Vec2 newDir = RotNewDirectionVec(anchor_j.direction, anim_j.rotation);
                                    ImVec2 dir = { newDir.x, newDir.y };
                                    float baseAngle = atan2f(dir.y, dir.x);
                                    float angle = baseAngle + actual_texture_rotation;
                                    float cosA = cosf(angle);
                                    float sinA = sinf(angle);
                                    ImVec2 offset = {
                                        actual_texture_offset_x * local_canvas_zoom,
                                        actual_texture_offset_y * local_canvas_zoom
                                    };
                                    ImVec2 half = { width * 0.5f, height * 0.5f };
                                    ImVec2 corners[4] = {
                                        {-half.x, -half.y},
                                        { half.x, -half.y},
                                        { half.x,  half.y},
                                        {-half.x,  half.y}
                                    };

                                    ImVec2 rotated[4];

                                    for (int k = 0; k < 4; k++) {
                                        float x = corners[k].x + offset.x;
                                        float y = corners[k].y + offset.y;
                                        rotated[k].x = center.x + (x * cosA - y * sinA);
                                        rotated[k].y = center.y + (x * sinA + y * cosA);
                                    }

                                    ImU32 ghostTint = IM_COL32(255, 255, 255, 120);
                                    ImVec2 uv0 = { actual_texture_cmin_x, actual_texture_cmin_y };
                                    ImVec2 uv1 = { actual_texture_cmax_x, actual_texture_cmax_y };

                                    draw->AddImageQuad(
                                        (ImTextureID)(intptr_t)actual_texture->id,
                                        rotated[0],
                                        rotated[1],
                                        rotated[2],
                                        rotated[3],
                                        ImVec2(uv0.x, uv0.y),
                                        ImVec2(uv1.x, uv0.y),
                                        ImVec2(uv1.x, uv1.y),
                                        ImVec2(uv0.x, uv1.y),
                                        ghostTint
                                    );
                                }
                            }
                        }


                        /*

                        EXTRA ONION LAYER FRAME TWO FRAMES AGO I THINK

                        if (anim.frames.size() > 2) {
                            // ------------------->
                            // Code for drawing previous onion frame

                            int adj_anim_frame = selected_anim_frame;

                            for (int i = 0; i < 2; i++) {
                                // Cycle back twice
                                adj_anim_frame--;
                                if (adj_anim_frame < 0) {
                                    adj_anim_frame = anim.frames.size()-1;
                                }
                            }

                            for (int i = 0; i < (*avatar_selected).default_texturing.joints.size(); i++) {
                            
                                int idx = get_idx_from_jlayer_id((*avatar_selected), i);
                                auto& anchor_j = (*avatar_selected).default_frame.joints[idx];
                                auto& anim_j = animations[anim_selected].frames[adj_anim_frame].joints[idx];
                                auto& j_text = (*avatar_selected).default_texturing.joints[idx];
                                float direction_point_x = 0.0f;
                                float direction_point_y = 0.0f;
                                direction_point_x = RotNewPositionVec({anchor_j.origin.x + anim_j.origin.x, anchor_j.origin.y + anim_j.origin.y}, anchor_j.direction, anim_j.rotation).x;
                                direction_point_y = RotNewPositionVec({anchor_j.origin.x + anim_j.origin.x, anchor_j.origin.y + anim_j.origin.y}, anchor_j.direction, anim_j.rotation).y;
                                ImVec2 a = WorldToScreen(ImVec2(anchor_j.origin.x + anim_j.origin.x, anchor_j.origin.y + anim_j.origin.y));
                                ImVec2 b = WorldToScreen(ImVec2(direction_point_x, direction_point_y));
                                float dist = DistancePointToSegment(mpos, a, b);
                                ImU32 color = IM_COL32(255, 255, 255, 45); // default
                                float thickness = 4.0f;
                                draw->AddLine(a, b, color, thickness);

                                if (j_text.texture != nullptr) {
                                    ImVec2 center = WorldToScreen(ImVec2(anchor_j.origin.x + anim_j.origin.x, anchor_j.origin.y + anim_j.origin.y));
                                    float width  = j_text.texture->width  * j_text.scale.x * local_canvas_zoom;
                                    float height = j_text.texture->height * j_text.scale.y * local_canvas_zoom;
                                    Vec2 newDir = RotNewDirectionVec(anchor_j.direction, anim_j.rotation);
                                    ImVec2 dir = { newDir.x, newDir.y };
                                    float baseAngle = atan2f(dir.y, dir.x);
                                    float angle = baseAngle + j_text.rotation;
                                    float cosA = cosf(angle);
                                    float sinA = sinf(angle);
                                    ImVec2 offset = {
                                        j_text.offset.x * local_canvas_zoom,
                                        j_text.offset.y * local_canvas_zoom
                                    };
                                    ImVec2 half = { width * 0.5f, height * 0.5f };
                                    ImVec2 corners[4] = {
                                        {-half.x, -half.y},
                                        { half.x, -half.y},
                                        { half.x,  half.y},
                                        {-half.x,  half.y}
                                    };

                                    ImVec2 rotated[4];

                                    for (int k = 0; k < 4; k++) {
                                        float x = corners[k].x + offset.x;
                                        float y = corners[k].y + offset.y;
                                        rotated[k].x = center.x + (x * cosA - y * sinA);
                                        rotated[k].y = center.y + (x * sinA + y * cosA);
                                    }

                                    ImU32 ghostTint = IM_COL32(255, 255, 255, 80);
                                    ImVec2 uv0 = { j_text.crop_min.x, j_text.crop_min.y };
                                    ImVec2 uv1 = { j_text.crop_max.x, j_text.crop_max.y };

                                    

                                    draw->AddImageQuad(
                                        (ImTextureID)(intptr_t)j_text.texture->id,
                                        rotated[0],
                                        rotated[1],
                                        rotated[2],
                                        rotated[3],
                                        ImVec2(uv0.x, uv0.y),
                                        ImVec2(uv1.x, uv0.y),
                                        ImVec2(uv1.x, uv1.y),
                                        ImVec2(uv0.x, uv1.y),
                                        ghostTint
                                    );
                                }

                            }
                        }

                        */

                    }


                    // --> Third, NORMAL FRAME: DRAW
                    // --------------------
                    // (Actual draw, no more onion layers)

                    const int joint_count = (*avatar_selected).default_texturing.joints.size();

                    // Allocate an array of the indicies to draw on the heap
                    int* jidx_draw_order = new int[joint_count];

                    // :: Step 1
                    // ---------------------
                    // Sort Code:
                    // ..................................
                    // Initialize the allocated memory to hold the correct joint draw order according to the frame

                    int draw_frame_idx = selected_anim_frame;

                    // In timing/interpolation mode, the canvas preview should follow scrubber position,
                    // not whichever frame is selected for manual editing in the frame list.
                    if (editing_anim_w_interpolation) {
                        int tick_cursor = 0;
                        draw_frame_idx = 0;
                        for (int k = 0; k < (int)animations[anim_selected].frames.size(); k++) {
                            int segment_len = std::max(1, animations[anim_selected].frames[k].time_to_next);
                            int segment_end = tick_cursor + segment_len + 1;
                            if (currentFrame >= tick_cursor && currentFrame < segment_end) {
                                draw_frame_idx = k;
                                break;
                            }
                            tick_cursor = segment_end;
                            draw_frame_idx = k;
                        }
                    }

                    draw_frame_idx = std::clamp(draw_frame_idx, 0, (int)animations[anim_selected].frames.size() - 1);

                    for (int i = 0; i < joint_count; i++) {

                        int jidx = 0;

                        for (int j = 0; j < joint_count; j++) {
                            if (i == animations[anim_selected].frames[draw_frame_idx].joints[j].draw_order || j >= animations[anim_selected].frames[draw_frame_idx].joints.size()) {
                                // The second case is a failsafe, but we should never be able to count past
                                // the joints in an animation otherwise theres a mismatch
                                assert(j < animations[anim_selected].frames[draw_frame_idx].joints.size() );
                                jidx = j;
                                break;
                            }
                        }
                        
                        jidx_draw_order[i] = jidx;
                    }

                    // :: Step 2
                    // ---------------------
                    // Draw Code:

                    // -------------- 
                    // Case Branch:
                    // ---> If we are in the FRAME editing tab OR we are in the TIMING editing tab with added stipulation
                    //      that If we CANNOT be in Playback/Preview Animation mode AND the scrubber must be on a KEY FRAME
                    //      >>  THEN and only then in TIMING MODE (or unambiguously in FRAME EDITING MODE) should we draw
                    //          according to a single keyframe
                    // ---> OTHERWISE, we must be in the Timing Editing tab and the scrubber must NOT be on a key frame
                    //      but rather between key frames (otherwise interpolation doesn't really work or make sense)
                    //      --> Playback mode may have its own beast to handle

                    int interp_frame_idx = selected_anim_frame;
                    int interp_frame_start_tick = 0;

                    // Find the keyframe segment containing currentFrame. We do this instead of relying solely
                    // on selected_anim_frame so interpolation remains stable while scrubbing.
                    if (!animations[anim_selected].frames.empty()) {
                        int tick_cursor = 0;
                        for (int k = 0; k < (int)animations[anim_selected].frames.size(); k++) {
                            int segment_len = animations[anim_selected].frames[k].time_to_next;
                            int segment_end = tick_cursor + segment_len + 1;
                            if (currentFrame >= tick_cursor && currentFrame < segment_end) {
                                interp_frame_idx = k;
                                interp_frame_start_tick = tick_cursor;
                                break;
                            }
                            tick_cursor = segment_end;
                        }

                    }


                    if (editing_anim_w_interpolation) {

                        // PLAYBACK / PREVIEW DRAW (with INTERPOLATION)

                        for (int i = 0; i < joint_count; i++) {

                            // 
                            int idx = jidx_draw_order[i];
                            auto& joint_anchor = (*avatar_selected).default_frame.joints[idx];
                            auto& joint_texture = (*avatar_selected).default_texturing.joints[idx];
                            auto& joint_anim = animations[anim_selected].frames[interp_frame_idx].joints[idx];

                            int segment_len = animations[anim_selected].frames[interp_frame_idx].time_to_next;
                            float percent_through_frame = ((float)(currentFrame - interp_frame_start_tick) / ((float)segment_len + 1));
                            percent_through_frame = std::clamp(percent_through_frame, 0.0f, 1.0f);

                            int anim_frame_to_interp = (interp_frame_idx + 1) % (animations[anim_selected].frames.size());

                            AnimJointAdjustmentFrame joint_interp = animations[anim_selected].frames[interp_frame_idx].joints[idx];
                            const AnimJointAdjustmentFrame& joint_blend = animations[anim_selected].frames[anim_frame_to_interp].joints[idx];

                            // Interpolation math  (too easy??)
                            joint_interp.origin.x = joint_anim.origin.x + ((joint_blend.origin.x - joint_anim.origin.x) * percent_through_frame);
                            joint_interp.origin.y = joint_anim.origin.y + ((joint_blend.origin.y - joint_anim.origin.y) * percent_through_frame);


                            float raw_delta = (joint_blend.rotation - joint_anim.rotation);
                            float shortest_delta = WrapDeg180(raw_delta);

                            if (joint_anim.normal_rotation) {
                                // Default behavior: rotate along shortest angular path.
                                joint_interp.rotation = joint_anim.rotation + (shortest_delta * percent_through_frame);
                            } else {
                                // Inverse behavior: rotate along the long way around.
                                float long_delta = (shortest_delta >= 0.0f) ? (shortest_delta - 360.0f) : (shortest_delta + 360.0f);
                                joint_interp.rotation = joint_anim.rotation + (long_delta * percent_through_frame);
                            }
                            

                            //AnimJointAdjustmentFrame joint_interpolation = animations[anim_selected].interpolate_joints(, i, currentFrame);
                            // The Primary place where we set [a] and [b], the two points for each joint and where they will be displayed

                            float direction_point_x = RotNewPositionVec({joint_anchor.origin.x + joint_interp.origin.x, joint_anchor.origin.y + joint_interp.origin.y}, joint_anchor.direction, joint_interp.rotation).x;
                            float direction_point_y = RotNewPositionVec({joint_anchor.origin.x + joint_interp.origin.x, joint_anchor.origin.y + joint_interp.origin.y}, joint_anchor.direction, joint_interp.rotation).y;

                            ImVec2 a = WorldToScreen(ImVec2(joint_anchor.origin.x + joint_interp.origin.x, joint_anchor.origin.y + joint_interp.origin.y));
                            ImVec2 b = WorldToScreen(ImVec2(direction_point_x, direction_point_y));

                            float dist = DistancePointToSegment(mpos, a, b);

                            if (show_preview_joints) {
                            
                                ImU32 color = IM_COL32(255, 255, 255, 120); // default

                                float thickness = 3.0f;

                                draw->AddLine(a, b, color, thickness);

                            }

                            const ResolvedJointTexture resolved_texture = joint_texture.resolve_texture(joint_anim.anim_texture_idx);
                            Texture2D* actual_texture = resolved_texture.texture;
                            float actual_texture_cmin_x = resolved_texture.crop_min.x;
                            float actual_texture_cmin_y = resolved_texture.crop_min.y;
                            float actual_texture_cmax_x = resolved_texture.crop_max.x;
                            float actual_texture_cmax_y = resolved_texture.crop_max.y;
                            float actual_texture_scale_x = resolved_texture.scale.x;
                            float actual_texture_scale_y = resolved_texture.scale.y;
                            float actual_texture_offset_x = resolved_texture.offset.x;
                            float actual_texture_offset_y = resolved_texture.offset.y;
                            float actual_texture_rotation = resolved_texture.rotation;

                            if (actual_texture != nullptr) {

                                // Center derivation... FIX JOINTS ANCHORED TO
                                ImVec2 center = WorldToScreen(ImVec2(joint_anchor.origin.x + joint_interp.origin.x, joint_anchor.origin.y + joint_interp.origin.y));

                                // Texture locale
                                float width  = actual_texture->width  * actual_texture_scale_x * local_canvas_zoom;
                                float height = actual_texture->height * actual_texture_scale_y * local_canvas_zoom;

                                // Direction --> angle
                                Vec2 newDir = RotNewDirectionVec(joint_anchor.direction, joint_interp.rotation);
                                ImVec2 dir = { newDir.x, newDir.y };
                                float baseAngle = atan2f(dir.y, dir.x);
                                float angle = baseAngle + actual_texture_rotation;
                                float cosA = cosf(angle);
                                float sinA = sinf(angle);

                                // Offset (in world space --> scaled)
                                ImVec2 offset = {actual_texture_offset_x * local_canvas_zoom, actual_texture_offset_y * local_canvas_zoom};

                                // Define local quad (centered)
                                ImVec2 half = { width * 0.5f, height * 0.5f };
                                ImVec2 corners[4] = {
                                    {-half.x, -half.y},
                                    { half.x, -half.y},
                                    { half.x,  half.y},
                                    {-half.x,  half.y}
                                };

                                ImVec2 rotated[4];

                                for (int k = 0; k < 4; k++) {
                                    float x = corners[k].x + offset.x;
                                    float y = corners[k].y + offset.y;
                                    rotated[k].x = center.x + (x * cosA - y * sinA);
                                    rotated[k].y = center.y + (x * sinA + y * cosA);
                                }

                                ImVec2 uv0 = { actual_texture_cmin_x, actual_texture_cmin_y };
                                ImVec2 uv1 = { actual_texture_cmax_x, actual_texture_cmax_y };

                                draw->AddImageQuad(
                                    (ImTextureID)(intptr_t)actual_texture->id,
                                    rotated[0],
                                    rotated[1],
                                    rotated[2],
                                    rotated[3],
                                    ImVec2(uv0.x, uv0.y),
                                    ImVec2(uv1.x, uv0.y),
                                    ImVec2(uv1.x, uv1.y),
                                    ImVec2(uv0.x, uv1.y)
                                );
                            }
                        }

                    } else {

                        // FRAME EDITABLE DRAW
                        for (int i = 0; i < joint_count; i++) {

                            // jidx size == joint_count by definition so... not doing an assertion here... yet...
                            int idx = jidx_draw_order[i];

                            auto& anchor_j = (*avatar_selected).default_frame.joints[idx];
                            auto& anim_j = animations[anim_selected].frames[selected_anim_frame].joints[idx];
                            auto& j_text = (*avatar_selected).default_texturing.joints[idx];
                            float direction_point_x = 0.0f;
                            float direction_point_y = 0.0f;

                            direction_point_x = RotNewPositionVec({anchor_j.origin.x + anim_j.origin.x, anchor_j.origin.y + anim_j.origin.y}, anchor_j.direction, anim_j.rotation).x;
                            direction_point_y = RotNewPositionVec({anchor_j.origin.x + anim_j.origin.x, anchor_j.origin.y + anim_j.origin.y}, anchor_j.direction, anim_j.rotation).y;

                            ImVec2 a = WorldToScreen(ImVec2(anchor_j.origin.x + anim_j.origin.x, anchor_j.origin.y + anim_j.origin.y));
                            ImVec2 b = WorldToScreen(ImVec2(direction_point_x, direction_point_y));

                            float dist = DistancePointToSegment(mpos, a, b);

                            bool hovered = dist < 8.0f;
                            bool selected = (idx == jointselected);

                            

                            ImU32 color;

                            if (selected) {
                                color = IM_COL32(0, 200, 255, 255);   // bright cyan selected
                            }
                            else if (hovered) {
                                color = IM_COL32(80, 170, 255, 255);  // blue hover
                            }
                            else {
                                color = IM_COL32(255, 255, 255, 120); // default
                            }

                            float thickness = selected ? 6.0f : (hovered ? 5.0f : 3.0f);

                            draw->AddLine(a, b, color, thickness);

                            // Get accurate texture info
                            const ResolvedJointTexture resolved_texture = j_text.resolve_texture(anim_j.anim_texture_idx);
                            Texture2D* actual_texture = resolved_texture.texture;
                            float actual_texture_cmin_x = resolved_texture.crop_min.x;
                            float actual_texture_cmin_y = resolved_texture.crop_min.y;
                            float actual_texture_cmax_x = resolved_texture.crop_max.x;
                            float actual_texture_cmax_y = resolved_texture.crop_max.y;
                            float actual_texture_scale_x = resolved_texture.scale.x;
                            float actual_texture_scale_y = resolved_texture.scale.y;
                            float actual_texture_offset_x = resolved_texture.offset.x;
                            float actual_texture_offset_y = resolved_texture.offset.y;
                            float actual_texture_rotation = resolved_texture.rotation;

                            if (actual_texture != nullptr) {

                                // Locations
                                ImVec2 center = WorldToScreen(ImVec2(anchor_j.origin.x + anim_j.origin.x, anchor_j.origin.y + anim_j.origin.y));
                                float width  = actual_texture->width  * actual_texture_scale_x * local_canvas_zoom;
                                float height = actual_texture->height * actual_texture_scale_y * local_canvas_zoom;

                                // Direction --> angle
                                Vec2 newDir = RotNewDirectionVec(anchor_j.direction, anim_j.rotation);
                                ImVec2 dir = { newDir.x, newDir.y };
                                float baseAngle = atan2f(dir.y, dir.x);
                                float angle = baseAngle + actual_texture_rotation;
                                float cosA = cosf(angle);
                                float sinA = sinf(angle);

                                // Offset (in world space --> scaled)
                                ImVec2 offset = {actual_texture_offset_x * local_canvas_zoom, actual_texture_offset_y * local_canvas_zoom};

                                // Define local quad (centered)
                                ImVec2 half = { width * 0.5f, height * 0.5f };
                                ImVec2 corners[4] = {
                                    {-half.x, -half.y},
                                    { half.x, -half.y},
                                    { half.x,  half.y},
                                    {-half.x,  half.y}
                                };

                                ImVec2 rotated[4];

                                for (int k = 0; k < 4; k++) {
                                    float x = corners[k].x + offset.x;
                                    float y = corners[k].y + offset.y;
                                    rotated[k].x = center.x + (x * cosA - y * sinA);
                                    rotated[k].y = center.y + (x * sinA + y * cosA);
                                }

                                ImVec2 uv0 = { actual_texture_cmin_x, actual_texture_cmin_y };
                                ImVec2 uv1 = { actual_texture_cmax_x, actual_texture_cmax_y };

                                draw->AddImageQuad(
                                    (ImTextureID)(intptr_t)actual_texture->id,
                                    rotated[0],
                                    rotated[1],
                                    rotated[2],
                                    rotated[3],
                                    ImVec2(uv0.x, uv0.y),
                                    ImVec2(uv1.x, uv0.y),
                                    ImVec2(uv1.x, uv1.y),
                                    ImVec2(uv0.x, uv1.y)
                                );
                            }

                            // CLICK SELECT only should make sense if we are on a keyframe
                            if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                                if (!sterilizeJointForPopup && !sterilizeJointForAnimPopup) {
                                    jointselected = idx;
                                }
                            }

                        }

                    }


                    // Clean up heap memory with jidx draw order
                    delete[] jidx_draw_order;

                }

                


                // @IMAWarriors TODO

                // JOINT MOVEMENT AND SELECTION
                
                // --> Some things haave to be edited in here, 
                // --> We always use default texturing for editing joints in drag,
                // --> but when dragging we want to edit joint data for whatever animation we are in,
                // --> editing the active frame and then saving to the animation data, saved as varaitions of default framing...
                //
                // ------
                // --> Default joint editing capability needs to be kept ONLY when editing default frame
                // --> Default joint skin ghosting should show very ghostly faint lines and sprites
                //     if onion mode is on with the Default joint data as well as
                // --> (#2 Priority, Low Priority, --> Add later) Custom # of frames backward from current frame in ghosting onion layer (more and more faint as we go back)

                bool doingJointMoveThisFrame = false;

                static bool point1 = false;
                static bool point2 = false;

                static ImVec2 orig_point;
                static ImVec2 orig_mouse;

                static float orig_rot;

                if (!editing_anim_w_interpolation) {
                    // Joint dragging logic
                    if (selected_anim_frame == -1) {

                        // Dragging script setup and draw dragging notation
                        if (jointselected != -1) {

                            ImVec2 mouse = ImGui::GetIO().MousePos;

                            // circle position (world or screen space)
                            ImVec2 c1 = WorldToScreen(ImVec2((*avatar_selected).default_frame.joints[jointselected].origin.x, (*avatar_selected).default_frame.joints[jointselected].origin.y));
                            ImVec2 c2 = WorldToScreen(ImVec2((*avatar_selected).default_frame.joints[jointselected].origin.x + (*avatar_selected).default_frame.joints[jointselected].direction.x, (*avatar_selected).default_frame.joints[jointselected].origin.y + (*avatar_selected).default_frame.joints[jointselected].direction.y));
                            float radius = 10.0f;

                            // distance check for hover
                            float dx1 = mouse.x - c1.x;
                            float dy1 = mouse.y - c1.y;
                            bool hov1 = (dx1*dx1 + dy1*dy1) <= (radius * radius);

                            // distance check for hover
                            float dx2 = mouse.x - c2.x;
                            float dy2 = mouse.y - c2.y;
                            bool hov2 = (dx2*dx2 + dy2*dy2) <= (radius * radius);

                            // choose color
                            ImU32 col1 = hov1
                                ? IM_COL32(255, 140, 0, 150)   // orange
                                : IM_COL32(0, 160, 255, 100);   // bright blue

                            if (!(point1 || point2)) {
                                if (hov1) {

                                    doingJointMoveThisFrame = true;

                                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                                        point1 = true;
                                        point2 = false;
                                        orig_point = ImVec2((*avatar_selected).default_frame.joints[jointselected].origin.x, (*avatar_selected).default_frame.joints[jointselected].origin.y);
                                        orig_mouse = ImGui::GetIO().MousePos;
                                    }
                                } else if (hov2) {

                                    doingJointMoveThisFrame = true;

                                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                                        point1 = false;
                                        point2 = true;
                                        orig_point = ImVec2((*avatar_selected).default_frame.joints[jointselected].direction.x, (*avatar_selected).default_frame.joints[jointselected].direction.y);
                                        orig_mouse = ImGui::GetIO().MousePos;
                                        
                                    }
                                    
                                } else {
                                    
                                }
                            }

                            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                            {
                                point1 = false;
                                point2 = false;
                            }

                            if (point1) {
                                ImVec2 delta = {ImGui::GetIO().MousePos.x - orig_mouse.x, ImGui::GetIO().MousePos.y - orig_mouse.y};

                                (*avatar_selected).default_frame.joints[jointselected].origin =
                                {
                                    orig_point.x + delta.x / local_canvas_zoom,
                                    orig_point.y + delta.y / local_canvas_zoom
                                };
                            }

                            if (point2) {
                                ImVec2 delta = {ImGui::GetIO().MousePos.x - orig_mouse.x, ImGui::GetIO().MousePos.y - orig_mouse.y};

                                (*avatar_selected).default_frame.joints[jointselected].direction =
                                {
                                    orig_point.x + delta.x / local_canvas_zoom,
                                    orig_point.y + delta.y / local_canvas_zoom
                                };
                            }
                                
                            // choose color
                            ImU32 col2 = hov2
                                ? IM_COL32(255, 255, 0, 150)   // orange
                                : IM_COL32(150, 180, 255, 100);   // bright blue

                            // draw filled circle
                            draw->AddCircleFilled(c1, radius, col1);
                            draw->AddCircleFilled(c2, radius, col2);

                            


                        }
                        
                    } else {

                        if (jointselected != -1) {

                            // If we are trying to drag-edit an animation frame
                            ImVec2 mouse = ImGui::GetIO().MousePos;

                            int idx = jointselected;

                            const auto& anchor_joint = (*avatar_selected).default_frame.joints[idx];

                            auto& anim_frame_joint = animations[anim_selected].frames[selected_anim_frame].joints[idx];

                            Vec2 new_direction_point = {0.0f, 0.0f};

                            new_direction_point.x = RotNewPositionVec({anchor_joint.origin.x + anim_frame_joint.origin.x, anchor_joint.origin.y + anim_frame_joint.origin.y}, anchor_joint.direction, anim_frame_joint.rotation).x;
                            new_direction_point.y = RotNewPositionVec({anchor_joint.origin.x + anim_frame_joint.origin.x, anchor_joint.origin.y + anim_frame_joint.origin.y}, anchor_joint.direction, anim_frame_joint.rotation).y;

                            // circle position (world or screen space)
                            ImVec2 c1 = WorldToScreen(ImVec2(anchor_joint.origin.x + anim_frame_joint.origin.x, anchor_joint.origin.y + anim_frame_joint.origin.y));
                            ImVec2 c2 = WorldToScreen(ImVec2(new_direction_point.x, new_direction_point.y));
                            
                            float radius = 10.0f;

                            // distance check for hover
                            float dx1 = mouse.x - c1.x;
                            float dy1 = mouse.y - c1.y;
                            bool hov1 = (dx1*dx1 + dy1*dy1) <= (radius * radius);

                            // distance check for hover
                            float dx2 = mouse.x - c2.x;
                            float dy2 = mouse.y - c2.y;
                            bool hov2 = (dx2*dx2 + dy2*dy2) <= (radius * radius);

                            // choose color
                            ImU32 col1 = hov1
                                ? IM_COL32(255, 140, 0, 150)   // orange
                                : IM_COL32(0, 160, 255, 100);   // bright blue

                            if (!(point1 || point2)) {
                                if (hov1) {

                                    // 1st Point: If hovering the first point...
                                    doingJointMoveThisFrame = true;

                                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                                        point1 = true;
                                        point2 = false;
                                        orig_point = ImVec2(anim_frame_joint.origin.x, anim_frame_joint.origin.y);
                                        orig_mouse = ImGui::GetIO().MousePos;
                                    }

                                } else if (hov2) {

                                    // 2nd Point: If hovering the second point...
                                    doingJointMoveThisFrame = true;

                                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                                        point1 = false;
                                        point2 = true;

                                        // Get direction of joint from default frame and apply rotation that has been done this animation frame
                                        Vec2 this_dir = RotNewDirectionVec(anchor_joint.direction, anim_frame_joint.rotation);

                                        orig_point = ImVec2(this_dir.x, this_dir.y);
                                        orig_rot = anim_frame_joint.rotation;
                                        orig_mouse = ImGui::GetIO().MousePos;
                                        
                                    }

                                } else {


                                    
                                }
                            }

                            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                                point1 = false;
                                point2 = false;
                            }

                            if (point1) {
                                ImVec2 delta = {ImGui::GetIO().MousePos.x - orig_mouse.x, ImGui::GetIO().MousePos.y - orig_mouse.y};
                                anim_frame_joint.origin = {
                                    orig_point.x + delta.x / local_canvas_zoom,
                                    orig_point.y + delta.y / local_canvas_zoom
                                };

                            }

                            if (point2) {
                                ImVec2 delta = {ImGui::GetIO().MousePos.x - orig_mouse.x, ImGui::GetIO().MousePos.y - orig_mouse.y};

                                float anchor_rotation = RotDegBetweenPoints(
                                    {0.0f, 0.0f},
                                    anchor_joint.direction
                                );

                                // Change in rotation based on change in Mouse XY and where the Anchor's point is
                                //  --> That degree change should tell you where to put the new direction end Vec2 point
                                //  --> Not changing magnitude of original direction vector, but just the rotation to where mouose goes

                                ImVec2 screen_orig = WorldToScreen(ImVec2(anchor_joint.origin.x + anim_frame_joint.origin.x, anchor_joint.origin.y + anim_frame_joint.origin.y));

                                float new_anim_rotation = RotDegBetweenPoints(
                                    {screen_orig.x, screen_orig.y},
                                    { ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y }
                                );

                                anim_frame_joint.rotation = new_anim_rotation - anchor_rotation;
                            }
                                
                            // choose color
                            ImU32 col2 = hov2
                                ? IM_COL32(255, 255, 0, 150)   // orange
                                : IM_COL32(150, 180, 255, 100);   // bright blue

                            // draw filled circle
                            draw->AddCircleFilled(c1, radius, col1);
                            draw->AddCircleFilled(c2, radius, col2);

                        }

                    }
                }

                // Camera dragging scripts
                if (ImGui::IsWindowHovered() && !point1 && !point2) {
                    local_canvas_zoom += ImGui::GetIO().MouseWheel * 1.0f;
                    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                        if (!draggingLastFrame) {

                            draggingLastFrame = true;
                            drag_start = {local_canvas_scroll.x, local_canvas_scroll.y};
                            mouse_start = ImGui::GetIO().MousePos;
                            
                        } else {
                            local_canvas_scroll = {
                                drag_start.x - (ImGui::GetIO().MousePos.x - mouse_start.x) / local_canvas_zoom,
                                drag_start.y - (ImGui::GetIO().MousePos.y - mouse_start.y) / local_canvas_zoom
                            };
                        }
                    }

                    if (local_canvas_zoom < 0.5f) {
                        local_canvas_zoom = 0.5f;
                    }
                }

                // Dragging pull reset scripts
                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    draggingLastFrame = false;
                }

                // Inspector info
                draw->AddText(
                    ImVec2(canvasPos.x + 5.0f, (canvasPos.y + canvasSize.y) - ImGui::GetFontSize() - 5.0f),
                    IM_COL32(255, 255, 255, 255),
                    std::string("(" + std::to_string(local_canvas_scroll.x) + ", " + std::to_string(local_canvas_scroll.y) + ")   " + std::to_string(local_canvas_zoom)).c_str() );


                // Label of what frame we are editing
                if (!editing_anim_w_interpolation) {
                    if (selected_anim_frame == -1) {
                        draw->AddText(
                            ImVec2(canvasPos.x + 5.0f, (canvasPos.y + 35.0f) - ImGui::GetFontSize() - 5.0f),
                            IM_COL32(20, 255, 70, 255),
                            std::string("Anchor Frame: ").c_str() );
                    } else {
                        draw->AddText(
                            ImVec2(canvasPos.x + 5.0f, (canvasPos.y + 35.0f) - ImGui::GetFontSize() - 5.0f),
                            IM_COL32(255, 255, 255, 255),
                            std::string("Frame #" + std::to_string(selected_anim_frame) + ": ").c_str() );
                    }
                } else {
                    // If we ARE editing with interpolation

                    bool scrubberOnKeyframe = false;
                    int tick_frame_acc = 0;
                    int keyframe_idx = 0;

                    for (const KeyAnimFrame& kframe : animations[anim_selected].frames) {
                        if (currentFrame < tick_frame_acc) {
                            break;
                        }
                        tick_frame_acc += kframe.time_to_next + 1;
                        keyframe_idx++;
                    }

                    tick_frame_acc = 0;

                    for (const KeyAnimFrame& kframe : animations[anim_selected].frames) {
                        if (currentFrame == tick_frame_acc) {
                            scrubberOnKeyframe = true;
                            break;
                        }
                        tick_frame_acc += kframe.time_to_next + 1;
                    }

                    draw->AddText(
                            ImVec2(canvasPos.x + 5.0f, (canvasPos.y + 20.0f) - ImGui::GetFontSize() - 2.0f),
                            IM_COL32(255, 20, 20, 255),
                            std::string("ANIMATION MODE: ").c_str() );

                    draw->AddText(
                            ImVec2(canvasPos.x + 115.0f, (canvasPos.y + 20.0f) - ImGui::GetFontSize() - 2.0f),
                            IM_COL32(255, 255, 255, 255),
                            std::string("Key Frame: " + std::to_string(keyframe_idx)).c_str() );
                    

                    if (scrubberOnKeyframe) {

                        draw->AddText(
                            ImVec2(canvasPos.x + 220.0f, (canvasPos.y + 20.0f) - ImGui::GetFontSize() - 2.0f),
                            IM_COL32(100, 255, 100, 255),
                            std::string("[Key Frame]").c_str() );


                        draw->AddText(
                            ImVec2(canvasPos.x + 300.0f, (canvasPos.y + 20.0f) - ImGui::GetFontSize() - 2.0f),
                            IM_COL32(100, 255, 100, 255),
                            std::string("Tick Frame: " + std::to_string(currentFrame)).c_str() );

                    } else {

                        draw->AddText(
                            ImVec2(canvasPos.x + 220.0f, (canvasPos.y + 20.0f) - ImGui::GetFontSize() - 2.0f),
                            IM_COL32(255, 255, 100, 255),
                            std::string("[Interpolation Frame]").c_str() );

                        draw->AddText(
                            ImVec2(canvasPos.x + 410.0f, (canvasPos.y + 20.0f) - ImGui::GetFontSize() - 2.0f),
                            IM_COL32(255, 255, 255, 255),
                            std::string("Tick Frame: " + std::to_string(currentFrame)).c_str() );
                    
                    }

                    
                    

                }

                // Draw Origin then Canvvas lines ***********
                ImVec2 origin = WorldToScreen(ImVec2(0.0f, 0.0f));

                draw->AddLine(
                    ImVec2(canvasPos.x, origin.y),
                    ImVec2(canvasPos.x + canvasSize.x, origin.y),
                    IM_COL32(255,255,255,255),
                    2.0f
                );

                draw->AddLine(
                    ImVec2(origin.x, canvasPos.y),
                    ImVec2(origin.x, canvasPos.y + canvasSize.y),
                    IM_COL32(255,255,255,255),
                    2.0f
                );

                float line_spacing = (local_canvas_zoom * gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS);
                int total_vert_lines = canvasSize.x / line_spacing;
                int total_horz_lines = canvasSize.y / line_spacing;

                float tile = gwconst::SCREEN_BASE_TILESIZE_GAMEPIXELS;

                // find visible world bounds
                float left   = local_canvas_scroll.x;
                float right  = local_canvas_scroll.x + canvasSize.x / local_canvas_zoom;
                float top    = local_canvas_scroll.y;
                float bottom = local_canvas_scroll.y + canvasSize.y / local_canvas_zoom;

                // snap start to grid
                int startX = (int)floor(left / tile);
                int endX   = (int)ceil(right / tile);

                int startY = (int)floor(top / tile);
                int endY   = (int)ceil(bottom / tile);

                // vertical lines
                for (int x = startX; x <= endX; x++) {
                    float worldX = x * tile;

                    ImVec2 a = WorldToScreen(ImVec2(worldX, top));
                    ImVec2 b = WorldToScreen(ImVec2(worldX, bottom));

                    draw->AddLine(a, b, IM_COL32(80,80,80,255), 1.0f);
                }

                // horizontal lines
                for (int y = startY; y <= endY; y++) {
                    float worldY = y * tile;

                    ImVec2 a = WorldToScreen(ImVec2(left, worldY));
                    ImVec2 b = WorldToScreen(ImVec2(right, worldY));

                    draw->AddLine(a, b, IM_COL32(80,80,80,255), 1.0f);
                }

                // ~~~~~~~~~~~~~~~~~~~~~~~~
                // Animation Control Bar Rendering?
                // *******

                /*
                

                */

                // Reserve layout space for the manually drawn canvas so following widgets
                // (like the preview controller) stay inside CenterView instead of being
                // pushed outside the editor row.






                ImGui::Dummy(canvasSize);

                struct TimelineFlag {
                    int frame;
                    ImU32 color;
                    int keyframe_index;
                };

                

                std::vector<TimelineFlag> flags;

                if (editing_anim_w_interpolation) {

                    int i = 0;
                    int scrub_fr = 0;

                    // Build timeline flags
                    for (auto& fr : animations[anim_selected].frames) {

                        flags.push_back({
                            scrub_fr,
                            IM_COL32(
                                ((255 + (i * 50)) % 255),
                                ((120 + (i * 50)) % 255),
                                ((180 + (i * 50)) % 255), 255), 
                            i});

                        scrub_fr += fr.time_to_next + 1;

                        i++;
                    }

                    // Sort timeline flags
                    std::sort(flags.begin(), flags.end(),[](const TimelineFlag& a, const TimelineFlag& b) {
                        return a.frame < b.frame;
                    });

                    // ======== LAMBDA CONTROLLERS =================>

                    auto FindPreviousKeyframe = [&](int frame) -> int {
                        int prev = 0;
                        for (const TimelineFlag& fl : flags) {
                            if (fl.frame < frame) {
                                prev = fl.frame;
                            } else {
                                break;
                            }
                        }
                        return prev;
                    };
                    
                    auto FindNextKeyframe = [&](int frame) -> int {
                        for (const TimelineFlag& fl : flags) {
                            if (fl.frame > frame) {
                                return fl.frame;
                            }
                        }
                        return animation_tick_frame_length - 1;
                    };

                    // ===========================================>

                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.04f,0.04f,0.04f,0.94f));
                
                    ImGui::BeginChild(
                        "PreviewControllerRegion",
                        ImVec2(0, controller_bar_height),
                        true
                    );

                    // Preview Controller

                    ImGui::Text("Animation Controller");

                    ImGui::Separator();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f,0.15f,0.15f,0.8f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f,0.2f,0.2f,0.8f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f,0.1f,0.1f,0.8f));


                    if (ImGui::Button(" |< ", ImVec2(40, 30))) {

                        // First find frame tick index of last key frame

                        play_preview_animation = false;
                        currentFrame = FindPreviousKeyframe(currentFrame);

                    }

                    ImGui::SameLine();

                    if (ImGui::Button(" << ", ImVec2(40, 30))) {

                        play_preview_animation = false;
                        currentFrame = std::max(0, currentFrame - 1);

                    }

                    ImGui::SameLine();

                    if (play_preview_animation) {
                        if (ImGui::Button(" || ", ImVec2(40, 30))) {
                            play_preview_animation = false;
                        }
                    } else {
                        if (ImGui::Button(" -> ", ImVec2(40, 30))) {
                            play_preview_animation = true;
                        }
                    }

                    ImGui::SameLine();

                    ImGui::BeginDisabled(!play_preview_animation);

                    if (ImGui::Button(" XX ", ImVec2(40, 30))) {
                        play_preview_animation = false;
                    }

                    ImGui::EndDisabled();

                    ImGui::SameLine();

                    if (ImGui::Button(" >> ", ImVec2(40, 30))) {

                        play_preview_animation = false;

                        currentFrame = std::min(
                            animation_tick_frame_length - 1,
                            currentFrame + 1
                        );

                    }

                    ImGui::SameLine();

                    if (ImGui::Button(" >| ", ImVec2(40, 30))) {

                        play_preview_animation = false;
                        currentFrame = FindNextKeyframe(currentFrame);
                        
                    }

                    ImGui::SameLine();

                    // @@@

                    ImGui::SetNextItemWidth(85.0f);

                    ImGui::DragInt("ms per tick", &animations[anim_selected].ms_per_tick_frame, 1.0f, 1, 128);



                    ImGui::PopStyleColor(3);



                    ImGui::EndChild();
                
                
                    ImGui::PopStyleColor();
            
                }

                

                ImGui::EndChild();


                



                // End of Canvas

                // *****************************************************





                // JOINT INSPECTOR PALLET
                

                if (jointselected != -1 || selected_anim_frame != -1) {

                    ImGui::SameLine();

                    ImGui::BeginChild(
                            "JointEditor",
                            ImVec2(ImGui::GetContentRegionAvail().x, 0),
                            true
                        );


                    if (ImGui::BeginTabBar("animtypetabs")) {

                        if (jointselected != -1 || selected_anim_frame == -1 || (selected_anim_frame != -1 && ( (*avatar_selected).default_frame.joints.size() > 0 ) ) ) {

                            if (ImGui::BeginTabItem("Frame")) {

                                ImGui::Separator();
                               
                                if (selected_anim_frame != -1) {

                                    ImGui::Text(std::string("Joint Inspector").c_str());
                                    ImGui::SameLine();
                                    ImGui::Text(std::string("|").c_str());
                                    ImGui::SameLine();

                                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.3f, 1.0f));
                                    ImGui::Text(std::string("Frame #" + std::to_string(selected_anim_frame)).c_str());
                                    ImGui::PopStyleColor();
                                
                                } else {

                                    ImGui::Text(std::string("Joint Inspector").c_str());
                                    ImGui::SameLine();
                                    ImGui::Text(std::string("|").c_str());
                                    ImGui::SameLine();

                                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 8.0f, 0.1f, 1.0f));
                                    ImGui::Text(std::string("Anchor Frame").c_str());
                                    ImGui::PopStyleColor();
                                }

                                ImGui::Separator(); 

                                editing_anim_w_interpolation = false;
                                play_preview_animation = false;

                                /* If no joint is selected when we pop into Frame tab,
                                    set to a default joint, joint 0 : DEACTIVATED
                                ==========================================================

                                if (selected_anim_frame != -1 && jointselected == -1) {
                                    if ((*avatar_selected).default_frame.joints.size() > 0)
                                        jointselected = 0;
                                }
                                */

                                if (jointselected!=-1) {
                                    // ANCHOR FRAME EDITOR: This if branch handles if a joint is selected
                                    // for the joint inspector but we are on the default frame, so original
                                    // character data SHOULD be altered; this should be VICE VESA for
                                    // the INDIVIDUAL FRAME EDITOR
                                    if (selected_anim_frame == -1) {

                                        auto& joint = (*avatar_selected).default_frame.joints[jointselected];
                                        auto& joint_text = (*avatar_selected).default_texturing.joints[jointselected];

                                        // ONLY update buffer when selection changes
                                        if (lastJointSelected != jointselected) {
                                            lastJointSelected = jointselected;

                                            strncpy(jointnamebuffer, joint_text.name.c_str(), sizeof(jointnamebuffer));
                                            jointnamebuffer[sizeof(jointnamebuffer) - 1] = '\0'; // safety null terminator
                                        }

                                        ImGui::TextUnformatted(joint_text.name.c_str());

                                        ImGui::Separator();

                                        if (ImGui::BeginTable("JointInspector", 2, ImGuiTableFlags_SizingStretchProp)) {
                                            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                                            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                                            std::function<void(const char*, std::function<void()>)> Row =
                                            [&](const char* label, std::function<void()> func)
                                            {
                                                ImGui::TableNextRow();
                                                ImGui::TableSetColumnIndex(0);
                                                ImGui::TextUnformatted(label);

                                                ImGui::TableSetColumnIndex(1);
                                                func();
                                            };

                                            Row("Name", [&]() {
                                                ImGui::InputText("##name", jointnamebuffer, IM_ARRAYSIZE(jointnamebuffer));
                                                joint_text.name = jointnamebuffer;
                                            });

                                            Row("oX", [&]() { ImGui::DragFloat("##ox", &joint.origin.x, 0.1f); });
                                            Row("oY", [&]() { ImGui::DragFloat("##oy", &joint.origin.y, 0.1f); });

                                            Row("dX", [&]() { ImGui::DragFloat("##dx", &joint.direction.x, 0.1f); });
                                            Row("dY", [&]() { ImGui::DragFloat("##dy", &joint.direction.y, 0.1f); });

                                            if (joint_text.joint_has_texture()) {
                                                ImGui::Separator();
                                                ImGui::Text("Image Settings");

                                                Row("Offset X", [&]() { ImGui::DragFloat("##offx", &joint_text.offset.x, 0.1f); });
                                                Row("Offset Y", [&]() { ImGui::DragFloat("##offy", &joint_text.offset.y, 0.1f); });

                                                Row("Scale X", [&]() { ImGui::DragFloat("##scalex", &joint_text.scale.x, 0.01f); });
                                                Row("Scale Y", [&]() { ImGui::DragFloat("##scaley", &joint_text.scale.y, 0.01f); });

                                                Row("Crop Min X", [&]() { ImGui::DragFloat("##cminx", &joint_text.crop_min.x, 0.1f); });
                                                Row("Crop Min Y", [&]() { ImGui::DragFloat("##cminy", &joint_text.crop_min.y, 0.1f); });

                                                Row("Crop Max X", [&]() { ImGui::DragFloat("##cmaxx", &joint_text.crop_max.x, 0.1f); });
                                                Row("Crop Max Y", [&]() { ImGui::DragFloat("##cmaxy", &joint_text.crop_max.y, 0.1f); });

                                                float deg = joint_text.rotation * 180.0f / 3.14159265f;
                                                Row("Rotation", [&]() { if (ImGui::DragFloat("##rot", &deg, 1.0f)) {joint_text.rotation = deg * 3.14159265f / 180.0f;} });
                                            }

                                            ImGui::EndTable();
                                        }

                                        

                                        
                                        ImGui::Separator();

                                        // -------------------------------------------------------------------------- //
                                        // -------------- Joint animation library textures controller --------------- //
                                        //   - Save variables to manage popups
                                        //   - Maintain texture adding buttons
                                        //  
                                        //
                                        //

                                        // LoadChangeRootTexturePopup Popup Manager
                                        bool LoadChangeRootTexturePopup = false;
                                        bool LoadAnimJointTextureLibraryPopup = false;
                                        
                                        // Joint Root Texture Selection
                                        ImGui::Text("Joint Root Texture (Idx: -1):");

                                        ImGui::BeginChild("RootTexture", ImVec2(0, 60), true);

                                        if (ImGui::Button("Change Root Texture")) {
                                            // Open change root texture pop-up
                                            LoadChangeRootTexturePopup = true;
                                        }

                                        ImGui::Separator();

                                        const std::string& fullPath = joint_text.texturePath;
                                        std::string displayName = std::filesystem::path(fullPath).filename().string();

                                        ImGui::Text(std::string("Idx -1: "+ displayName).c_str());


                                        ImGui::EndChild();


                                        // ---------- Open Popup: Change Root Texture -----------

                                        if (LoadChangeRootTexturePopup) {
                                            ImGui::OpenPopup("Change Joint Root Texture");
                                        }

                                        ImVec2 lcrtModalSize = {
                                            std::max(620.0f, GetScreenWidth() * 0.72f),
                                            std::max(420.0f, GetScreenHeight() * 0.76f)
                                        };

                                        // Start the Save Avatar .avr file popup
                                        ImGui::GetStyle().WindowPadding = ImVec2(8,8);
                                        ImGui::SetNextWindowSize(lcrtModalSize, ImGuiCond_Appearing);

                                        if (ImGui::BeginPopupModal("Change Joint Root Texture", NULL, ImGuiWindowFlags_NoCollapse)) {

                                            sterilizeJointForPopup = true;

                                            // %%%
                                            ImGui::Text("Choose a valid texture file to replace the Root Texture of the ");
                                            ImGui::SameLine();
                                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                                            ImGui::Text(joint_text.name.c_str());
                                            ImGui::PopStyleColor();
                                            ImGui::SameLine();
                                            ImGui::Text(" joint.");

                                            ImGui::Separator();

                                            // --------------------->

                                            // --- Load in potential joint image files into this vector ---
                                            static int selectedFileIndex = -1;
                                            static Texture2D* previewTexture = nullptr;
                                            static std::string previewTexturePath = "";

                                            static std::vector<std::string> characterFiles;
                                            static bool filesLoaded = false;
                                            
                                            if (!filesLoaded) {
                                                selectedFileIndex = -1;
                                                characterFiles.clear();
                                                std::string root = "assets/sprites/characters";
                                                for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
                                                    if (!entry.is_directory()) {
                                                        std::string path = entry.path().string();
                                                        if (entry.path().extension() == ".png") {
                                                            characterFiles.push_back(path);
                                                        }
                                                    }
                                                }
                                                filesLoaded = true;
                                            }
                                            // -----------------------------------------------------------

                                            float winsize = ImGui::GetContentRegionAvail().x * 0.98f;
                                            float width_filepane = winsize * 0.55;
                                            float width_currpane = winsize * 0.225;
                                            float width_previewpane = winsize * 0.225;

                                            // ***************** CHILD: ROOT CHARACTER FILES *************************
                                            ImGui::BeginChild("RootCharacterFiles", ImVec2(width_filepane, 500), true);

                                            
                                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.7f, 1.0f, 1.0f));
                                            ImGui::Text("Joint Texture Filenames:");
                                            ImGui::PopStyleColor();

                                            ImGui::Separator();

                                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.55f, 0.8f, 1.0f));
                                            ImGui::Text("{ assets / sprites / characters / ... }");
                                            ImGui::PopStyleColor();
                                            
                                            ImGui::Separator();

                                            for (int i = 0; i < characterFiles.size(); i++) {
                                                const std::string& fullPath = characterFiles[i];

                                                // Optional: show only filename instead of full path
                                                std::string displayName = std::filesystem::path(fullPath).filename().string();

                                                if (ImGui::Selectable(displayName.c_str(), selectedFileIndex == i)) {

                                                    if (selectedFileIndex != i) {
                                                        selectedFileIndex = i;
                                                    } else {
                                                        selectedFileIndex = -1;
                                                    }

                                                    if (selectedFileIndex == -1) {
                                                        previewTexturePath = "";
                                                        previewTexture = nullptr;
                                                    } else {
                                                        previewTexturePath = fullPath;
                                                        previewTexture = &assets.LoadTextureAsset(previewTexturePath);
                                                    }

                                                }
                                            }
                                            ImGui::EndChild();

                                            // ************************************************************************


                                            // ********************** CHILD: PREVIEW PANE ******************************

                                            ImGui::SameLine();
                                            ImGui::BeginChild("JFilePreviewPane", ImVec2(width_previewpane, 500), true);
                                            
                                            ImGui::Text("Preview ");
                                            ImGui::SameLine();
                                            
                                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                                            ImGui::Text(joint_text.name.c_str());
                                            ImGui::PopStyleColor();

                                            ImGui::SameLine();
                                            ImGui::Text(" Texture:");

                                            ImGui::Separator();

                                            if (selectedFileIndex == -1) { 
                                                previewTexture = nullptr;
                                                previewTexturePath = "";
                                            } 

                                            if (selectedFileIndex == -1 || previewTexture == nullptr || previewTexturePath == "") {

                                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                                                ImGui::Text("No texture selected.");
                                                ImGui::PopStyleColor();

                                            } else {

                                                float placewidth = width_previewpane / 1.4f;
                                                float imgscale = 1.0f; // placewidth / (*previewTexture).width;

                                                Vec2 imgsize = {
                                                    (*previewTexture).width * imgscale,
                                                    (*previewTexture).height * imgscale
                                                };
                                                
                                                float availWidth = ImGui::GetContentRegionAvail().x;
                                                float offsetX = (availWidth - imgsize.x) * 0.5f;

                                                ImGui::Text("");
                                                if (offsetX > 0.0f)
                                                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

                                                
                                                ImGui::Image((ImTextureID)(*previewTexture).id, ImVec2(imgsize.x, imgsize.y));
                                                ImGui::Text("");

                                            }

                                            ImGui::Separator();

                                            ImGui::EndChild();



                                            // ********************** CHILD: CURRENT PANE ******************************

                                            ImGui::SameLine();
                                            ImGui::BeginChild("CFilePreviewPane", ImVec2(width_currpane, 500), true);

                                            ImGui::Text("Current ");
                                            ImGui::SameLine();
                                            
                                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                                            ImGui::Text(joint_text.name.c_str());
                                            ImGui::PopStyleColor();

                                            ImGui::SameLine();
                                            ImGui::Text(" Texture:");

                                            ImGui::Separator();

                                            // Draw image
                                            static Texture2D* currentLoadedJTexture = nullptr;
                                            static std::string currentLoadedPath = "";
                                            
                                            if (currentLoadedPath != joint_text.texturePath) {
                                                currentLoadedJTexture = &assets.LoadTextureAsset(joint_text.texturePath);
                                                currentLoadedPath = joint_text.texturePath;
                                            }

                                            if (currentLoadedJTexture != nullptr && currentLoadedPath != "" && currentLoadedPath != "NONE") {

                                                float cplacewidth = width_currpane / 1.4f;
                                                float cimgscale = 1.0f; // cplacewidth / (*currentLoadedJTexture).width;

                                                Vec2 imgsize = {
                                                    (*currentLoadedJTexture).width * cimgscale,
                                                    (*currentLoadedJTexture).height * cimgscale
                                                };

                                                float availWidth = ImGui::GetContentRegionAvail().x;
                                                float offsetX = (availWidth - imgsize.x) * 0.5f;

                                                ImGui::Text("");

                                                if (offsetX > 0.0f)
                                                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

                                                ImGui::Image((ImTextureID)(*currentLoadedJTexture).id, ImVec2(imgsize.x, imgsize.y));
                                                ImGui::Text("");

                                            } else {
                                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                                                ImGui::Text("Joint has no applied texture");
                                                ImGui::PopStyleColor();
                                            }

                                            ImGui::Separator();

                                            ImGui::EndChild();
                                            
                                            // *************************************************************************

                                            

                                            ImGui::BeginDisabled((selectedFileIndex == -1));

                                            if (ImGui::Button("Load Texture", ImVec2(110, 25))) {

                                                if (selectedFileIndex != -1) {
                                                    joint_text.load_texture_from_path(assets, characterFiles[selectedFileIndex]);
                                                }

                                                // Load Texture
                                                filesLoaded = false;
                                                ImGui::CloseCurrentPopup();
                                            }

                                            ImGui::EndDisabled();

                                            ImGui::SameLine();

                                            if (ImGui::Button("Cancel", ImVec2(70, 25))) {
                                                // Cancel
                                                filesLoaded = false;
                                                ImGui::CloseCurrentPopup();
                                            }

                                            ImGui::EndPopup();
                                        } else {

                                            // As long as the other joint texture editing Popup for
                                            // multiple animation textures isn't open too, not only this popup, 
                                            // then yeah you can allow us to move joints again it

                                            sterilizeJointForPopup = false;

                                        }

                                        // -------------------------------------------------------------




                                        // Animations Texture Loading Child
                                        static int selected_anim_texture_idx = -1;

                                        ImGui::Separator();
                                        ImGui::Text("Avatar Joint Anim Textures:");

                                        ImGui::BeginChild("AnimTextures", ImVec2(0, 90), true);

                                        auto& joint_texture = (*avatar_selected).default_texturing.joints[jointselected];

                                        if (ImGui::Button("New")) {
                                            // Open new anim texture pop-up
                                            joint_texture.push_back_new_anim_texture(assets, joint_texture.texturePath);
                                        }

                                        ImGui::SameLine();

                                        ImGui::BeginDisabled((selected_anim_texture_idx == -1));

                                        if (ImGui::Button("Delete")) {
                                            if (selected_anim_texture_idx >= 0 && selected_anim_texture_idx < joint_texture.animation_texture_library.size()) {
                                                joint_texture.animation_texture_library.erase(joint_texture.animation_texture_library.begin() + selected_anim_texture_idx);
                                            }

                                            if (selected_anim_texture_idx >= 0 && selected_anim_texture_idx < joint_texture.animation_texture_library.size()) {
                                                selected_anim_texture_idx = -1;
                                            }
                                        }

                                        ImGui::SameLine();

                                        if (ImGui::Button("Load")) {
                                            // Open popup for loading texture
                                            LoadAnimJointTextureLibraryPopup = true;

                                        }

                                        ImGui::SameLine();

                                        if (ImGui::Button("^^")) {
                                            if (1 <= selected_anim_texture_idx && selected_anim_texture_idx < joint_texture.animation_texture_library.size()) {
                                                
                                                // --
                                                std::swap(
                                                    joint_texture.animation_texture_library[selected_anim_texture_idx],
                                                    joint_texture.animation_texture_library[selected_anim_texture_idx-1]
                                                );

                                                selected_anim_texture_idx--;

                                            }
                                        }

                                        ImGui::SameLine();

                                        if (ImGui::Button("vv")) {
                                            if (0 <= selected_anim_texture_idx && selected_anim_texture_idx < joint_texture.animation_texture_library.size()-1) {
                                                
                                                // --
                                                std::swap(
                                                    joint_texture.animation_texture_library[selected_anim_texture_idx],
                                                    joint_texture.animation_texture_library[selected_anim_texture_idx+1]
                                                );

                                                selected_anim_texture_idx++;

                                            }
                                            
                                        }

                                        ImGui::EndDisabled();


                                        ImGui::Separator();

                                        // List
                                        for (int i = 0; i < joint_texture.animation_texture_library.size(); i++) {

                                            const std::string& fullPath = joint_texture.animation_texture_library[i].texture_path;
                                            std::string displayName = "Idx " + std::to_string(i) + ": " + std::filesystem::path(fullPath).filename().string();

                                            if (ImGui::Selectable(displayName.c_str(), selected_anim_texture_idx == i)) {
                                                if (selected_anim_texture_idx == i) {
                                                    selected_anim_texture_idx = -1;
                                                } else {
                                                    selected_anim_texture_idx = i;
                                                }
                                            }
                                        }
                                        ImGui::EndChild();



                                        // ------------------------------------------------------------------------- //
                                        // ------- Handle Popup: Anim Joint Texture Library Loader ----------------- //
                                        // ------------------------------------------------------------------------- //

                                        if (LoadAnimJointTextureLibraryPopup) {
                                            ImGui::OpenPopup("Load Joint Animation Library Textures");
                                        }

                                        ImVec2 lajtlpModalSize = {
                                            std::max(620.0f, GetScreenWidth() * 0.72f),
                                            std::max(420.0f, GetScreenHeight() * 0.76f)
                                        };

                                        ImGui::GetStyle().WindowPadding = ImVec2(8,8);
                                        ImGui::SetNextWindowSize(lajtlpModalSize, ImGuiCond_Appearing);

                                        if (ImGui::BeginPopupModal("Load Joint Animation Library Textures", NULL, ImGuiWindowFlags_NoCollapse)) {

                                            sterilizeJointForAnimPopup = true;

                                            // %%%
                                            ImGui::Text("Choose a texture file to load into index ");
                                            ImGui::SameLine();
                                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                                            ImGui::Text(std::to_string(selected_anim_texture_idx).c_str());
                                            ImGui::PopStyleColor();
                                            ImGui::SameLine();
                                            ImGui::Text(" of the ");
                                            ImGui::SameLine();
                                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                                            ImGui::Text(joint_text.name.c_str());
                                            ImGui::PopStyleColor();
                                            ImGui::SameLine();
                                            ImGui::Text(" joint animation library");

                                            ImGui::Separator();

                                            // --------------------->

                                            // --- Load in potential joint image files into this vector ---
                                            static int selectedFileIndex = -1;
                                            static Texture2D* previewTexture = nullptr;
                                            static std::string previewTexturePath = "";

                                            static std::vector<std::string> characterFiles;
                                            static bool filesLoaded = false;
                                            
                                            if (!filesLoaded) {
                                                selectedFileIndex = -1;
                                                characterFiles.clear();
                                                std::string root = "assets/sprites/characters";
                                                for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
                                                    if (!entry.is_directory()) {
                                                        std::string path = entry.path().string();
                                                        if (entry.path().extension() == ".png") {
                                                            characterFiles.push_back(path);
                                                        }
                                                    }
                                                }
                                                filesLoaded = true;
                                            }
                                            // -----------------------------------------------------------

                                            float winsize = ImGui::GetContentRegionAvail().x * 0.98f;
                                            float width_filepane = winsize * 0.45f;
                                            float width_libraryjoints = winsize * 0.275f;
                                            float width_previewpane = winsize * 0.275f;

                                            // ***************** CHILD: Root Char Files *************************
                                            ImGui::BeginChild("RootCharacterFiles", ImVec2(width_filepane, 500), true);

                                            
                                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.7f, 1.0f, 1.0f));
                                            ImGui::Text("Joint Texture Filenames:");
                                            ImGui::PopStyleColor();

                                            ImGui::Separator();

                                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.55f, 0.8f, 1.0f));
                                            ImGui::Text("{ assets / sprites / characters / ... }");
                                            ImGui::PopStyleColor();
                                            
                                            ImGui::Separator();

                                            for (int i = 0; i < characterFiles.size(); i++) {
                                                const std::string& fullPath = characterFiles[i];

                                                // Optional: show only filename instead of full path
                                                std::string displayName = std::filesystem::path(fullPath).filename().string();

                                                if (ImGui::Selectable(displayName.c_str(), selectedFileIndex == i)) {

                                                    if (selectedFileIndex != i) {
                                                        selectedFileIndex = i;
                                                    } else {
                                                        selectedFileIndex = -1;
                                                    }

                                                    if (selectedFileIndex == -1) {
                                                        previewTexturePath = "";
                                                        previewTexture = nullptr;
                                                    } else {
                                                        previewTexturePath = fullPath;
                                                        previewTexture = &assets.LoadTextureAsset(previewTexturePath);
                                                    }

                                                }
                                            }
                                            ImGui::EndChild();

                                            // ************************************************************************



                                            // ********************** CHILD: PREVIEW PANE ******************************

                                            ImGui::SameLine();
                                            ImGui::BeginChild("JFilePreviewPane", ImVec2(width_previewpane, 500), true);
                                            
                                            ImGui::Text("Preview Texture to Load:");

                                            ImGui::Separator();

                                            if (selectedFileIndex == -1) { 
                                                previewTexture = nullptr;
                                                previewTexturePath = "";
                                            } 

                                            if (selectedFileIndex == -1 || previewTexture == nullptr || previewTexturePath == "") {

                                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                                                ImGui::Text("No texture selected.");
                                                ImGui::PopStyleColor();

                                            } else {

                                                float placewidth = width_previewpane / 1.4f;
                                                float imgscale = 1.0f; // placewidth / (*previewTexture).width;

                                                Vec2 imgsize = {
                                                    (*previewTexture).width * imgscale,
                                                    (*previewTexture).height * imgscale
                                                };
                                                
                                                float availWidth = ImGui::GetContentRegionAvail().x;
                                                float offsetX = (availWidth - imgsize.x) * 0.5f;

                                                ImGui::Text("");
                                                if (offsetX > 0.0f)
                                                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

                                                
                                                ImGui::Image((ImTextureID)(*previewTexture).id, ImVec2(imgsize.x, imgsize.y));
                                                ImGui::Text("");

                                            }

                                            ImGui::Separator();

                                            ImGui::EndChild();
                                            // ------------------------------------------------------




                                            // ***************** CHILD: CURRENT TEXTURE PANE ******************************

                                            ImGui::SameLine();
                                            ImGui::BeginChild("JFileCurrentPane", ImVec2(width_previewpane, 500), true);
                                            
                                            ImGui::Text("Current Texture:");

                                            ImGui::Separator();

                                            // ++++++++++ TODO: Draw the Current Joint Texture Preview +++++++++++


                                            static Texture2D* currentLoadedJTexture = nullptr;
                                            static std::string currentLoadedPath = "";

                                            assert(selected_anim_texture_idx != -1);
                                            
                                            if (currentLoadedPath != joint_text.animation_texture_library[selected_anim_texture_idx].texture_path) {
                                                currentLoadedJTexture = &assets.LoadTextureAsset(joint_text.animation_texture_library[selected_anim_texture_idx].texture_path);
                                                currentLoadedPath = joint_text.animation_texture_library[selected_anim_texture_idx].texture_path;
                                            }

                                            if (currentLoadedJTexture != nullptr && currentLoadedPath != "" && currentLoadedPath != "NONE") {
                                                    

                                                float placewidth = width_previewpane / 1.4f;
                                                float imgscale = 1.0f; // placewidth / (*currentLoadedJTexture).width;

                                                Vec2 imgsize = {
                                                    (*currentLoadedJTexture).width * imgscale,
                                                    (*currentLoadedJTexture).height * imgscale
                                                };
                                                
                                                float availWidth = ImGui::GetContentRegionAvail().x;
                                                float offsetX = (availWidth - imgsize.x) * 0.5f;

                                                ImGui::Text("");
                                                if (offsetX > 0.0f)
                                                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

                                                
                                                ImGui::Image((ImTextureID)(*currentLoadedJTexture).id, ImVec2(imgsize.x, imgsize.y));
                                                ImGui::Text("");

                                            }


                                            // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

                                            ImGui::Separator();

                                            ImGui::EndChild();


                                            

                                            // Buttons for controlling

                                            ImGui::BeginDisabled((selectedFileIndex == -1));

                                            if (ImGui::Button("Load Texture", ImVec2(110, 25))) {

                                                if (selectedFileIndex != -1) {
                                                    joint_text.change_anim_texture(assets, selected_anim_texture_idx, characterFiles[selectedFileIndex]);
                                                }

                                                // Load Texture
                                                filesLoaded = false;
                                                ImGui::CloseCurrentPopup();
                                            }

                                            ImGui::EndDisabled();

                                            ImGui::SameLine();

                                            if (ImGui::Button("Cancel", ImVec2(70, 25))) {
                                                // Cancel
                                                filesLoaded = false;
                                                ImGui::CloseCurrentPopup();
                                            }

                                            ImGui::EndPopup();
                                        } else {

                                            // As long as the other joint texture editing Popup for
                                            // multiple animation textures isn't open too, not only this popup, 
                                            // then yeah you can allow us to move joints again it

                                            sterilizeJointForAnimPopup = false;

                                        }

                                        // -------------------------------------------------------------

                                        

                                        /* 
                                        
                                        // "Select a Texture"

                                        for (int i = 0; i < characterFiles.size(); i++) {
                                            const std::string& fullPath = characterFiles[i];

                                            // Optional: show only filename instead of full path
                                            std::string displayName = std::filesystem::path(fullPath).filename().string();

                                            if (ImGui::Selectable(displayName.c_str(), selectedFileIndex == i)) {
                                                selectedFileIndex = i;
                                            }
                                        }

                                        if (ImGui::Button("Set Image")) {
                                            
                                        }
                                        */

                                    } else {

                                        // INDIVIDUAL FRAME EDITOR: 
                                        // if (selected_anim_frame == 0, 1, 2, ... etc. ) {
                                        auto& joint = animations[anim_selected].frames[selected_anim_frame].joints[jointselected];
                                        auto& joint_text = (*avatar_selected).default_texturing.joints[jointselected];

                                        // ONLY update buffer when selection changes
                                        if (lastJointSelected != jointselected) {
                                            lastJointSelected = jointselected;

                                            strncpy(jointnamebuffer, joint_text.name.c_str(), sizeof(jointnamebuffer));
                                            jointnamebuffer[sizeof(jointnamebuffer) - 1] = '\0'; // safety null terminator
                                        }

                                        ImGui::TextUnformatted(joint_text.name.c_str());
                                        ImGui::Separator();

                                        // TODO
                                        ImGui::Text("Animation Frame Editor:");
                                        ImGui::Separator();

                                        ImGui::Text(std::string(" >> Selected Frame: " + std::to_string(selected_anim_frame)).c_str());

                                        if (ImGui::BeginTable("JointInspector", 2, ImGuiTableFlags_SizingStretchProp)) {

                                            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                                            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                                            std::function<void(const char*, std::function<void()>)> Row =
                                            [&](const char* label, std::function<void()> func)
                                            {
                                                ImGui::TableNextRow();
                                                ImGui::TableSetColumnIndex(0);
                                                ImGui::TextUnformatted(label);

                                                ImGui::TableSetColumnIndex(1);
                                                func();
                                            };

                                            Row("Name", [&]() {
                                                ImGui::Text(jointnamebuffer);
                                            });

                                            Row(">> X", [&]() { ImGui::DragFloat("##ox", &joint.origin.x, 0.1f); });
                                            Row(">> Y", [&]() { ImGui::DragFloat("##oy", &joint.origin.y, 0.1f); });

                                            Row(">> Rot.", [&]() { ImGui::DragFloat("##dr", &joint.rotation, 0.1f); });
                                            Row(">> Shortest Angle Rot.", [&]() { ImGui::Checkbox("", &joint.normal_rotation); });

                                            // Row("Draw Order: ", [&]() { ImGui::Text(std::string("Draw Order: " + std::to_string(joint.draw_order)).c_str()); });

                                            ImGui::EndTable();

                                            // Anim Frame Texture Editor
                                            // TODO HERE@@@

                                        }


                                        ImGui::Separator();
                                        ImGui::Text("Frame Texture Editor:");
                                        ImGui::Separator();

                                        ImGui::Text(std::string(joint_text.name + " Texture Library Index:").c_str());

                                        int& frame_selected_anim_texture_idx = animations[anim_selected].frames[selected_anim_frame].joints[jointselected].anim_texture_idx;

                                        std::string preview_anim_string;

                                        if (joint_text.get_anim_texture_or_null(frame_selected_anim_texture_idx) == nullptr) {
                                            const std::string& fullPath = joint_text.texturePath;
                                            std::string display_option = std::filesystem::path(fullPath).filename().string();
                                            preview_anim_string = "Idx " + std::to_string(frame_selected_anim_texture_idx) + ": " + display_option;
                                        } else {
                                            const std::string& fullPath = joint_text.animation_texture_library[frame_selected_anim_texture_idx].texture_path;
                                            std::string display_option = std::filesystem::path(fullPath).filename().string();
                                            preview_anim_string = "Idx " + std::to_string(frame_selected_anim_texture_idx) + ": " + display_option;
                                        }

                                        if (ImGui::BeginCombo("##animDD", preview_anim_string.c_str())) {

                                            for (int i = -1; i < (int)joint_text.animation_texture_library.size(); i++) {

                                                std::string anim_option;

                                                if (i == -1) {
                                                    const std::string& fullPath = joint_text.texturePath;
                                                    std::string display_option = std::filesystem::path(fullPath).filename().string();
                                                    anim_option = "Idx " + std::to_string(i) + ": " + display_option;
                                                } else {
                                                    const std::string& fullPath = joint_text.animation_texture_library[i].texture_path;
                                                    std::string display_option = std::filesystem::path(fullPath).filename().string();
                                                    anim_option = "Idx " + std::to_string(i) + ": " + display_option;
                                                }

                                                bool isSelected = (frame_selected_anim_texture_idx == i);

                                                if (ImGui::Selectable( anim_option.c_str(), isSelected)) {
                                                    frame_selected_anim_texture_idx = i;
                                                }

                                                if (isSelected)
                                                    ImGui::SetItemDefaultFocus();
                                            }

                                            ImGui::EndCombo();
                                        }

                                        ImGui::Separator();

                                        static bool extra_texturing_options = false;

                                        ImGui::Checkbox("Advanced View", &extra_texturing_options);

                                        ImGui::Separator();

                                        if (extra_texturing_options) {
                                            
                                            const std::string& tsettingdisplay = std::string(joint_text.name + " Idx: " + std::to_string(frame_selected_anim_texture_idx) + " Settings:");
                                            ImGui::Text(tsettingdisplay.c_str());


                                            if (joint_text.get_anim_texture_or_null(frame_selected_anim_texture_idx) != nullptr) {

                                                auto& selected_anim_text = joint_text.animation_texture_library[frame_selected_anim_texture_idx];

                                                ImGui::PushItemWidth(-FLT_MIN);

                                                ImGui::Text("Offset X: "); 
                                                ImGui::SameLine();
                                                ImGui::DragFloat("##offx", &selected_anim_text.offset.x, 0.1f);
                                                
                                                ImGui::Text("Offset Y: "); 
                                                ImGui::SameLine();
                                                ImGui::DragFloat("##offy", &selected_anim_text.offset.y, 0.1f);

                                                ImGui::Text("Scale X: "); 
                                                ImGui::SameLine();
                                                ImGui::DragFloat("##scalex", &selected_anim_text.scale.x, 0.01f);

                                                ImGui::Text("Scale Y: "); 
                                                ImGui::SameLine();
                                                ImGui::DragFloat("##scaley", &selected_anim_text.scale.y, 0.01f);

                                                ImGui::Text("Crop Min X: "); 
                                                ImGui::SameLine();
                                                ImGui::DragFloat("##cminx", &selected_anim_text.crop_min.x, 0.1f);

                                                ImGui::Text("Crop Min Y: "); 
                                                ImGui::SameLine();
                                                ImGui::DragFloat("##cminy", &selected_anim_text.crop_min.y, 0.1f);

                                                ImGui::Text("Crop Max X: "); 
                                                ImGui::SameLine();
                                                ImGui::DragFloat("##cmaxx", &selected_anim_text.crop_max.x, 0.1f);
                                                
                                                ImGui::Text("Crop Max Y: "); 
                                                ImGui::SameLine();
                                                ImGui::DragFloat("##cmaxy", &selected_anim_text.crop_max.y, 0.1f);

                                                ImGui::Text("Rotation: "); 
                                                ImGui::SameLine();
                                                float deg = selected_anim_text.rotation * 180.0f / 3.14159265f;
                                                if (ImGui::DragFloat("##rot", &deg, 1.0f)) {
                                                    selected_anim_text.rotation = deg * 3.14159265f / 180.0f;
                                                }

                                                ImGui::PopItemWidth();

                                            } else {

                                                ImGui::Separator();

                                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

                                                ImGui::Text("Note: The joint texture index");
                                                ImGui::Text("chosen for this frame refers");
                                                ImGui::Text("to the anchor frame's Root");
                                                ImGui::Text("Texture texturing (idx: -1).");

                                                ImGui::Text("");

                                                ImGui::Text("Any changes made to these");
                                                ImGui::Text("settings WILL persist to the");
                                                ImGui::Text("Anchor Frame and any other");
                                                ImGui::Text("frames that use default");
                                                ImGui::Text("texturing.");

                                                ImGui::PopStyleColor();

                                                ImGui::Separator();
                                                
                                                ImGui::PushItemWidth(-FLT_MIN);

                                                ImGui::Text("Offset X: "); 
                                                ImGui::SameLine();
                                                ImGui::DragFloat("##offx", &joint_text.offset.x, 0.1f);
                                                
                                                ImGui::Text("Offset Y: "); 
                                                ImGui::SameLine();
                                                ImGui::DragFloat("##offy", &joint_text.offset.y, 0.1f);

                                                ImGui::Text("Scale X: "); 
                                                ImGui::SameLine();
                                                ImGui::DragFloat("##scalex", &joint_text.scale.x, 0.01f);

                                                ImGui::Text("Scale Y: "); 
                                                ImGui::SameLine();
                                                ImGui::DragFloat("##scaley", &joint_text.scale.y, 0.01f);

                                                ImGui::Text("Crop Min X: "); 
                                                ImGui::SameLine();
                                                ImGui::DragFloat("##cminx", &joint_text.crop_min.x, 0.1f);

                                                ImGui::Text("Crop Min Y: "); 
                                                ImGui::SameLine();
                                                ImGui::DragFloat("##cminy", &joint_text.crop_min.y, 0.1f);

                                                ImGui::Text("Crop Max X: "); 
                                                ImGui::SameLine();
                                                ImGui::DragFloat("##cmaxx", &joint_text.crop_max.x, 0.1f);
                                                
                                                ImGui::Text("Crop Max Y: "); 
                                                ImGui::SameLine();
                                                ImGui::DragFloat("##cmaxy", &joint_text.crop_max.y, 0.1f);

                                                ImGui::Text("Rotation: "); 
                                                ImGui::SameLine();
                                                float deg = joint_text.rotation * 180.0f / 3.14159265f;
                                                if (ImGui::DragFloat("##rot", &deg, 1.0f)) {
                                                    joint_text.rotation = deg * 3.14159265f / 180.0f;
                                                }

                                                ImGui::PopItemWidth();
                                            }

                                            ImGui::Separator();

                                        }
 
                                    }


                                    
                                    


                                    // ============================ Draw Order Tooling ============================= //
                                    if (selected_anim_frame != -1) {
                                        // Draw order list, unique ids
                                        

                                        auto& joint = animations[anim_selected].frames[selected_anim_frame].joints[jointselected];
        
                                        ImGui::Text(std::string("Draw Order: " + std::to_string(joint.draw_order)).c_str());
                                        ImGui::BeginChild(
                                            "dorder",
                                            ImVec2(ImGui::GetContentRegionAvail().x, 110),
                                            true
                                        );

                                        std::vector<std::string> nameDO;
                                        std::vector<int> unid;

                                        // Um this is extremely unoptimized code im pretty sure...
                                        // like really bad... but i couldnt think of a better way off the top of my head
                                        // so fuck you

                                        for (int i = 0; i < animations[anim_selected].frames[selected_anim_frame].joints.size(); i++) {
                                            
                                            int ujid = -1;

                                            for (AnimJointAdjustmentFrame j : animations[anim_selected].frames[selected_anim_frame].joints) {

                                                // Optimization?
                                                if (j.draw_order < i) {
                                                    continue;
                                                }

                                                if (j.draw_order == i) {
                                                    unid.push_back(j.unique_id);
                                                    ujid = j.unique_id;
                                                    break;
                                                }
                                            }

                                            for (int j = 0; j < (*avatar_selected).default_texturing.joints.size(); j++ ) {
                                                const auto& position = (*avatar_selected).default_frame.joints[j];
                                                const auto& texture = (*avatar_selected).default_texturing.joints[j];

                                                if (position.unique_id == ujid) {
                                                    nameDO.push_back(texture.name);
                                                    break;
                                                }
                                            }
                                        
                                        }

                                        assert(nameDO.size() == unid.size());

                                        // Display draw order
                                        for (int i = 0; i < nameDO.size(); i++) {

                                            if (joint.unique_id == unid[i]) {
                                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.0f, 1.0f));
                                            }

                                            std::string disp = std::string(nameDO[i] + " > Unique JID: " + std::to_string(unid[i]));
                                            ImGui::Text(disp.c_str());

                                            if (joint.unique_id == unid[i]) {
                                                ImGui::PopStyleColor();
                                            }
                                        }

                                        ImGui::EndChild();
                                        

                                        if (ImGui::Button("^^ Draw Order")) {

                                            if (avatar_selected != nullptr  && avatar_selected_idx != -1 && anim_selected != -1) {

                                                if (jointselected != -1 && (*avatar_selected).default_texturing.joints.size() >= 2 && jointselected >= 0 && jointselected < (*avatar_selected).default_texturing.joints.size()) {
                                                    auto& frame_joints = animations[anim_selected].frames[selected_anim_frame].joints;
                                                    int sel_draw_order = frame_joints[jointselected].draw_order;
                                                    int swap_idx = -1;

                                                    for (int i = 0; i < frame_joints.size(); i++) {
                                                        if (i != jointselected && frame_joints[i].draw_order == sel_draw_order - 1) {
                                                            swap_idx = i;
                                                            break;
                                                        }
                                                    }

                                                    if (swap_idx != -1) {
                                                        frame_joints[jointselected].draw_order = frame_joints[swap_idx].draw_order;
                                                        frame_joints[swap_idx].draw_order = sel_draw_order;
                                                    }
                                                }
                                            }

                                            
                                        }

                                        if (ImGui::Button("vv Draw Order")) {

                                            if (avatar_selected != nullptr && avatar_selected_idx != -1) {

                                                if (jointselected != -1 && (*avatar_selected).default_texturing.joints.size() >= 2 && jointselected >= 0 && jointselected < (*avatar_selected).default_texturing.joints.size()) {
                                                    auto& frame_joints = animations[anim_selected].frames[selected_anim_frame].joints;
                                                    int sel_draw_order = frame_joints[jointselected].draw_order;
                                                    int swap_idx = -1;

                                                    for (int i = 0; i < frame_joints.size(); i++) {
                                                        if (i != jointselected && frame_joints[i].draw_order == sel_draw_order + 1) {
                                                            swap_idx = i;
                                                            break;
                                                        }
                                                    }

                                                    if (swap_idx != -1) {
                                                        frame_joints[jointselected].draw_order = frame_joints[swap_idx].draw_order;
                                                        frame_joints[swap_idx].draw_order = sel_draw_order;
                                                    }

                                                }
                                            }
                                        }

                                    }

                                    


                                } else {

                                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.1f, 0.1f, 1.0f));
                                    ImGui::Text("No joint selected");
                                    ImGui::PopStyleColor();

                                }

                                // ================= Viewing Options ================= //
                                ImGui::Separator();
                                ImGui::Text("View Settings:");

                                // Onion the previous frame,
                                // Onion the anchor frame,

                                ImGui::Separator();

                                ImGui::Checkbox("Anchor Frame Ref", &onion_frame_anch);
                                ImGui::Checkbox("Previous Frame Ref", &onion_frame_prev);
                                // ===================================================== //

                                ImGui::EndTabItem();
                            }

                        }

                        if (selected_anim_frame != -1 && !frame_control_disabled) {

                            ImGuiTabItemFlags timing_tab_flags = force_open_timing_tab ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
                            if (ImGui::BeginTabItem("Timing", nullptr, timing_tab_flags)) {
                                force_open_timing_tab = false;

                                // ----- set the main static variable controller this shit to true: ------
                                editing_anim_w_interpolation = true;

                                // ============================================================
                                // TIMELINE PANEL SPACE
                                // ============================================================

                                ImVec2 p0 = ImGui::GetCursorScreenPos();
                                ImVec2 avail = ImGui::GetContentRegionAvail();

                                ImDrawList* dl = ImGui::GetWindowDrawList();

                                dl->AddRectFilled(
                                    p0,
                                    ImVec2(p0.x + avail.x, p0.y + avail.y),
                                    IM_COL32(22,22,22,255)
                                );

                                

                                dl->AddRectFilled(
                                    ImVec2(p0.x, 
                                        p0.y + ((0 * pixelsPerFrame) - timelineScroll)),
                                    ImVec2(p0.x + avail.x, 
                                        p0.y + (((animation_tick_frame_length - 1) * pixelsPerFrame) - timelineScroll)),
                                    IM_COL32(34,34,34,255)
                                );

                                

                            

                                // ============================================================
                                // SCROLL + ZOOM
                                // ============================================================

                                ImVec2 mouse = ImGui::GetIO().MousePos;

                                bool hovered =
                                    mouse.x >= p0.x &&
                                    mouse.x <= p0.x + avail.x &&
                                    mouse.y >= p0.y &&
                                    mouse.y <= p0.y + avail.y;

                                if (hovered) {
                                    if (ImGui::GetIO().KeyCtrl) {

                                        pixelsPerFrame += ImGui::GetIO().MouseWheel * 2.0f;

                                        pixelsPerFrame = std::clamp(
                                            pixelsPerFrame,
                                            6.0f,
                                            60.0f
                                        );

                                    } else {

                                        timelineScroll -= ImGui::GetIO().MouseWheel * 60.0f;

                                        if (timelineScroll < -80.0f)
                                            timelineScroll = -80.0f;
                                    }
                                }

                                // ============================================================
                                // RAIL
                                // ============================================================

                                float railX = p0.x + 70.0f;

                                dl->AddLine(
                                    ImVec2(railX, p0.y),
                                    ImVec2(railX, p0.y + avail.y),
                                    IM_COL32(90,90,90,255),
                                    3.0f
                                );

                                // ============================================================
                                // DRAW FRAMES
                                // ============================================================

                                bool endReached = false;
                                int lastFrame = 0;

                                for (int frame = 0; frame <= animation_tick_frame_length - 1; frame++) {

                                    float y =
                                        p0.y +
                                        (frame * pixelsPerFrame) -
                                        timelineScroll;

                                    // cull invisible frames BEFORE visible region
                                    if (y < p0.y - 40.0f)
                                        continue;

                                    // cull invisible frames AFTER visible region
                                    if (y > p0.y + avail.y + 40.0f) {

                                        if (!endReached)
                                            lastFrame = frame;

                                        endReached = true;
                                        continue;
                                    }

                                    if (frame > animation_tick_frame_length - 1) {
                                        continue;
                                    }

                                    bool major = (frame % 10 == 0);

                                    float tick =
                                        major ? 26.0f : 14.0f;

                                    dl->AddLine(
                                        ImVec2(railX - tick * 0.5f, y),
                                        ImVec2(railX + tick * 0.5f, y),
                                        IM_COL32(170,170,170,255),
                                        major ? 2.0f : 1.0f
                                    );

                                    if (major) {

                                        dl->AddText(
                                            ImVec2(p0.x + 10.0f, y - 8.0f),
                                            IM_COL32(220,220,220,255),
                                            std::to_string(frame).c_str()
                                        );
                                    }
                                }


                                bool isSomeFlagHovered = false;

                                for (const auto& flag : flags) {

                                    float y =
                                        p0.y +
                                        (flag.frame * pixelsPerFrame) -
                                        timelineScroll;

                                    if (y < p0.y - 20.0f)
                                        continue;

                                    if (y > p0.y + avail.y + 20.0f)
                                        continue;

                                    bool this_flag_hovered =
                                        hovered &&
                                        mouse.y > y - 10.0f &&
                                        mouse.y < y + 10.0f &&
                                        mouse.x > railX - 10.0f &&
                                        mouse.x < railX + 140.0f;

                                    if (this_flag_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {

                                        // dont change current frame of scrubber not there
                                        if (!(ImGui::GetIO().KeyShift)) {
                                            currentFrame = flag.frame;
                                            selected_anim_frame = flag.keyframe_index;
                                        } else {
                                            selected_anim_frame = flag.keyframe_index;
                                        }
                                    }

                                    ImU32 drawColor = flag.color;

                                    if (this_flag_hovered || selected_anim_frame == flag.keyframe_index) {
                                        if (selected_anim_frame != flag.keyframe_index) {
                                            drawColor = IM_COL32(120,120,150,255);
                                        } else {
                                            if (!this_flag_hovered) {
                                                drawColor = IM_COL32(200,200,200,255);
                                            } else {
                                                drawColor = IM_COL32(255,255,255,255);
                                            }
                                        }
                                    }

                                    dl->AddLine(
                                        ImVec2(railX, y),
                                        ImVec2(railX + 14.0f, y),
                                        drawColor,
                                        2.0f
                                    );

                                    dl->AddRectFilled(
                                        ImVec2(railX + 14.0f, y - 5.0f),
                                        ImVec2(railX + 34.0f, y + 5.0f),
                                        drawColor,
                                        2.0f
                                    );

                                    dl->AddText(
                                        ImVec2(railX + 45.0f, y - 7.0f),
                                        IM_COL32(190,190,190,255),
                                        std::to_string(flag.keyframe_index).c_str()
                                    );

                                    if (this_flag_hovered)
                                        isSomeFlagHovered = true;

                                }


                                // =====================

                                // Given some random millisecond frame in the timeline, find what the last
                                // Key Frame defined was and return its index basically

                                auto getLastKFrameFromTick = [&](int tick_frame) -> int {

                                    if (animations[anim_selected].frames.size() == flags.size()) {
                                        int i = 0;
                                        for (const KeyAnimFrame& kframe : animations[anim_selected].frames) {
                                            const TimelineFlag flag = flags[i];
                                            if (tick_frame >= flag.frame && tick_frame < flag.frame + kframe.time_to_next + 1  ) {
                                                return i;
                                            }

                                            i++;
                                        }
                                    }

                                    // Failsafe I guess?
                                    return 0;
                                };



                                // ============================================================
                                // SCRUBBER
                                // ============================================================

                                float scrubY =
                                    p0.y +
                                    (currentFrame * pixelsPerFrame) -
                                    timelineScroll;

                                bool hoverScrubber =
                                    mouse.x >= p0.x &&
                                    mouse.x <= p0.x + avail.x &&
                                    mouse.y >= scrubY - 12.0f &&
                                    mouse.y <= scrubY + 12.0f;

                                static bool draggingScrubber = false;

                                if (hoverScrubber && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !(ImGui::GetIO().KeyShift)) {
                                    draggingScrubber = true;
                                }

                                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                                    draggingScrubber = false;
                                }

                                if (draggingScrubber) {

                                    // Scroll scrubber when dragging
                                    float local = mouse.y - p0.y + timelineScroll;
                                    currentFrame = (int)round(local / pixelsPerFrame);

                                    if (currentFrame > animation_tick_frame_length - 1) {
                                        currentFrame = animation_tick_frame_length - 1;
                                    }

                                    if (currentFrame < 0) {
                                        currentFrame = 0;
                                    }


                                    // Keep selected frame stable while scrubbing; scrubber drives preview independently.

                                    if (endReached && currentFrame > (lastFrame - 4)) {
                                        timelineScroll += pixelsPerFrame;
                                    }

                                    

                                    // Auto-scroll up when dragging near the top of timeline view.
                                    const float auto_scroll_top_zone = p0.y + 8.0f;
                                    if (mouse.y <= auto_scroll_top_zone) {
                                        timelineScroll = std::max(-80.0f, timelineScroll - (pixelsPerFrame * 0.75f));
                                    }

                                    
                                }

                                // scrubber line
                                dl->AddLine(
                                    ImVec2(p0.x, scrubY),
                                    ImVec2(p0.x + avail.x, scrubY),
                                    IM_COL32(255,40,40,255),
                                    3.0f
                                );

                                // scrubber triangle
                                dl->AddTriangleFilled(
                                    ImVec2(railX - 18.0f, scrubY),
                                    ImVec2(railX - 2.0f, scrubY - 10.0f),
                                    ImVec2(railX - 2.0f, scrubY + 10.0f),
                                    IM_COL32(255,50,50,255)
                                );

                                // current frame label
                                dl->AddText(
                                    ImVec2(p0.x + 140.0f, scrubY - 10.0f),
                                    IM_COL32(255,80,80,255),
                                    std::string("FRAME " + std::to_string(currentFrame)).c_str()
                                );

                                ImGui::InvisibleButton( "timeline_capture", avail);

                                if (!isSomeFlagHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                                    // Left mouse button double-click detected

                                    float local = mouse.y - p0.y + timelineScroll;
                                    currentFrame = (int)round(local / pixelsPerFrame);
                                    
                                    if (currentFrame > animation_tick_frame_length - 1) {
                                        currentFrame = animation_tick_frame_length - 1;
                                    }

                                }

                                ImGui::EndTabItem();
                            }

                        }
                        if (selected_anim_frame != -1 && frame_control_disabled) {
                            ImGui::BeginDisabled();
                            ImGuiTabItemFlags timing_tab_flags = force_open_timing_tab ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
                            if (ImGui::BeginTabItem("Timing", nullptr, timing_tab_flags)) {
                                force_open_timing_tab = false;
                                ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.45f, 1.0f), "Timing editor disabled: avatar and animation joint counts do not match.");
                                ImGui::Text("Restore matching joint count/IDs before editing timing.");
                                ImGui::EndTabItem();
                            }
                            ImGui::EndDisabled();
                        }

                        ImGui::EndTabBar();
                    }

                    ImGui::EndChild();
                    ImGui::GetStyle().WindowPadding = orig;

                } else {
                    editing_anim_w_interpolation = false;
                }



                ImVec2 saveAvatarModalSize = {
                    std::max(620.0f, GetScreenWidth() * 0.72f),
                    std::max(420.0f, GetScreenHeight() * 0.76f)
                };

                if (saveAvrPopup) {
                    ImGui::OpenPopup("Save Avatar");
                }

                if (saveAnimPopup) {
                    ImGui::OpenPopup("Save Animation");
                }

                if (leaveAvrEditorPopup) {
                    ImGui::OpenPopup("Leave Avatar Editor");
                }
                
                // Start the Save Avatar .avr file popup
                ImGui::GetStyle().WindowPadding = ImVec2(8,8);
                ImGui::SetNextWindowSize(saveAvatarModalSize, ImGuiCond_Appearing);

                if (ImGui::BeginPopupModal("Save Avatar", NULL, ImGuiWindowFlags_NoCollapse)) {

                    static int sel_avr_index = -1;
                    static bool init_avr_window = false;
                    static std::string sel_avr_filename = "";
                    static char avrBuffer[128];
                    static bool filename_exists = false;

                    ImGui::Text("To what file would you like to save this Avatar?");

                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

                    ImGui::Text("Warning! This filename already exists and saving will overrite data.");
                    ImGui::Text("(ignore if you are updating a .avr file)");


                    ImGui::PopStyleColor();

                    // --------------------->

                    std::vector<std::string> avrpaths = assets.GetFilepathsInDirectory(AVATARDIR, "avr");
                    std::vector<std::string> avrnames = assets.GetFilenamesInDirectory(AVATARDIR, "avr");

                    if (!init_avr_window) {
                        filename_exists = false;
                        for (int i = 0; i < avrnames.size(); i++) {
                            if (avrnames[i] == (*avatar_selected).name) {
                                sel_avr_index = i;
                                sel_avr_filename = avrnames[i];
                                filename_exists = true;
                                break;
                            }
                        }

                        if (!filename_exists) {
                            sel_avr_filename = avrBuffer;
                        }
                    }

                    ImGui::BeginChild("SaveAvrList", ImVec2(0, saveAvatarModalSize.y * 0.28f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);


                    for (int i = 0; i < (int)avrpaths.size(); i++) {
                        if (ImGui::Selectable(avrpaths[i].c_str(), sel_avr_index == i, ImGuiSelectableFlags_DontClosePopups)) {
                            sel_avr_index = i;
                            sel_avr_filename = strip_ext(avrnames[i]);
                        }
                    }



                    ImGui::EndChild();

                    
                    std::snprintf(avrBuffer, sizeof(avrBuffer), "%s", (*avatar_selected).name.c_str());

                    if (ImGui::InputText("Avatar Name", avrBuffer, sizeof(avrBuffer))) {
                        (*avatar_selected).name = avrBuffer;
                        sel_avr_index = -1;

                        filename_exists = false;

                        for (int i = 0; i < avrnames.size(); i++) {
                            if (avrnames[i] == (*avatar_selected).name) {
                                sel_avr_index = i;
                                sel_avr_filename = avrnames[i];
                                filename_exists = true;
                                break;
                            }
                        }

                        if (!filename_exists) {
                            sel_avr_filename = avrBuffer;
                        }
                    }

                    if (ImGui::Button("Save Avatar", ImVec2(70, 35))) {

                        (*avatar_selected).SaveAvrFile(sel_avr_filename, AVATARDIR);

                        saveAvrPopup = false;
                        init_avr_window = false;

                        ImGui::CloseCurrentPopup();
                    
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Cancel", ImVec2(70, 35))) {

                        saveAvrPopup = false;
                        init_avr_window = false;

                        ImGui::CloseCurrentPopup();
                    
                    }

                    ImGui::EndPopup();

                }


                if (ImGui::BeginPopupModal("Save Animation", NULL, ImGuiWindowFlags_NoCollapse)) {

                    static int sel_anim_index = -1;
                    static bool init_anim_window = false;
                    static std::string sel_anim_filename = "";
                    static char animBuffer[128];
                    static bool filename_exists = false;

                    ImGui::Text("To what file would you like to save this Animation?");

                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

                    ImGui::Text("Warning! This filename already exists and saving will overrite data.");
                    ImGui::Text("(ignore if you are updating a .anim file)");


                    ImGui::PopStyleColor();

                    // --------------------->

                    std::vector<std::string> animpaths = assets.GetFilepathsInDirectory(ANIMATIONDIR, "anim");
                    std::vector<std::string> animnames = assets.GetFilenamesInDirectory(ANIMATIONDIR, "anim");

                    if (!init_anim_window) {
                        filename_exists = false;
                        for (int i = 0; i < animnames.size(); i++) {
                            if (animnames[i] == animations[anim_selected].name) {
                                sel_anim_index = i;
                                sel_anim_filename = animnames[i];
                                filename_exists = true;
                                break;
                            }
                        }

                        if (!filename_exists) {
                            sel_anim_filename = animBuffer;
                        }
                    }

                    ImGui::BeginChild("SaveAnimList", ImVec2(0, saveAvatarModalSize.y * 0.28f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);


                    for (int i = 0; i < (int)animpaths.size(); i++) {
                        if (ImGui::Selectable(animpaths[i].c_str(), sel_anim_index == i, ImGuiSelectableFlags_DontClosePopups)) {
                            sel_anim_index = i;
                            sel_anim_filename = strip_ext(animnames[i]);
                        }
                    }



                    ImGui::EndChild();

                    
                    std::snprintf(animBuffer, sizeof(animBuffer), "%s", animations[anim_selected].name.c_str());

                    if (ImGui::InputText("Animation Name", animBuffer, sizeof(animBuffer))) {
                        animations[anim_selected].name = animBuffer;
                        sel_anim_index = -1;

                        filename_exists = false;

                        for (int i = 0; i < animnames.size(); i++) {
                            if (animnames[i] == animations[anim_selected].name) {
                                sel_anim_index = i;
                                sel_anim_filename = animnames[i];
                                filename_exists = true;
                                break;
                            }
                        }

                        if (!filename_exists) {
                            sel_anim_filename = animBuffer;
                        }

                    }

                    
                    if (ImGui::Button("Save Animation", ImVec2(100, 35))) {

                        animations[anim_selected].SaveAnimFile(sel_anim_filename, ANIMATIONDIR);

                        saveAnimPopup = false;
                        init_anim_window = false;

                        ImGui::CloseCurrentPopup();
                    
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Cancel", ImVec2(70, 35))) {

                        saveAnimPopup = false;
                        init_anim_window = false;

                        ImGui::CloseCurrentPopup();
                    
                    }

                    ImGui::EndPopup();

                }


                if (ImGui::BeginPopupModal("Leave Avatar Editor", NULL, ImGuiWindowFlags_NoCollapse)) {

                    ImGui::Text("Are you sure you want to leave the Avatar Editor?");

                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

                    ImGui::Text("Any unsaved changes will be lost when syncing main menu with");
                    ImGui::Text("file data on the disk.");

                    ImGui::PopStyleColor();

                    ImGui::Separator();

                    if (ImGui::Button("Confirm Exit Editor", ImVec2(210, 30))) {

                        SyncAndReloadAvatars(AVATARDIR);
                        SyncAndReloadAnimations(ANIMATIONDIR);

                        // Reset static constroller variables so there is no problem with accessing things
                        avatar_selected_idx = -1;
                        avatar_selected = nullptr;
                        anim_selected = -1;
                        selected_anim_frame = -1;
                        timings_preview_frame = -1;
                        play_preview_animation = false;
                        preview_anim_accumulated_ms = 0;

                        avatarMenu = AvatarCreatorMenu::AVATAR_SELECTION;

                        ImGui::CloseCurrentPopup();

                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Cancel", ImVec2(50, 30))) {
                        ImGui::CloseCurrentPopup();
                    }


                    ImGui::EndPopup();

                }
                

                
            }

            

        };

        

        ImGui::End();
        ImGui::PopStyleColor(6);

        



    };


    // End of definition for avatar editor









    // Level editor code

    auto DrawLayerManager = [&] () {

        ImVec2 orig = ImGui::GetStyle().WindowPadding;
        ImGui::GetStyle().WindowPadding = ImVec2(4,4);



        // Anchor to top-right corner
            ImVec2 windowSize = ImVec2(220 * fullscreenScale.x, 300 * fullscreenScale.y);
            ImVec2 windowPos = ImVec2(
                GetScreenWidth() - windowSize.x - 10.0f,
                30.0f * fullscreenScale.y
            );

            ImGui::SetNextWindowPos(windowPos, ImGuiCond_Once);
            ImGui::SetNextWindowSize(windowSize, ImGuiCond_Once);

            ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.2f, 0.2f, 0.7f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.1f, 0.1f, 0.3f, 1.0f));

            if (ImGui::Begin("Layer Manager", nullptr ))
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

                        if (selectedLayer == i) {
                            selectedLayer = -1;
                            scene.EDITOR_ONLY_SELECTED_LAYER = selectedLayer;
                        } else {
                            selectedLayer = i;
                            scene.EDITOR_ONLY_SELECTED_LAYER = selectedLayer;
                        }
                    }
                }

                ImGui::EndChild();

                // =========================
                // Buttons
                // =========================

                float buttonWidth = (ImGui::GetContentRegionAvail().x - 3 * ImGui::GetStyle().ItemSpacing.x) / 4.0f;


                // Add Layer
                if (ImGui::Button(" + ", ImVec2(buttonWidth, 0))) {
                    scene.tiles_push_new_layer();
                    selectedLayer = (int)scene.tile_layers.size() - 1;
                    scene.EDITOR_ONLY_SELECTED_LAYER = selectedLayer;
                }

                ImGui::SameLine();

                // Delete Layer
                if (ImGui::Button(" - ", ImVec2(buttonWidth, 0))) {
                    if (!scene.tile_layers.empty() && selectedLayer >= 0 && selectedLayer < (int)scene.tile_layers.size()) {
                        scene.tile_layers.erase(scene.tile_layers.begin() + selectedLayer);

                        if (selectedLayer >= (int)scene.tile_layers.size()) {
                            selectedLayer = (int)scene.tile_layers.size() - 1;
                        }
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button(" ^ ", ImVec2(buttonWidth, 0))) {

                    if (!scene.tile_layers.empty() && selectedLayer > 0 && selectedLayer < (int)scene.tile_layers.size()) {

                        // Swap up
                        TileGrid temp = scene.tile_layers[selectedLayer];
                        scene.tile_layers[selectedLayer] = scene.tile_layers[selectedLayer - 1];
                        scene.tile_layers[selectedLayer - 1] = temp;

                    }

                }

                ImGui::SameLine();

                if (ImGui::Button(" v ", ImVec2(buttonWidth, 0))) {

                    if (!scene.tile_layers.empty() && selectedLayer >= 0 && selectedLayer < (int)scene.tile_layers.size() - 1) {

                        // Swap down
                        TileGrid temp = scene.tile_layers[selectedLayer];
                        scene.tile_layers[selectedLayer] = scene.tile_layers[selectedLayer + 1];
                        scene.tile_layers[selectedLayer + 1] = temp;

                    }
                    
                }

                // =========================
                // Info
                // =========================
                ImGui::Separator();
                ImGui::Text("Editing Layer: %d", selectedLayer);

                ImGui::SameLine();

                static bool onion = false;

                ImGui::Checkbox(" | Onion: ", &onion);

                // expose this to rest of editor
                scene.EDITOR_ONLY_ONION_LAYER_MODE = onion;
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

            if (selectedIndex >= 0 && selectedIndex < (int)scene.loaded_atlases.size()) {

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

                const int total_tiles = total_cols * total_rows;
                atlas_tiles_per_page = std::clamp(atlas_tiles_per_page, 32, 2048);
                atlas_page_columns = std::clamp(atlas_page_columns, 1, 64);
                const int atlas_total_pages = std::max(1, (total_tiles + atlas_tiles_per_page - 1) / atlas_tiles_per_page);
                atlas_page_index = std::clamp(atlas_page_index, 0, atlas_total_pages - 1);

                ImGui::PushItemWidth(120 * fullscreenScale.x);
                ImGui::SliderInt("Atlas Page Size", &atlas_tiles_per_page, 32, 2048);
                ImGui::SliderInt("Atlas Columns", &atlas_page_columns, 1, 64);
                ImGui::PopItemWidth();
                if (ImGui::Button("<##atlas_page_prev") && atlas_page_index > 0) {
                    atlas_page_index--;
                }
                ImGui::SameLine();
                if (ImGui::Button(">##atlas_page_next") && atlas_page_index + 1 < atlas_total_pages) {
                    atlas_page_index++;
                }
                ImGui::SameLine();
                ImGui::Text("Page %d / %d (%d tiles)", atlas_page_index + 1, atlas_total_pages, total_tiles);
                ImGui::Separator();

                const int atlas_page_start = atlas_page_index * atlas_tiles_per_page;
                const int atlas_page_end = std::min(total_tiles, atlas_page_start + atlas_tiles_per_page);
                int page_tile_counter = 0;

                for (int tile_index = atlas_page_start; tile_index < atlas_page_end; tile_index++) {
                    const int row = tile_index / total_cols;
                    const int col = tile_index % total_cols;

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

                        page_tile_counter++;
                        if (page_tile_counter % atlas_page_columns != 0 && tile_index + 1 < atlas_page_end) {
                            ImGui::SameLine();
                        }






                        ImGui::PopStyleColor(3);
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

    static bool showCreateAvatarEditor = false;

    bool triggerOpenScenePopup = false;
    bool triggerSaveScenePopup = false;

    ImGui::GetStyle().FramePadding = ImVec2(4, 4);

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene")) {}
            if (ImGui::MenuItem("Open Scene")) {
                openSceneSelectedIndex = -1;
                triggerOpenScenePopup = true;
            }
            if (ImGui::MenuItem("Save Scene")) {
                saveSceneInitialized = false;
                triggerSaveScenePopup = true;
            }
            ImGui::Separator();


            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                CloseWindow();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Show All Debug Vars", nullptr, &G_DEBUGGER.showAllInfo);
            ImGui::MenuItem("Show Coll Outlines", "Permits Tile Editing", &G_DEBUGGER.showTileOutlines);
            ImGui::MenuItem("Show Cam Clamps", nullptr, &G_DEBUGGER.showCameraClamps);
            ImGui::MenuItem("Show PhysBody HBs", nullptr, &G_DEBUGGER.showPhysicsBodyHitboxes);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            ImGui::MenuItem("Layer Manager", nullptr, &showLayerManager);
            ImGui::MenuItem("Workspace", nullptr, &WORKSPACE_WINDOW_DRAW);
            
            if (ImGui::MenuItem("Avatars...", nullptr, &showCreateAvatarEditor)) {
                dcaeJustOpened = true;
            }
            

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Shaders")) {
            ImGui::MenuItem("Enable Shader (Standard Build)", nullptr, &window.painter_enabled_game);
            ImGui::MenuItem("Enable Shader (Editor Viewport)", nullptr, &window.painter_enabled_editor);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Player")) {
            ImGui::MenuItem("Avatar Visible", nullptr, &G_DEBUGGER.playerAvatarVisible);
            ImGui::MenuItem("Invincible", nullptr, &G_DEBUGGER.playerInvincible);
            ImGui::MenuItem("Unexpirable", nullptr, &G_DEBUGGER.playerUnexpirable);
            ImGui::MenuItem("Infinite Aura", nullptr, &G_DEBUGGER.playerInfiniteAura);
            ImGui::MenuItem("Can Fly", nullptr, &G_DEBUGGER.playerCanFly);
            ImGui::EndMenu();
        }

        


        ImGui::EndMainMenuBar();

    }

    if (triggerOpenScenePopup) {
        ImGui::OpenPopup("Open Scene");
    }
    if (triggerSaveScenePopup) {
        ImGui::OpenPopup("Save Scene");
    }

    std::filesystem::create_directories(SCENEDIR);

    // Save modal or wtv

    ImVec2 saveSceneModalSize = {
        std::max(620.0f, GetScreenWidth() * 0.72f),
        std::max(420.0f, GetScreenHeight() * 0.76f)
    };


    ImGui::GetStyle().WindowPadding = ImVec2(8,8);

    ImGui::SetNextWindowSize(saveSceneModalSize, ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Save Scene", NULL, ImGuiWindowFlags_NoCollapse)) {

        std::vector<std::string> scenepaths = assets.GetFilepathsInDirectory(SCENEDIR, "scene");
        std::vector<std::string> scenenames = assets.GetFilenamesInDirectory(SCENEDIR, "scene");

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
                finalPath = SCENEDIR + strip_ext(saveSceneNameInput) + ".scene";
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

        std::vector<std::string> scenepaths = assets.GetFilepathsInDirectory(SCENEDIR, "scene");

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

                    selectedIndex = -1;
                    phystab_selectedTileIndex = -1;



                    if (scene.tile_layers.empty()) {
                        selectedLayer = 0;
                    } else if (selectedLayer >= (int)scene.tile_layers.size()) {
                        selectedLayer = (int)scene.tile_layers.size() - 1;
                    } else if (selectedLayer < 0) {
                        selectedLayer = 0;
                    }

                    scene.EDITOR_ONLY_SELECTED_ATLAS = -1;
                    scene.EDITOR_ONLY_SELECTED_PALLET_TILE = -1;
                    scene.EDITOR_ONLY_SELECTED_LAYER = selectedLayer;

                    selectedLayer = 0;

                    selectedTileIndex = -1;


                    animParamsMatch = true;
                    phystab_selectedTileIndex = -1;
                    newTilesetSplitMatch = true;


                    // Reset positions on load
                    for (Entity player : registry.view<tag::Player>()) {
                        auto & transform = registry.get_component<comp::Transform>(player);
                        transform.position = transform.init_position; // {200.0f, -1080.0f};
                    }

                    for (Entity camera : registry.view<comp::Camera>()) {
                        auto & transform = registry.get_component<comp::Transform>(camera);
                        transform.position = transform.init_position;
                    }

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

    if (showCreateAvatarEditor) {
        DrawCreateAvatarEditor();
    }







    // Workspace Area
    // ================================
    // ..........................
    // .............

    ImGui::SetNextWindowPos(
        {0 * fullscreenScale.x, (720 - 215) * fullscreenScale.y},
        ImGuiCond_Once
    );

    ImGui::SetNextWindowSize(
        {1280 * fullscreenScale.x, 215 * fullscreenScale.y},
        ImGuiCond_Once
    );

    static bool matchfs = fullscreen;

    if (matchfs != fullscreen) {
        matchfs = fullscreen;
        ImGui::SetNextWindowPos(
            {0 * fullscreenScale.x, (720 - 215) * fullscreenScale.y}
        );

        ImGui::SetNextWindowSize(
            {1280 * fullscreenScale.x, 215 * fullscreenScale.y}
        );
    }


    ImGui::SetNextWindowSizeConstraints(
        ImVec2(700 * fullscreenScale.x, 215 * fullscreenScale.y),
        ImVec2((float)GetScreenWidth(), 700 * fullscreenScale.y)
    );

    if (WORKSPACE_WINDOW_DRAW) {    

        if (ImGui::Begin("Workspace", nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoTitleBar)) {

            ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(0.22f, 0.22f, 0.26f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(0.18f, 0.18f, 0.22f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TabUnfocused, ImVec4(0.09f, 0.09f, 0.11f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, ImVec4(0.14f, 0.14f, 0.17f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.22f, 0.22f, 0.26f, 1.0f));

            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

            float btnW = 22.0f * fullscreenScale.x;
            float btnH = 18.0f * fullscreenScale.y;
            ImVec2 winPos = ImGui::GetWindowPos();
            ImVec2 winSize = ImGui::GetWindowSize();
            ImVec2 btnMin = ImVec2(winPos.x + winSize.x - btnW - 4.0f, winPos.y + 2.0f);
            ImVec2 btnMax = ImVec2(btnMin.x + btnW, btnMin.y + btnH);

            ImDrawList* dl = ImGui::GetWindowDrawList();
            bool hovered = ImGui::IsMouseHoveringRect(btnMin, btnMax);
            bool clicked = hovered && ImGui::IsMouseClicked(0);

            dl->AddRectFilled(btnMin, btnMax, hovered ? IM_COL32(80,80,80,200) : IM_COL32(50,50,50,150));
            dl->AddText(ImVec2(btnMin.x + btnW * 0.3f, btnMin.y + 2.0f), IM_COL32(255,255,255,255), "-");

            if (clicked) {
                WORKSPACE_WINDOW_DRAW = false;
            }

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

                    const float tilesetListHeight = std::max(100.0f, ImGui::GetContentRegionAvail().y - 12.0f);
                    ImGui::BeginChild("ItemListPanel", ImVec2(220 * fullscreenScale.x, tilesetListHeight), true);

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

                        std::vector<std::string> tileset_paths = assets.GetTilesetPaths(TILESETDIR);

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
                            ImGui::BeginChild("TilesetSplitPreviewChild", previewChildSize, true);

                            const int imageCols = std::max(1, (int)std::ceil((float)previewTilesetTexture.width / (float)new_tilesize));
                            const int imageRows = std::max(1, (int)std::ceil((float)previewTilesetTexture.height / (float)new_tilesize));
                            const int gridCols = std::max(imageCols, new_split_columns);
                            const int gridRows = std::max(imageRows, new_split_rows);
                            const int previewGridCellCount = gridCols * gridRows;
                            const int maxPreviewGridCells = 4096;

                            if (previewGridCellCount > maxPreviewGridCells) {
                                ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "Image is too large for split preview.");
                                ImGui::Text("No preview available (%d cells requested).", previewGridCellCount);
                                ImGui::Text("Preview maximum: %d cells.", maxPreviewGridCells);
                            } else {
                                ImDrawList * drawList = ImGui::GetWindowDrawList();
                                ImVec2 canvasStart = ImGui::GetCursorScreenPos();
                                const float padding = 8.0f;
                                const float maxPreviewWidth = std::max(320.0f, loadTilesetModalSize.x - 56.0f);
                                const float maxPreviewHeight = std::max(180.0f, loadTilesetModalSize.y * 0.35f);
                                const float imgScale = std::max(
                                    0.001f,
                                    std::min(maxPreviewWidth / (float)previewTilesetTexture.width, maxPreviewHeight / (float)previewTilesetTexture.height)
                                );

                                const float scaledTileSize = (float)new_tilesize * imgScale;
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

                                // Draw by lines instead of one rectangle per cell, which can exceed ImGui's index limits.
                                const int maxPreviewLines = 4096;
                                if ((gridCols + gridRows + 2) > maxPreviewLines) {
                                    ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "Split grid too dense to draw safely.");
                                    ImGui::Text("Showing image only. Reduce rows/columns for grid lines.");
                                } else {
                                    const ImVec2 imageBottomRight = {
                                        canvasStart.x + previewTilesetTexture.width * imgScale,
                                        canvasStart.y + previewTilesetTexture.height * imgScale
                                    };

                                    for (int c = 0; c <= gridCols; c++) {
                                        const float x = canvasStart.x + (float)c * scaledTileSize;
                                        const bool inSelection = c <= new_split_columns;
                                        const bool fitsBounds = (c * new_tilesize) <= previewTilesetTexture.width;
                                        const ImU32 color = (inSelection && fitsBounds) ? gridColorValid : gridColorInvalid;
                                        drawList->AddLine({x, canvasStart.y}, {x, imageBottomRight.y}, color, 1.2f);
                                    }

                                    for (int r = 0; r <= gridRows; r++) {
                                        const float y = canvasStart.y + (float)r * scaledTileSize;
                                        const bool inSelection = r <= new_split_rows;
                                        const bool fitsBounds = (r * new_tilesize) <= previewTilesetTexture.height;
                                        const ImU32 color = (inSelection && fitsBounds) ? gridColorValid : gridColorInvalid;
                                        drawList->AddLine({canvasStart.x, y}, {imageBottomRight.x, y}, color, 1.2f);
                                    }
                                }

                                ImGui::Dummy(ImVec2(previewSize.x + padding, previewSize.y + padding));
                            }
                            ImGui::EndChild();

                        }


                        ImGui::Spacing();

                        // CONFIRM BUTTON
                        ImGui::BeginDisabled(!canConfirm);
                        if (ImGui::Button("Confirm", ImVec2(120, 0))) {


                            loadTilesetNameError = !uniqueName;

                            if (canConfirm) {
                                Texture2D & importedTexture = assets.LoadTilesetTexture(selectedTilesetPathToLoad);

                                scene.load_new_tileset(newAtlasNameTrimmed, importedTexture, new_tilesize, new_split_columns, new_split_rows, selectedTilesetPathToLoad);
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
                            ImGui::BeginChild("PhysicsTileGrid", ImVec2(260 * fullscreenScale.x, 135 * fullscreenScale.y), true, ImGuiWindowFlags_NoScrollbar);
                        } else {
                            ImGui::BeginChild("PhysicsTileGrid", ImVec2(260 * fullscreenScale.x, 110 * fullscreenScale.y), true, ImGuiWindowFlags_NoScrollbar);
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
                            const int total_tiles = total_cols * total_rows;
                            physics_tiles_per_page = std::clamp(physics_tiles_per_page, 16, 1024);
                            physics_page_columns = std::clamp(physics_page_columns, 1, 32);
                            const int physics_total_pages = std::max(1, (total_tiles + physics_tiles_per_page - 1) / physics_tiles_per_page);
                            physics_page_index = std::clamp(physics_page_index, 0, physics_total_pages - 1);

                            ImGui::PushItemWidth(120 * fullscreenScale.x);
                            ImGui::SliderInt("Physics Page Size", &physics_tiles_per_page, 16, 1024);
                            ImGui::SliderInt("Physics Columns", &physics_page_columns, 1, 32);
                            ImGui::PopItemWidth();
                            if (ImGui::Button("<##phys_page_prev") && physics_page_index > 0) {
                                physics_page_index--;
                            }
                            ImGui::SameLine();
                            if (ImGui::Button(">##phys_page_next") && physics_page_index + 1 < physics_total_pages) {
                                physics_page_index++;
                            }
                            ImGui::SameLine();
                            ImGui::Text("Page %d / %d (%d tiles)", physics_page_index + 1, physics_total_pages, total_tiles);
                            ImGui::Separator();

                            const int phys_page_start = physics_page_index * physics_tiles_per_page;
                            const int phys_page_end = std::min(total_tiles, phys_page_start + physics_tiles_per_page);

                            int page_tile_counter = 0;
                            for (int tile_index = phys_page_start; tile_index < phys_page_end; tile_index++) {
                                const int row = tile_index / total_cols;
                                const int col = tile_index % total_cols;

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

                                    // For future reference, this is where we draw the specific
                                    // icon slices for the specific collision types

                                    if (coll == CollisionType::COLL_EMPTY) {
                                        ico_slice = editorAssets.util_tileset_geticon(CollisionIcons::UTIL_EMPTY_COL_ICO );
                                    } else if (coll == CollisionType::COLL_FULL_SOLID) {
                                        ico_slice = editorAssets.util_tileset_geticon(CollisionIcons::UTIL_FULL_COL_ICO );
                                    } else if (coll == CollisionType::COLL_PSLOPE1_SOLID) {
                                        ico_slice = editorAssets.util_tileset_geticon(CollisionIcons::UTIL_PSLOPE1_COL_ICO );
                                    } else if (coll == CollisionType::COLL_NSLOPE1_SOLID) {
                                        ico_slice = editorAssets.util_tileset_geticon(CollisionIcons::UTIL_NSLOPE1_COL_ICO );
                                    } else if (coll == CollisionType::COLL_FULL_SEMISOLID) {
                                        ico_slice = editorAssets.util_tileset_geticon(CollisionIcons::UTIL_FULL_SEMISOL_COL_ICO );
                                    } else if (coll == CollisionType::COLL_GROUND_HAZARD) {
                                        ico_slice = editorAssets.util_tileset_geticon(CollisionIcons::UTIL_GROUND_HAZARD_COL_ICO );
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

                                    page_tile_counter++;
                                    if ((page_tile_counter % physics_page_columns) != 0 && (tile_index + 1) < phys_page_end) {
                                        ImGui::SameLine();
                                    }






                                    ImGui::PopStyleColor(3);
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
                            else if (currentCollision && (*currentCollision == CollisionType::COLL_FULL_SEMISOLID))
                                currentIcon = editorAssets.util_tileset_geticon(CollisionIcons::UTIL_FULL_SEMISOL_COL_ICO );
                            else if (currentCollision && (*currentCollision == CollisionType::COLL_GROUND_HAZARD))
                                currentIcon = editorAssets.util_tileset_geticon(CollisionIcons::UTIL_GROUND_HAZARD_COL_ICO);
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

                        CollisionOption options[6] = {
                            {CollisionType::COLL_EMPTY,        CollisionIcons::UTIL_EMPTY_COL_ICO,   "Empty Collision"},
                            {CollisionType::COLL_FULL_SOLID,   CollisionIcons::UTIL_FULL_COL_ICO,    "Full Solid"},
                            {CollisionType::COLL_PSLOPE1_SOLID,CollisionIcons::UTIL_PSLOPE1_COL_ICO, "Positive Slope"},
                            {CollisionType::COLL_NSLOPE1_SOLID,CollisionIcons::UTIL_NSLOPE1_COL_ICO, "Negative Slope"},
                            {CollisionType::COLL_FULL_SEMISOLID ,CollisionIcons::UTIL_FULL_SEMISOL_COL_ICO, "Semi-Solid Collision"},
                            {CollisionType::COLL_GROUND_HAZARD ,CollisionIcons::UTIL_GROUND_HAZARD_COL_ICO, "Ground Hazard"}
                        };

                        ImVec2 buttonSize = ImVec2(28 * fullscreenScale.x, 28 * fullscreenScale.y);

                        for (int i = 0; i < 6; i++) {


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

                    const float paletteListHeight = std::max(90.0f, ImGui::GetContentRegionAvail().y - 12.0f);
                    ImGui::BeginChild("ItemListPanel", ImVec2(220 * fullscreenScale.x, paletteListHeight), true);


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

                    ImGui::SameLine();
                    ImGui::PushItemWidth(125 * fullscreenScale.x);
                    ImGui::SliderInt("Tiles / Page", &palette_tiles_per_page, 16, 1024);
                    ImGui::PopItemWidth();

                    ImGui::SameLine();
                    ImGui::PushItemWidth(105 * fullscreenScale.x);
                    ImGui::SliderInt("Page Cols", &palette_page_columns, 1, 32);
                    ImGui::PopItemWidth();

                    UIPos(1050, 563, 1050, 547);
                    ImGui::Text("________");

                    UIPos(1050, 558, 1050, 542);
                    ImGui::Text("Tile Settings:         ");



                    // Below the text...?


                    







                    bool selectedAtlasValid = (selectedAtlas >= 0 && selectedAtlas < (int)scene.loaded_atlases.size());

                    

                    // ----------------------------------->
                    // Draw Tile window
                    //

                    UIPos(240, 605, 240, 570);
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f,0.2f,0.2f,0.7f));
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0,1)); // black
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f); // thickness

                    const ImVec2 paletteAvail = ImGui::GetContentRegionAvail();
                    ImGui::BeginChild("TileGrid", ImVec2(800.0f * fullscreenScale.x, std::max(90.0f, paletteAvail.y)), true, ImGuiWindowFlags_NoScrollbar);

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
                        std::vector<int> palette_tile_indices;
                        palette_tile_indices.reserve(total_cols * total_rows);
                        for (int idx = 0; idx < total_cols * total_rows; idx++) {
                            if (scene.loaded_atlases[selectedAtlas].tile_data[idx].anim_parent_index == -1) {
                                palette_tile_indices.push_back(idx);
                            }
                        }

                        palette_tiles_per_page = std::clamp(palette_tiles_per_page, 16, 1024);
                        palette_page_columns = std::clamp(palette_page_columns, 1, 32);
                        const int palette_total_tiles = (int)palette_tile_indices.size();
                        const int palette_total_pages = std::max(1, (palette_total_tiles + palette_tiles_per_page - 1) / palette_tiles_per_page);
                        palette_page_index = std::clamp(palette_page_index, 0, palette_total_pages - 1);

                        if (ImGui::Button("<##palette_page_prev") && palette_page_index > 0) {
                            palette_page_index--;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button(">##palette_page_next") && palette_page_index + 1 < palette_total_pages) {
                            palette_page_index++;
                        }
                        ImGui::SameLine();
                        ImGui::Text("Page %d / %d (%d tiles)", palette_page_index + 1, palette_total_pages, palette_total_tiles);
                        ImGui::Separator();

                        const int palette_page_start = palette_page_index * palette_tiles_per_page;
                        const int palette_page_end = std::min(palette_total_tiles, palette_page_start + palette_tiles_per_page);

                        int page_tile_counter = 0;
                        for (int i = palette_page_start; i < palette_page_end; i++) {
                                const int tile_index = palette_tile_indices[i];
                                const int row = tile_index / total_cols;
                                const int col = tile_index % total_cols;

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

                                page_tile_counter++;
                                if ((page_tile_counter % palette_page_columns) != 0 && (i + 1) < palette_page_end) {
                                    ImGui::SameLine();
                                }






                                ImGui::PopStyleColor(3);
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



                    


                    //ImGui::ImageButton( (void*)(intptr_t)index, textureID, ImVec2(32, 32), uv0, uv1);

                    // EMPTY
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Background")) {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.11f, 0.13f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.14f, 0.15f, 0.18f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.25f, 0.30f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.34f, 0.42f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.18f, 0.21f, 0.27f, 1.0f));
                    scene.EDITOR_ONLY_BACKGROUND_TAB_SELECTED = true;
                    scene.EDITOR_ONLY_ACTIVE_BACKGROUND_EDITOR = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

                    std::vector<std::string> background_paths = assets.GetFilepathsInDirectory(BACKGROUNDIMAGEDIR, ".png");
                    std::vector<std::string> jpg_paths = assets.GetFilepathsInDirectory(BACKGROUNDIMAGEDIR, ".jpg");
                    std::vector<std::string> jpeg_paths = assets.GetFilepathsInDirectory(BACKGROUNDIMAGEDIR, ".jpeg");
                    background_paths.insert(background_paths.end(), jpg_paths.begin(), jpg_paths.end());
                    background_paths.insert(background_paths.end(), jpeg_paths.begin(), jpeg_paths.end());
                    std::sort(background_paths.begin(), background_paths.end());
                    static int selectedBackgroundSourceIndex = -1;

                    if ((int)scene.background.layers.size() <= 0) {
                        selectedBackgroundLayer = -1;
                    } else if (selectedBackgroundLayer >= (int)scene.background.layers.size()) {
                        selectedBackgroundLayer = (int)scene.background.layers.size() - 1;
                    }

                    const float actionsWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.25f;
                    if (ImGui::Button("+ Layer", ImVec2(actionsWidth, 0))) {
                        scene.background.new_layer(0.0f, 0.0f, 1.0f + (float)scene.background.layers.size(), WHITE);
                        selectedBackgroundLayer = (int)scene.background.layers.size() - 1;
                        selectedBackgroundNode = -1;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("- Layer", ImVec2(actionsWidth, 0))) {
                        if (selectedBackgroundLayer >= 0) {
                            scene.background.remove_layer(selectedBackgroundLayer, assets);
                            if (scene.background.layers.empty()) {
                                selectedBackgroundLayer = -1;
                            } else if (selectedBackgroundLayer >= (int)scene.background.layers.size()) {
                                selectedBackgroundLayer = (int)scene.background.layers.size() - 1;
                            }
                            selectedBackgroundNode = -1;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("^ Layer", ImVec2(actionsWidth, 0))) {
                        if (selectedBackgroundLayer > 0) {
                            if (scene.background.move_layer(selectedBackgroundLayer, selectedBackgroundLayer - 1)) {
                                selectedBackgroundLayer--;
                                selectedBackgroundNode = -1;
                                backgroundStatusIsError = false;
                                backgroundStatusMessage = "Moved layer up.";
                            }
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("v Layer", ImVec2(actionsWidth, 0))) {
                        if (selectedBackgroundLayer >= 0 && selectedBackgroundLayer + 1 < (int)scene.background.layers.size()) {
                            if (scene.background.move_layer(selectedBackgroundLayer, selectedBackgroundLayer + 1)) {
                                selectedBackgroundLayer++;
                                selectedBackgroundNode = -1;
                                backgroundStatusIsError = false;
                                backgroundStatusMessage = "Moved layer down.";
                            }
                        }
                    }
                    

                    ImGui::Separator();
                    ImGui::TextUnformatted("Static backdrop image");
                    ImGui::TextDisabled("Image folder: %s", BACKGROUNDIMAGEDIR.c_str());
                    ImGui::TextDisabled("Parallax nodes are rendered into %dx%d slots (non-matching images will be stretched).", gwconst::SCREEN_WIDTH_GAMEPIXELS, gwconst::SCREEN_HEIGHT_GAMEPIXELS);
                    if (ImGui::BeginListBox("##BackdropImageList", ImVec2(-1, 90.0f * fullscreenScale.y))) {
                        for (int i = 0; i < (int)background_paths.size(); i++) {
                            const bool selected = selectedBackgroundSourceIndex == i;
                            if (ImGui::Selectable(background_paths[i].c_str(), selected)) {
                                selectedBackgroundSourceIndex = i;
                            }
                        }
                        ImGui::EndListBox();
                    }
                    if (ImGui::Button("Set Backdrop") && selectedBackgroundSourceIndex >= 0 && selectedBackgroundSourceIndex < (int)background_paths.size()) {
                        const bool set_ok = scene.background.set_backdrop_image(assets, background_paths[(size_t)selectedBackgroundSourceIndex]);
                        backgroundStatusIsError = !set_ok;
                        backgroundStatusMessage = set_ok ? "Backdrop set." : "Failed to set backdrop image.";
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Clear Backdrop")) {
                        scene.background.clear_backdrop_image(assets);
                        backgroundStatusIsError = false;
                        backgroundStatusMessage = "Backdrop cleared.";
                    }
                    if (background_paths.empty()) {
                        ImGui::TextDisabled("No images found. Drop .png/.jpg files in assets/sprites/backgrounds/.");
                    }

                    ImGui::Separator();
                    ImGui::TextUnformatted("Background Layers");
                    if (ImGui::BeginListBox("##BackgroundLayerList", ImVec2(-1, 110.0f * fullscreenScale.y))) {
                        for (int i = 0; i < (int)scene.background.layers.size(); i++) {
                            std::string label = "Layer " + std::to_string(i) + " (z=" + std::to_string(scene.background.layers[(size_t)i].z_dist_offset) + ")";
                            if (ImGui::Selectable(label.c_str(), selectedBackgroundLayer == i)) {
                                selectedBackgroundLayer = i;
                                selectedBackgroundNode = -1;
                            }
                        }
                        ImGui::EndListBox();
                    }

                    if (selectedBackgroundLayer >= 0 && selectedBackgroundLayer < (int)scene.background.layers.size()) {
                        ParallaxLayer & selectedLayerRef = scene.background.layers[(size_t)selectedBackgroundLayer];

                        ImGui::DragFloat("Layer X Offset", &selectedLayerRef.x_dist_offset, 1.0f);
                        ImGui::DragFloat("Layer Y Offset", &selectedLayerRef.y_dist_offset, 1.0f);
                        ImGui::DragFloat("Layer Z Depth", &selectedLayerRef.z_dist_offset, 0.01f, 0.01f, 100.0f);
                        float tintColor[4] = {
                            selectedLayerRef.tint.r / 255.0f,
                            selectedLayerRef.tint.g / 255.0f,
                            selectedLayerRef.tint.b / 255.0f,
                            selectedLayerRef.tint.a / 255.0f
                        };
                        if (ImGui::ColorEdit4("Layer Tint", tintColor)) {
                            selectedLayerRef.tint = {
                                (unsigned char)(std::clamp(tintColor[0], 0.0f, 1.0f) * 255.0f),
                                (unsigned char)(std::clamp(tintColor[1], 0.0f, 1.0f) * 255.0f),
                                (unsigned char)(std::clamp(tintColor[2], 0.0f, 1.0f) * 255.0f),
                                (unsigned char)(std::clamp(tintColor[3], 0.0f, 1.0f) * 255.0f)
                            };
                        }

                        if (ImGui::Button("Reset Tint")) {
                            selectedLayerRef.tint = WHITE;
                        }


                        ImGui::Checkbox("Parallax Painter Mode", &backgroundPainterMode);
                        if (backgroundPainterMode) {
                            ImGui::TextDisabled("Left click in viewport places selected image at hovered screen-tile seat.");
                        }

                        ImGui::DragInt("Node Seat X", &backgroundSeatX, 1.0f, Background::ALLOWED_NODE_SPACE_X_MIN, Background::ALLOWED_NODE_SPACE_X_MAX);
                        ImGui::DragInt("Node Seat Y", &backgroundSeatY, 1.0f, Background::ALLOWED_NODE_SPACE_Y_MIN, Background::ALLOWED_NODE_SPACE_Y_MAX);
                        if (ImGui::Button("Add/Replace Node At Seat")) {
                            if (selectedBackgroundSourceIndex >= 0 && selectedBackgroundSourceIndex < (int)background_paths.size()) {
                                const bool add_ok = scene.background.new_parallax(selectedBackgroundLayer, assets, background_paths[(size_t)selectedBackgroundSourceIndex], backgroundSeatX, backgroundSeatY, false, false);
                                backgroundStatusIsError = !add_ok;
                                backgroundStatusMessage = add_ok ? "Node added/updated." : "Failed to add node (seat or image invalid).";
                            } else {
                                backgroundStatusIsError = true;
                                backgroundStatusMessage = "Select a source image first.";
                            }
                        }

                        if (ImGui::BeginListBox("##BackgroundNodeList", ImVec2(-1, 90.0f * fullscreenScale.y))) {
                            for (int i = 0; i < (int)selectedLayerRef.nodes.size(); i++) {
                                const ParallaxNode & node = selectedLayerRef.nodes[(size_t)i];
                                std::string label = "(" + std::to_string(node.seat_x) + "," + std::to_string(node.seat_y) + ") xRep=" + (node.x_repeating ? "1" : "0") + " yRep=" + (node.y_repeating ? "1" : "0");
                                if (ImGui::Selectable(label.c_str(), selectedBackgroundNode == i)) {
                                    selectedBackgroundNode = i;
                                }
                            }
                            ImGui::EndListBox();
                        }

                        if (selectedBackgroundNode >= 0 && selectedBackgroundNode < (int)selectedLayerRef.nodes.size()) {
                            ParallaxNode & node = selectedLayerRef.nodes[(size_t)selectedBackgroundNode];
                            bool xrep = node.x_repeating;
                            bool yrep = node.y_repeating;

                            if (ImGui::Checkbox("Repeat X", &xrep)) {
                                scene.background.set_node_repeat_x(selectedBackgroundLayer, selectedBackgroundNode, xrep);
                            }
                            if (ImGui::Checkbox("Repeat Y", &yrep)) {
                                scene.background.set_node_repeat_y(selectedBackgroundLayer, selectedBackgroundNode, yrep);
                            }

                            if (ImGui::Button("Delete Selected Node")) {
                                scene.background.remove_parallax(selectedBackgroundLayer, selectedBackgroundNode, assets);
                                selectedBackgroundNode = -1;
                                backgroundStatusIsError = false;
                                backgroundStatusMessage = "Node deleted.";
                            }
                        }

                        if (backgroundPainterMode && selectedBackgroundSourceIndex >= 0 && selectedBackgroundSourceIndex < (int)background_paths.size()) {
                            Vec2 cam = renderer.get_camera_position();
                            float zoom = renderer.get_camera_zoom();
                            Vec2 mouse = winstats::ScreenMousePosition();

                            float depth = std::max(0.01f, selectedLayerRef.z_dist_offset);
                            float factor = 1.0f / depth;
                            float parallax_cam_x = (cam.x * factor) + selectedLayerRef.x_dist_offset;
                            float parallax_cam_y = (cam.y * factor) + selectedLayerRef.y_dist_offset;
                            float world_left = parallax_cam_x - ((config::GAME_WORLD_WIDTH * 0.5f) / zoom);
                            float world_top = parallax_cam_y - ((config::GAME_WORLD_HEIGHT * 0.5f) / zoom);
                            int seat_x = (int)std::floor((world_left + (mouse.x / zoom)) / gwconst::SCREEN_WIDTH_GAMEPIXELS);
                            int seat_y = (int)std::floor((world_top + (mouse.y / zoom)) / gwconst::SCREEN_HEIGHT_GAMEPIXELS);

                            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !scene.uiCapturesMouse) {
                                const bool add_ok = scene.background.new_parallax(selectedBackgroundLayer, assets, background_paths[(size_t)selectedBackgroundSourceIndex], seat_x, seat_y, false, false);
                                backgroundStatusIsError = !add_ok;
                                backgroundStatusMessage = add_ok ? "Node painted." : "Failed to paint node.";
                            }

                            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && !scene.uiCapturesMouse) {
                                ParallaxLayer & painterLayer = scene.background.layers[(size_t)selectedBackgroundLayer];
                                for (int i = 0; i < (int)painterLayer.nodes.size(); i++) {
                                    if (painterLayer.nodes[(size_t)i].seat_x == seat_x && painterLayer.nodes[(size_t)i].seat_y == seat_y) {
                                        scene.background.remove_parallax(selectedBackgroundLayer, i, assets);
                                         backgroundStatusIsError = false;
                                        backgroundStatusMessage = "Node erased.";
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    if (!backgroundStatusMessage.empty()) {
                        const ImVec4 statusColor = backgroundStatusIsError ? ImVec4(0.95f, 0.35f, 0.35f, 1.0f) : ImVec4(0.35f, 0.95f, 0.45f, 1.0f);
                        ImGui::TextColored(statusColor, "%s", backgroundStatusMessage.c_str());
                    }

                    ImGui::PopStyleColor(5);
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Spawndefs")) {
                    // EMPTY
                    ImGui::EndTabItem();
                }


                if (ImGui::BeginTabItem("Clamps")) {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.10f, 0.12f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.16f, 0.14f, 0.18f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.32f, 0.24f, 0.22f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.40f, 0.30f, 0.27f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.27f, 0.20f, 0.18f, 1.0f));

                    if (scene.active_clamps.empty()) {
                        selectedClamp = -1;
                    } else if (selectedClamp >= (int)scene.active_clamps.size()) {
                        selectedClamp = (int)scene.active_clamps.size() - 1;
                    } else if (selectedClamp < 0) {
                        selectedClamp = 0;
                    }


                    const float clampActionButtonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

                    if (ImGui::Button("+ New Clamp", ImVec2(clampActionButtonWidth, 0))) {

                        Vec2 camera_position = {0.0f, 0.0f};

                        for (Entity camera : registry.view<comp::Camera>()) {

                            camera_position = registry.get_component<comp::Transform>(camera).position;
                            break;
                        }

                        
                        scene.active_clamps.push_back({camera_position.x-200, camera_position.y-100, camera_position.x+200, camera_position.y+100, camera_position.x-100, camera_position.y-50, camera_position.x+100, camera_position.y+50, 8.0f, false});
                        selectedClamp = (int)scene.active_clamps.size() - 1;

                    }

                    ImGui::SameLine();
                    if (ImGui::Button("- Delete Clamp", ImVec2(clampActionButtonWidth, 0))) {

                        if (selectedClamp >= 0 && selectedClamp < (int)scene.active_clamps.size()) {
                            scene.active_clamps.erase(scene.active_clamps.begin() + selectedClamp);

                            if (scene.active_clamps.empty()) {
                                selectedClamp = -1;
                            } else if (selectedClamp >= (int)scene.active_clamps.size()) {
                                selectedClamp = (int)scene.active_clamps.size() - 1;
                            }
                        }
                    }

                    ImGui::Separator();

                    ImGui::TextUnformatted("Camera Clamps");
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));
                    if (ImGui::BeginListBox("##CameraClampsList", ImVec2(-1, 128.0f * fullscreenScale.y))) {
                        for (int i = 0; i < (int)scene.active_clamps.size(); i++) {
                            std::string clamp_label = "Clamp " + std::to_string(i + 1);
                            if (ImGui::Selectable(clamp_label.c_str(), selectedClamp == i)) {
                                selectedClamp = i;
                            }
                        }
                        ImGui::EndListBox();
                    }
                    ImGui::PopStyleVar();

                    if (selectedClamp >= 0 && selectedClamp < (int)scene.active_clamps.size()) {
                        CameraClamp & clamp = scene.active_clamps[selectedClamp];

                        ImGui::Text("Selected: Clamp %d", selectedClamp + 1);
                        ImGui::TextDisabled("Edit values live while running.");
                        ImGui::Separator();

                        ImGui::Text("Clamp Zone (camera)");
                        ImGui::PushItemWidth(-1);
                        ImGui::InputFloat2("Clamp Top Left", &clamp.clamp_top_left.x);
                        ImGui::InputFloat2("Clamp Bottom Right", &clamp.clamp_bottom_right.x);
                        ImGui::PopItemWidth();

                        if (clamp.clamp_top_left.x > clamp.clamp_bottom_right.x) {
                            std::swap(clamp.clamp_top_left.x, clamp.clamp_bottom_right.x);
                        }
                        if (clamp.clamp_top_left.y > clamp.clamp_bottom_right.y) {
                            std::swap(clamp.clamp_top_left.y, clamp.clamp_bottom_right.y);
                        }

                        ImGui::Spacing();
                        ImGui::Text("Player Trigger Zone");
                        ImGui::PushItemWidth(-1);
                        ImGui::InputFloat2("Player Top Left", &clamp.player_zone_top_left.x);
                        ImGui::InputFloat2("Player Bottom Right", &clamp.player_zone_bottom_right.x);
                        ImGui::PopItemWidth();

                        if (clamp.player_zone_top_left.x > clamp.player_zone_bottom_right.x) {
                            std::swap(clamp.player_zone_top_left.x, clamp.player_zone_bottom_right.x);
                        }
                        if (clamp.player_zone_top_left.y > clamp.player_zone_bottom_right.y) {
                            std::swap(clamp.player_zone_top_left.y, clamp.player_zone_bottom_right.y);
                        }

                        ImGui::Spacing();
                        ImGui::DragFloat("Smoothing Override", &clamp.smoothing_override, 0.0001f, 0.0001f, 0.0300f, "%.4f");
                        ImGui::Checkbox("Snap To Clamp", &clamp.snap_to_clamp);
                    } else {
                        ImGui::TextDisabled("No clamps in scene.");
                        
                    }

                    ImGui::PopStyleColor(5);

                    ImGui::EndTabItem();   
                }


                ImGui::EndTabBar();

                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(6);
            }


            
        }


        scene.uiCapturesMouse = ImGui::GetIO().WantCaptureMouse;

        ImGui::End();

    } else {

        ImVec2 btnSize = ImVec2(120 * fullscreenScale.x, 28 * fullscreenScale.y);
        ImVec2 btnPos  = ImVec2( GetScreenWidth()  - btnSize.x - 10.0f, GetScreenHeight() - btnSize.y - 10.0f);
        ImGui::SetNextWindowPos(btnPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(btnSize, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.35f);   // transparency

        ImGui::Begin("##showworkspace", nullptr,
            ImGuiWindowFlags_NoTitleBar     |
            ImGuiWindowFlags_NoResize       |
            ImGuiWindowFlags_NoMove         |
            ImGuiWindowFlags_NoScrollbar    |
            ImGuiWindowFlags_NoSavedSettings
        );

        if (ImGui::Button("Show Workspace", btnSize)) {
            WORKSPACE_WINDOW_DRAW = true;
        }

        ImGui::End();

    }



}
