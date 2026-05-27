#pragma once

#include <imgui.h>
#include <glm/glm.hpp>

namespace materializr {

class Camera;

enum class ViewCubeAction {
    None, Front, Back, Left, Right, Top, Bottom,
    FrontTopRight, FrontTopLeft, BackTopRight, BackTopLeft,
    RotateLeft, RotateRight, RotateUp, RotateDown
};

class ViewCube {
public:
    ViewCube();

    // Render the view cube overlay. Call inside an ImGui window.
    // Returns the action if a face was clicked.
    ViewCubeAction render(Camera& camera);

private:
    void drawCubeFace(const char* label, ImVec2 pos, ImVec2 size, bool hovered);
    float m_size = 120.0f;
};

} // namespace materializr
