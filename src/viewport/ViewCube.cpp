#include "ViewCube.h"
#include "Camera.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace materializr {

ViewCube::ViewCube() {}

// 3DSMax-style navigation overlay: a projected 3D cube (face clicks snap to
// orthographic views) wrapped in a horizon ring (drag to rotate the camera
// around its target). Drawn with ImDrawList so it shares ImGui's window state.

ViewCubeAction ViewCube::render(Camera& camera, bool invertDrag)
{
    ViewCubeAction action = ViewCubeAction::None;

    // --- Layout: top-right of the current window, leaving room for the title bar.
    ImVec2 wp = ImGui::GetWindowPos();
    ImVec2 ws = ImGui::GetWindowSize();
    const float pad   = 10.0f;
    const float cubeR = 38.0f;   // half-extent of cube projection (px)
    ImVec2 center(wp.x + ws.x - pad - cubeR - 26.0f,
                  wp.y + pad + cubeR + 42.0f);

    // --- Camera view-rotation matrix (no translation), so the cube spins with
    //     the camera's orientation.
    glm::mat4 V = camera.getViewMatrix();
    V[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    // Project local cube corners (±1 on each axis) to screen space using only
    // the rotation: x → screen.x, y → -screen.y (flip), z → depth.
    auto projectXY = [&](const glm::vec3& v) -> ImVec2 {
        glm::vec4 e = V * glm::vec4(v, 1.0f);
        return ImVec2(center.x + e.x * cubeR, center.y - e.y * cubeR);
    };
    auto eyeZ = [&](const glm::vec3& v) -> float {
        return (V * glm::vec4(v, 1.0f)).z;
    };

    static const glm::vec3 kCorners[8] = {
        {-1,-1,-1}, { 1,-1,-1}, { 1, 1,-1}, {-1, 1,-1},
        {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1}, {-1, 1, 1},
    };
    ImVec2 sc[8];
    for (int i = 0; i < 8; ++i) sc[i] = projectXY(kCorners[i]);

    // Face definitions. Corner order is CCW from OUTSIDE the cube, so a face's
    // 2D screen winding flips sign when the face turns away from the camera.
    struct Face { int c[4]; glm::vec3 n; const char* label; ViewCubeAction act; };
    static const Face kFaces[6] = {
        {{4,5,6,7}, { 0, 0, 1}, "Front", ViewCubeAction::Front},
        {{1,0,3,2}, { 0, 0,-1}, "Back",  ViewCubeAction::Back},
        {{0,4,7,3}, {-1, 0, 0}, "Left",  ViewCubeAction::Left},
        {{5,1,2,6}, { 1, 0, 0}, "Right", ViewCubeAction::Right},
        {{3,7,6,2}, { 0, 1, 0}, "Top",   ViewCubeAction::Top},
        {{0,1,5,4}, { 0,-1, 0}, "Bottom",ViewCubeAction::Bottom},
    };

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 mp = ImGui::GetMousePos();

    // --- Cube faces. Compute visibility per face from eye-space normal.z.
    auto pointInQuad = [&](const ImVec2* q, ImVec2 p) -> bool {
        float sign = 0.0f;
        for (int i = 0; i < 4; ++i) {
            ImVec2 a = q[i], b = q[(i+1) % 4];
            float c = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
            if (i == 0) sign = c;
            else if ((c > 0) != (sign > 0) && std::abs(c) > 1e-3f && std::abs(sign) > 1e-3f)
                return false;
        }
        return true;
    };

    bool cubeHover = false;
    // Two passes so back faces (depth-sorted) draw before front faces. Cheap:
    // sort by face-centroid eye Z descending (further first).
    struct VisFace { int idx; float depth; };
    std::vector<VisFace> drawList;
    drawList.reserve(6);
    for (int i = 0; i < 6; ++i) {
        glm::vec4 ne = V * glm::vec4(kFaces[i].n, 0.0f);
        if (ne.z > 0.0f) {
            glm::vec3 ctr3 = kFaces[i].n; // face center is the normal scaled to 1
            drawList.push_back({i, eyeZ(ctr3)});
        }
    }
    std::sort(drawList.begin(), drawList.end(),
              [](const VisFace& a, const VisFace& b){ return a.depth < b.depth; });

    for (auto& vf : drawList) {
        const Face& f = kFaces[vf.idx];
        ImVec2 q[4] = { sc[f.c[0]], sc[f.c[1]], sc[f.c[2]], sc[f.c[3]] };
        bool hover = pointInQuad(q, mp);
        if (hover) cubeHover = true;
        ImU32 fill = hover ? IM_COL32(80, 140, 220, 230) : IM_COL32(70, 75, 90, 215);
        dl->AddConvexPolyFilled(q, 4, fill);
        dl->AddPolyline(q, 4, IM_COL32(220, 220, 230, 255), ImDrawFlags_Closed, 1.4f);

        // Label centred on the face.
        ImVec2 ctr((q[0].x + q[1].x + q[2].x + q[3].x) * 0.25f,
                   (q[0].y + q[1].y + q[2].y + q[3].y) * 0.25f);
        ImVec2 ts = ImGui::CalcTextSize(f.label);
        dl->AddText(ImVec2(ctr.x - ts.x * 0.5f, ctr.y - ts.y * 0.5f),
                    IM_COL32(255, 255, 255, 255), f.label);

        // Press on a face arms a pending snap; the actual snap fires on RELEASE
        // (and only if the user didn't drag in between, so dragging the cube
        // produces a free orbit instead).
        if (hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_pendingClick = f.act;
            m_cubeDragging = false;
        }
    }

    // --- Corner click-spots: a small dot at each visible vertex snaps to the
    //     matching isometric view. We test corners after faces so a face hover
    //     wins when they overlap (corner spots sit inside the face polygons).
    static const ViewCubeAction kCornerActions[8] = {
        ViewCubeAction::BackBottomLeft,    // 0: -X -Y -Z
        ViewCubeAction::BackBottomRight,   // 1: +X -Y -Z
        ViewCubeAction::BackTopRight,      // 2: +X +Y -Z
        ViewCubeAction::BackTopLeft,       // 3: -X +Y -Z
        ViewCubeAction::FrontBottomLeft,   // 4: -X -Y +Z
        ViewCubeAction::FrontBottomRight,  // 5: +X -Y +Z
        ViewCubeAction::FrontTopRight,     // 6: +X +Y +Z
        ViewCubeAction::FrontTopLeft       // 7: -X +Y +Z
    };
    for (int i = 0; i < 8; ++i) {
        if (eyeZ(kCorners[i]) < 0.0f) continue; // back-of-cube vertex
        ImVec2 cp = sc[i];
        float dist = std::sqrt((mp.x - cp.x) * (mp.x - cp.x) +
                               (mp.y - cp.y) * (mp.y - cp.y));
        bool hover = dist < 7.0f;
        ImU32 col = hover ? IM_COL32(255, 220, 80, 240) : IM_COL32(200, 200, 220, 200);
        dl->AddCircleFilled(cp, hover ? 6.0f : 4.0f, col);
        if (hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_pendingClick = kCornerActions[i];
            m_cubeDragging = false;
            cubeHover = true; // counts as cube interaction for hover-suppression
        }
        if (hover) cubeHover = true;
    }

    // --- Four 90° rotation arrows positioned just outside the cube on each
    //     cardinal side. Clicking emits RotateLeft/Right/Up/Down.
    {
        const float r = cubeR + 18.0f;          // arrow centre distance
        const float s = 6.0f;                   // arrow half-size
        struct Arrow { ImVec2 dir; ViewCubeAction act; };
        Arrow arrows[4] = {
            {{-1, 0}, ViewCubeAction::RotateLeft},
            {{ 1, 0}, ViewCubeAction::RotateRight},
            {{ 0,-1}, ViewCubeAction::RotateUp},
            {{ 0, 1}, ViewCubeAction::RotateDown}
        };
        for (auto& ar : arrows) {
            ImVec2 ac(center.x + ar.dir.x * r, center.y + ar.dir.y * r);
            // Triangle pointing in ar.dir.
            ImVec2 tip(ac.x + ar.dir.x * s, ac.y + ar.dir.y * s);
            // Two base corners perpendicular to dir.
            ImVec2 perp(-ar.dir.y, ar.dir.x);
            ImVec2 b1(ac.x - ar.dir.x * s + perp.x * s, ac.y - ar.dir.y * s + perp.y * s);
            ImVec2 b2(ac.x - ar.dir.x * s - perp.x * s, ac.y - ar.dir.y * s - perp.y * s);
            float dx = mp.x - ac.x, dy = mp.y - ac.y;
            bool hover = std::sqrt(dx * dx + dy * dy) < s + 3.0f;
            ImU32 col = hover ? IM_COL32(255, 220, 80, 255) : IM_COL32(200, 200, 220, 220);
            dl->AddTriangleFilled(tip, b1, b2, col);
            if (hover) {
                cubeHover = true;
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    action = ar.act;
                }
            }
        }
    }

    // --- Drag the cube body itself to free-orbit (yaw + pitch). Click without
    //     drag snaps to the pressed face.
    if (m_pendingClick != ViewCubeAction::None) {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 4.0f)) {
            m_cubeDragging = true;
            ImVec2 d = ImGui::GetIO().MouseDelta;
            // Drag scaling: ~0.5° per pixel — gentle enough for precise aiming
            // but covers full rotation in a short stroke. `invertDrag` flips
            // the orbit sign for users who prefer the opposite mapping.
            const float k = invertDrag ? 0.01f : -0.01f; // radians per pixel
            camera.rotateAroundTarget(d.x * k, d.y * k);
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            if (!m_cubeDragging) action = m_pendingClick; // commit the snap
            m_pendingClick = ViewCubeAction::None;
            m_cubeDragging = false;
        }
    }

    // Remember overall hover (including in-progress drags) so the viewport
    // suppresses its own click logic for everything the cube is handling.
    m_lastHovered = cubeHover || m_cubeDragging ||
                    (m_pendingClick != ViewCubeAction::None);
    return action;
}

} // namespace materializr
