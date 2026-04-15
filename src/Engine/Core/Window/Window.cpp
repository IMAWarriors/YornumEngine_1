// Window.cpp

#include "Window.h"


void Window::ListenFullscreenToggle() {
    
    if (IsKeyPressed(KEY_F11)) {
        
        fullscreen = !fullscreen;

        int monitor = GetCurrentMonitor();
        int monitorWidth  = GetMonitorWidth(monitor);
        int monitorHeight = GetMonitorHeight(monitor);

        if (fullscreen) {
            // Remove borders
            SetWindowState(FLAG_WINDOW_UNDECORATED);

            // Resize to EXACT monitor resolution
            SetWindowSize(monitorWidth, monitorHeight);

            // Move window to monitor origin
            SetWindowPosition(0, 0);

        } else {
            // Restore borders
            ClearWindowState(FLAG_WINDOW_UNDECORATED);

            // Restore original window size
            SetWindowSize(
                config::INIT_WINDOW_DISPLAY_WIDTH,
                config::INIT_WINDOW_DISPLAY_HEIGHT
            );

            // Recenter
            SetWindowPosition(
                (monitorWidth  - config::INIT_WINDOW_DISPLAY_WIDTH)  / 2,
                (monitorHeight - config::INIT_WINDOW_DISPLAY_HEIGHT) / 2
            );

        }
    }
}

Window::Window (const char * title, bool edit) {

    // SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    fullscreen = false;

    if (edit) {
        editorflag = true;
    }

    SetConfigFlags(FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);

    InitWindow(config::INIT_WINDOW_DISPLAY_WIDTH, config::INIT_WINDOW_DISPLAY_HEIGHT, title);
    SetTargetFPS(config::GAME_WORLD_FPS);

    // =================================== Choose what ICON to load =======================================
    if (!editorflag) {
        Image icon = LoadImage("assets/icons/g1_mainicon.png");
        SetWindowIcon(icon);
        UnloadImage(icon);
    } else {
        Image icon = LoadImage("assets/icons/g1_mainicon_editor.png");
        SetWindowIcon(icon);
        UnloadImage(icon);
    }
    // ====================================================================================================


    // ******************* LOAD SHADERS HERE *****************************

    /*
    painter = LoadShader(0, "assets/shaders/painter.fs");

    painter_camera_pos_loc = GetShaderLocation(painter, "u_camera_pos");
    painter_resolution_loc = GetShaderLocation(painter, "u_resolution");
    painter_viewport_loc = GetShaderLocation(painter, "u_viewport");
    painter_zoom_loc = GetShaderLocation(painter, "u_zoom");
    painter_time_loc = GetShaderLocation(painter, "time");

    painter_ready = painter.id > 0;

    if (!painter_ready) {
        TraceLog(LOG_WARNING, "Painter shader unavailable; running without post-process shader.");
    } else {
        if (painter_camera_pos_loc < 0 || painter_resolution_loc < 0 || painter_viewport_loc < 0 || painter_zoom_loc < 0 || painter_time_loc < 0) {
            TraceLog(LOG_WARNING, "Painter shader loaded with missing optional uniforms; effect will run with partial data.");
        }
    }
    */

    


    // *******************************************************************


    int monitor = GetCurrentMonitor();
    int monitor_width  = GetMonitorWidth(monitor);
    int monitor_height = GetMonitorHeight(monitor);

    SetWindowPosition(
        ((monitor_width  - config::INIT_WINDOW_DISPLAY_WIDTH)  / 2), 
        ((monitor_height - config::INIT_WINDOW_DISPLAY_HEIGHT) / 2)  
    );


}

void Window::EndProgram () {

    CloseWindow();
    
}

bool Window::IsRunning () const {

    return !WindowShouldClose();

}