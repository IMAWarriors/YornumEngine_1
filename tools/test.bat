@echo off
cd /d %~dp0..

set RAYLIB_PATH=C:\Raylib\
set COMPILER=g++

echo INFO: Project ^>^> Linking and compiling project files...

windres assets\icons\gicon_main.rc -O coff -o cache\icons\gicon_main.res

%COMPILER% ^
cache\icons\gicon_main.res ^
src\Main.cpp ^
src\Engine\Core\Main\Main_CoreApplication.cpp ^
src\Engine\Core\Window\Window.cpp ^
src\Engine\Core\Rendering\Renderer.cpp ^
src\Engine\Core\Rendering\Func_RendererPresentDef.cpp ^
src\Engine\Platform\Input\InputManager.cpp ^
src\Engine\ECS\EntityManager.cpp ^
src\Engine\ECS\Registry.cpp ^
src\Gamefiles\Assets\AssetManager.cpp ^
src\Gamefiles\Systems\CameraSystem.cpp ^
src\Gamefiles\Systems\RenderSystem.cpp ^
src\Gamefiles\Systems\InputSystem.cpp ^
src\Gamefiles\Systems\DebugOverlaySystem.cpp ^
src\Gamefiles\Systems\EngineManagerSystem.cpp ^
src\Gamefiles\Systems\PhysicsBodyMovementSystem.cpp ^
src\Gamefiles\World\SceneSerialization.cpp ^
src\Gamefiles\World\TileGrid.cpp ^
src\Gamefiles\Game\Func_GEAddEditorSystems.cpp ^
src\Gamefiles\Game\GameEngine.cpp ^
-o game.exe ^
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

game.exe

echo INFO: Project ^>^> Executable protocol finished!

pause

echo INFO: Project ^>^> Closing all dependencies...





