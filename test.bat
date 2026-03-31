@echo off

set RAYLIB_PATH=C:\Raylib\
set COMPILER=g++

echo INFO: Project ^>^> Linking and compiling project files...

%COMPILER% ^
main.cpp ^
Engine\Core\Main\Main_CoreApplication.cpp ^
Engine\Core\Window\Window.cpp ^
Engine\Core\Rendering\Renderer.cpp ^
Engine\Core\Rendering\Func_RendererPresentDef.cpp ^
Engine\Platform\Input\InputManager.cpp ^
Engine\ECS\EntityManager.cpp ^
Engine\ECS\Registry.cpp ^
Gamefiles\Assets\AssetManager.cpp ^
Gamefiles\Systems\CameraSystem.cpp ^
Gamefiles\Systems\RenderSystem.cpp ^
Gamefiles\Systems\InputSystem.cpp ^
Gamefiles\Systems\DebugOverlaySystem.cpp ^
Gamefiles\Systems\EngineManagerSystem.cpp ^
Gamefiles\World\SceneSerialization.cpp ^
Gamefiles\World\TileGrid.cpp ^
Gamefiles\Game\Func_GEAddEditorSystems.cpp ^
Gamefiles\Game\GameEngine.cpp ^
-o game.exe ^
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

game.exe

echo INFO: Project ^>^> Executable protocol finished!

pause

echo INFO: Project ^>^> Closing all dependencies...





