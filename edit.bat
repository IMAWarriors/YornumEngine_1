@echo off

set RAYLIB_PATH=C:\Raylib\
set COMPILER=g++

echo INFO: Project ^>^> Linking and compiling project files...
echo INFO: Project ^>^> Configuring EDITOR build version...

%COMPILER% ^
Tools\Main\Editor.cpp ^
External\rlimgui\rlImGui.cpp ^
External\imgui\imgui.cpp ^
External\imgui\imgui_draw.cpp ^
External\imgui\imgui_widgets.cpp ^
External\imgui\imgui_tables.cpp ^
Engine\Core\Main\Editor_CoreApplication.cpp ^
Engine\Core\Window\Window.cpp ^
Engine\Core\Rendering\Renderer.cpp ^
Engine\Core\Rendering\Func_ERendererPresentDef.cpp ^
Engine\Platform\Input\InputManager.cpp ^
Engine\ECS\EntityManager.cpp ^
Engine\ECS\Registry.cpp ^
Gamefiles\Assets\AssetManager.cpp ^
Gamefiles\Systems\CameraSystem.cpp ^
Gamefiles\Systems\RenderSystem.cpp ^
Gamefiles\Systems\InputSystem.cpp ^
Gamefiles\Systems\DebugOverlaySystem.cpp ^
Gamefiles\Systems\EngineManagerSystem.cpp ^
Gamefiles\Systems\EditorUISystem.cpp ^
Gamefiles\World\SceneSerialization.cpp ^
Gamefiles\World\TileGrid.cpp ^
Gamefiles\Game\Func_EGEAddEditorSystems.cpp ^
Gamefiles\Game\GameEngine.cpp ^
-IExternal\imgui ^
-IExternal\rlimgui ^
-o editor.exe ^
-mwindows ^
-I%RAYLIB_PATH%\include ^
-L%RAYLIB_PATH%\lib ^
-lraylib ^
-lopengl32 ^
-lgdi32 ^
-lwinmm ^
-lshell32

if %ERRORLEVEL% neq 0 (
    echo.
    echo Build FAILED.
    pause
    exit /b
)


echo INFO: Project ^>^> Files successfully compiled!
echo INFO: Project ^>^> Running main executable...

editor.exe

echo INFO: Project ^>^> Executable protocol finished!

pause

echo INFO: Project ^>^> Closing all dependencies...





