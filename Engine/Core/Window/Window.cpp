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