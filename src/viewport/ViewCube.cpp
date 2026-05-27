#include "ViewCube.h"
#include "Camera.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace materializr {

ViewCube::ViewCube() {}

ViewCubeAction ViewCube::render(Camera& camera)
{
    ViewCubeAction action = ViewCubeAction::None;

    // Position the view cube in the top-right corner of the current window
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    float padding = 10.0f;
    float btnSize = 32.0f;
    float gridWidth = btnSize * 3.0f + 4.0f;  // 3 columns + spacing
    float gridHeight = btnSize * 4.0f + 6.0f; // 4 rows + spacing

    float startX = windowPos.x + windowSize.x - gridWidth - padding;
    float startY = windowPos.y + padding + 20.0f; // offset for title bar

    ImGui::SetCursorScreenPos(ImVec2(startX, startY));

    // Use a child region so buttons are self-contained
    ImGui::BeginGroup();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.30f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.45f, 0.60f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.40f, 0.70f, 1.00f));

    // Row 1:        [Top]
    ImGui::SetCursorScreenPos(ImVec2(startX + btnSize + 2.0f, startY));
    if (ImGui::Button("Top", ImVec2(btnSize, btnSize))) {
        action = ViewCubeAction::Top;
    }

    // Row 2: [Left] [Front] [Right]
    float row2Y = startY + btnSize + 2.0f;
    ImGui::SetCursorScreenPos(ImVec2(startX, row2Y));
    if (ImGui::Button("L", ImVec2(btnSize, btnSize))) {
        action = ViewCubeAction::Left;
    }
    ImGui::SetCursorScreenPos(ImVec2(startX + btnSize + 2.0f, row2Y));
    if (ImGui::Button("F", ImVec2(btnSize, btnSize))) {
        action = ViewCubeAction::Front;
    }
    ImGui::SetCursorScreenPos(ImVec2(startX + (btnSize + 2.0f) * 2.0f, row2Y));
    if (ImGui::Button("R", ImVec2(btnSize, btnSize))) {
        action = ViewCubeAction::Right;
    }

    // Row 3:       [Bottom]
    float row3Y = startY + (btnSize + 2.0f) * 2.0f;
    ImGui::SetCursorScreenPos(ImVec2(startX + btnSize + 2.0f, row3Y));
    if (ImGui::Button("Bot", ImVec2(btnSize, btnSize))) {
        action = ViewCubeAction::Bottom;
    }

    // Row 4:       [Back]
    float row4Y = startY + (btnSize + 2.0f) * 3.0f;
    ImGui::SetCursorScreenPos(ImVec2(startX + btnSize + 2.0f, row4Y));
    if (ImGui::Button("Bk", ImVec2(btnSize, btnSize))) {
        action = ViewCubeAction::Back;
    }

    // Rotate buttons (15-degree orbit increments). Smaller, packed under the cube.
    float rotSize = btnSize * 0.8f;
    float rotPadY = 6.0f;
    float row5Y = startY + (btnSize + 2.0f) * 4.0f + rotPadY;
    // Layout:  [<]  [^]  [v]  [>]
    ImGui::SetCursorScreenPos(ImVec2(startX, row5Y));
    if (ImGui::Button("<##rotL", ImVec2(rotSize, rotSize))) {
        action = ViewCubeAction::RotateLeft;
    }
    ImGui::SetCursorScreenPos(ImVec2(startX + (rotSize + 2.0f), row5Y));
    if (ImGui::Button("^##rotU", ImVec2(rotSize, rotSize))) {
        action = ViewCubeAction::RotateUp;
    }
    ImGui::SetCursorScreenPos(ImVec2(startX + (rotSize + 2.0f) * 2.0f, row5Y));
    if (ImGui::Button("v##rotD", ImVec2(rotSize, rotSize))) {
        action = ViewCubeAction::RotateDown;
    }
    ImGui::SetCursorScreenPos(ImVec2(startX + (rotSize + 2.0f) * 3.0f, row5Y));
    if (ImGui::Button(">##rotR", ImVec2(rotSize, rotSize))) {
        action = ViewCubeAction::RotateRight;
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);

    ImGui::EndGroup();

    // Snap camera to the selected standard view
    if (action != ViewCubeAction::None) {
        // ViewCube actions are deliberate camera changes — leave the sketch ortho lock.
        camera.setOrthographic(false);

        float dist = glm::length(camera.getPosition() - camera.getTarget());
        glm::vec3 target = camera.getTarget();

        switch (action) {
            case ViewCubeAction::Front:
                camera.setPosition(target + glm::vec3(0, 0, dist));
                camera.setUp(glm::vec3(0, 1, 0));
                break;
            case ViewCubeAction::Back:
                camera.setPosition(target + glm::vec3(0, 0, -dist));
                camera.setUp(glm::vec3(0, 1, 0));
                break;
            case ViewCubeAction::Right:
                camera.setPosition(target + glm::vec3(dist, 0, 0));
                camera.setUp(glm::vec3(0, 1, 0));
                break;
            case ViewCubeAction::Left:
                camera.setPosition(target + glm::vec3(-dist, 0, 0));
                camera.setUp(glm::vec3(0, 1, 0));
                break;
            case ViewCubeAction::Top:
                camera.setPosition(target + glm::vec3(0, dist, 0));
                camera.setUp(glm::vec3(0, 0, -1));
                break;
            case ViewCubeAction::Bottom:
                camera.setPosition(target + glm::vec3(0, -dist, 0));
                camera.setUp(glm::vec3(0, 0, 1));
                break;
            case ViewCubeAction::FrontTopRight:
                camera.setPosition(target + glm::normalize(glm::vec3(1, 1, 1)) * dist);
                camera.setUp(glm::vec3(0, 1, 0));
                break;
            case ViewCubeAction::FrontTopLeft:
                camera.setPosition(target + glm::normalize(glm::vec3(-1, 1, 1)) * dist);
                camera.setUp(glm::vec3(0, 1, 0));
                break;
            case ViewCubeAction::BackTopRight:
                camera.setPosition(target + glm::normalize(glm::vec3(1, 1, -1)) * dist);
                camera.setUp(glm::vec3(0, 1, 0));
                break;
            case ViewCubeAction::BackTopLeft:
                camera.setPosition(target + glm::normalize(glm::vec3(-1, 1, -1)) * dist);
                camera.setUp(glm::vec3(0, 1, 0));
                break;
            case ViewCubeAction::RotateLeft:
                camera.rotateAroundTarget(static_cast<float>(M_PI) / 12.0f, 0.0f);
                break;
            case ViewCubeAction::RotateRight:
                camera.rotateAroundTarget(-static_cast<float>(M_PI) / 12.0f, 0.0f);
                break;
            case ViewCubeAction::RotateUp:
                camera.rotateAroundTarget(0.0f, static_cast<float>(M_PI) / 12.0f);
                break;
            case ViewCubeAction::RotateDown:
                camera.rotateAroundTarget(0.0f, -static_cast<float>(M_PI) / 12.0f);
                break;
            default:
                break;
        }
    }

    return action;
}

void ViewCube::drawCubeFace(const char* label, ImVec2 pos, ImVec2 size, bool hovered)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImU32 fillColor = hovered
        ? IM_COL32(90, 115, 150, 220)
        : IM_COL32(60, 65, 75, 200);
    ImU32 borderColor = IM_COL32(150, 160, 180, 255);
    ImU32 textColor = IM_COL32(220, 225, 235, 255);

    ImVec2 pMax(pos.x + size.x, pos.y + size.y);
    drawList->AddRectFilled(pos, pMax, fillColor, 2.0f);
    drawList->AddRect(pos, pMax, borderColor, 2.0f);

    // Center the label text
    ImVec2 textSize = ImGui::CalcTextSize(label);
    ImVec2 textPos(
        pos.x + (size.x - textSize.x) * 0.5f,
        pos.y + (size.y - textSize.y) * 0.5f
    );
    drawList->AddText(textPos, textColor, label);
}

} // namespace materializr
