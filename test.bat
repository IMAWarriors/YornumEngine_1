@echo off

set RAYLIB_PATH=C:\Raylib\
set COMPILER=g++

echo INFO: Project ^>^> Linking and compiling project files...

%COMPILER% ^
main.cpp ^
Engine\Window\Window.cpp ^
Engine\Platform\Input\InputManager.cpp ^
Engine\ECS\EntityManager.cpp ^
Engine\ECS\Registry.cpp ^
Engine\Rendering\Renderer.cpp ^
Gamefiles\Systems\RenderSystem.cpp ^
Gamefiles\Systems\InputSystem.cpp ^
-o game.exe ^
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





