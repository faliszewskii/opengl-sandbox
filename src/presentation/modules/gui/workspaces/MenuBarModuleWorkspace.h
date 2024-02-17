//
// Created by faliszewskii on 17.02.24.
//

#ifndef OPENGL_SANDBOX_MENUBARMODULEWORKSPACE_H
#define OPENGL_SANDBOX_MENUBARMODULEWORKSPACE_H

#include "../../../../logic/state/AppState.h"
#include "../MenuBarModule.h"
#include "ModulesWorkspaceHelpers.h"

namespace MenuBarWorkspace {
    inline void render(AppState &appState) {
        ModulesWorkspaceHelpers::renderModuleToggle(appState.menuBarModule->active);
    }
};

#endif //OPENGL_SANDBOX_MENUBARMODULEWORKSPACE_H
