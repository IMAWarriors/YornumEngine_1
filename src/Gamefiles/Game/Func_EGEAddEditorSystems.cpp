// Func_EGEAddEditorSystems.cpp


#include "GameEngine.h"

void GameEngine::AddEditorSystems() {

    systems.add_system<EditorUISystem>(Phases::EDITORUI, *renderer, scene, assets, editorAssets);
    
}