#include "RenderSystem.h"

RenderSystem::RenderSystem() {
    renderFPS.clear();
}

double RenderSystem::GetRenderedFPS() {
    return renderFPS.getFPS();
}
