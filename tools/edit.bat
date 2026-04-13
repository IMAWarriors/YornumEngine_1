@echo off
cd /d %~dp0..

set RAYLIB_PATH=C:\Raylib\
set COMPILER=g++

echo INFO: Project ^>^> Linking and compiling project files...
echo INFO: Project ^>^> Configuring EDITOR build version...

windres assets\icons\gicon_editor.rc -O coff -o cache\icons\gicon_editor.res

%COMPILER% ^
cache\icons\gicon_editor.res ^
src\EMain.cpp ^
external\rlimgui\rlImGui.cpp ^
external\imgui\imgui.cpp ^
external\imgui\imgui_draw.cpp ^
external\imgui\imgui_widgets.cpp ^
external\imgui\imgui_tables.cpp ^
src\Engine\Core\Main\Editor_CoreApplication.cpp ^
src\Engine\Core\Window\Window.cpp ^
src\Engine\Core\Rendering\Renderer.cpp ^
src\Engine\Core\Rendering\Func_ERendererPresentDef.cpp ^
src\Engine\Platform\Input\InputManager.cpp ^
src\Engine\ECS\EntityManager.cpp ^
src\Engine\ECS\Registry.cpp ^
src\Gamefiles\Assets\AssetManager.cpp ^
src\Gamefiles\Systems\CameraSystem.cpp ^
src\Gamefiles\Systems\RenderSystem.cpp ^
src\Gamefiles\Systems\InputSystem.cpp ^
src\Gamefiles\Systems\DebugOverlaySystem.cpp ^
src\Gamefiles\Systems\CameraClampManagerSystem.cpp ^
src\Gamefiles\Systems\EngineManagerSystem.cpp ^
src\Gamefiles\Systems\EditorUISystem.cpp ^
src\Gamefiles\Systems\PhysicsBodyMovementSystem.cpp ^
src\Gamefiles\World\SceneSerialization.cpp ^
src\Gamefiles\World\TileGrid.cpp ^
src\Gamefiles\World\Background.cpp ^
src\Gamefiles\Game\Func_EGEAddEditorSystems.cpp ^
src\Gamefiles\Game\GameEngine.cpp ^
-Iexternal\imgui ^
-Iexternal\rlimgui ^
-o editor.exe ^
-mwindows ^
-static -static-libgcc -static-libstdc++ ^
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





