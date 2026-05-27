#include "Toolbar.h"
#include "../core/SelectionManager.h"
#include <imgui.h>
#include <cmath>

namespace materializr {

Toolbar::Toolbar() = default;

void Toolbar::setSelectionManager(const SelectionManager* sel) {
    m_selection = sel;
}

ToolAction Toolbar::render() {
    ToolAction action = ToolAction::None;

    ImGui::Begin("Tools");

    if (m_sketchMode) {
        action = renderSketchTools();
    } else if (!m_selection || !m_selection->hasSelection()) {
        action = renderNoSelectionTools();
    } else if (m_selection->hasSelectedSketchRegions()) {
        action = renderSketchRegionTools();
    } else if (m_selection->hasSelectedSketches()) {
        action = renderSketchSelectedTools();
    } else if (m_selection->hasSelectedFaces()) {
        action = renderFaceTools();
        if (action == ToolAction::None) {
            action = renderBodyTools();
        }
    } else if (m_selection->hasSelectedBodies()) {
        action = renderBodyTools();
    } else if (m_selection->hasSelectedEdges()) {
        action = renderEdgeTools();
    } else {
        action = renderNoSelectionTools();
    }

    ImGui::End();
    return action;
}

void Toolbar::setSketchMode(bool active) {
    m_sketchMode = active;
}

bool Toolbar::isSketchMode() const {
    return m_sketchMode;
}

ToolAction Toolbar::renderSketchTools() {
    ToolAction action = ToolAction::None;

    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Sketch Tools");
    ImGui::Separator();

    // Grid step selector — drives both the visual face grid and snap-to-line behaviour
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Grid:");
    const float steps[] = { 0.1f, 0.5f, 1.0f, 10.0f };
    const char* labels[] = { "0.1", "0.5", "1", "10" };
    for (int i = 0; i < 4; ++i) {
        if (i > 0) ImGui::SameLine();
        bool selected = std::abs(m_gridStep - steps[i]) < 1e-6f;
        if (selected) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.45f, 0.85f, 1.0f));
        if (ImGui::Button(labels[i], ImVec2(34, 24))) m_gridStep = steps[i];
        if (selected) ImGui::PopStyleColor();
    }
    ImGui::Separator();

    if (ImGui::Button("Line", ImVec2(-1, 30)))
        action = ToolAction::Line;
    if (ImGui::Button("Circle", ImVec2(-1, 30)))
        action = ToolAction::Circle;
    if (ImGui::Button("Rectangle", ImVec2(-1, 30)))
        action = ToolAction::Rectangle;
    if (ImGui::Button("Arc", ImVec2(-1, 30)))
        action = ToolAction::Arc;
    if (ImGui::Button("Spline", ImVec2(-1, 30)))
        action = ToolAction::Spline;
    if (ImGui::Button("Polygon", ImVec2(-1, 30)))
        action = ToolAction::Polygon;
    if (ImGui::Button("Trim", ImVec2(-1, 30)))
        action = ToolAction::Trim;
    if (!m_cameraOrtho) {
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.55f, 0.85f, 1.0f));
        if (ImGui::Button("Look at Sketch", ImVec2(-1, 30)))
            action = ToolAction::LookAtSketch;
        ImGui::PopStyleColor();
    }

    ImGui::Separator();
    if (ImGui::Button("Finish Sketch", ImVec2(-1, 30)))
        action = ToolAction::FinishSketch;

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "General");
    ImGui::Separator();

    if (ImGui::Button("Measure", ImVec2(-1, 30)))
        action = ToolAction::Measure;
    if (ImGui::Button("Reset Camera", ImVec2(-1, 30)))
        action = ToolAction::ResetCamera;

    return action;
}

ToolAction Toolbar::renderNoSelectionTools() {
    ToolAction action = ToolAction::None;

    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Create");
    ImGui::Separator();

    if (ImGui::Button("Start Sketch", ImVec2(-1, 30)))
        action = ToolAction::StartSketch;
    if (ImGui::Button("Import", ImVec2(-1, 30)))
        action = ToolAction::Import;
    if (ImGui::Button("Construction Plane", ImVec2(-1, 30)))
        action = ToolAction::ConstructionPlane;

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Advanced");
    ImGui::Separator();

    if (ImGui::Button("Sweep", ImVec2(-1, 30)))
        action = ToolAction::Sweep;
    if (ImGui::Button("Loft", ImVec2(-1, 30)))
        action = ToolAction::Loft;
    if (ImGui::Button("Split Body", ImVec2(-1, 30)))
        action = ToolAction::SplitBody;

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "General");
    ImGui::Separator();

    if (ImGui::Button("Measure", ImVec2(-1, 30)))
        action = ToolAction::Measure;
    if (ImGui::Button("Reset Camera", ImVec2(-1, 30)))
        action = ToolAction::ResetCamera;

    return action;
}

ToolAction Toolbar::renderBodyTools() {
    ToolAction action = ToolAction::None;

    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Transform");
    ImGui::Separator();

    if (ImGui::Button("Move", ImVec2(-1, 30)))
        action = ToolAction::Move;
    if (ImGui::Button("Rotate", ImVec2(-1, 30)))
        action = ToolAction::Rotate;
    if (ImGui::Button("Mirror", ImVec2(-1, 30)))
        action = ToolAction::Mirror;
    if (ImGui::Button("Scale", ImVec2(-1, 30)))
        action = ToolAction::Scale;

    // Shared snap-to-grid controls for the gizmo translate (uses the sketch grid step).
    ImGui::Checkbox("Snap to grid", &m_snapToGrid);
    const float gridSteps[] = { 0.1f, 0.5f, 1.0f, 10.0f };
    const char* gridLabels[] = { "0.1", "0.5", "1", "10" };
    for (int i = 0; i < 4; ++i) {
        if (i > 0) ImGui::SameLine();
        bool selected = std::abs(m_gridStep - gridSteps[i]) < 1e-6f;
        if (selected) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.45f, 0.85f, 1.0f));
        ImGui::PushID(i);
        if (ImGui::SmallButton(gridLabels[i])) m_gridStep = gridSteps[i];
        ImGui::PopID();
        if (selected) ImGui::PopStyleColor();
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Boolean");
    ImGui::Separator();

    bool multipleSelected = m_selection && m_selection->selectedBodyCount() >= 2;

    ImGui::BeginDisabled(!multipleSelected);
    if (ImGui::Button("Union", ImVec2(-1, 30)))
        action = ToolAction::BoolUnion;
    if (ImGui::Button("Subtract", ImVec2(-1, 30)))
        action = ToolAction::BoolSubtract;
    if (ImGui::Button("Intersect", ImVec2(-1, 30)))
        action = ToolAction::BoolIntersect;
    ImGui::EndDisabled();

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Modify");
    ImGui::Separator();

    if (ImGui::Button("Shell", ImVec2(-1, 30)))
        action = ToolAction::Shell;

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Pattern");
    ImGui::Separator();

    if (ImGui::Button("Linear Pattern", ImVec2(-1, 30)))
        action = ToolAction::LinearPattern;
    if (ImGui::Button("Radial Pattern", ImVec2(-1, 30)))
        action = ToolAction::RadialPattern;

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "General");
    ImGui::Separator();

    if (ImGui::Button("Measure", ImVec2(-1, 30)))
        action = ToolAction::Measure;
    if (ImGui::Button("Reset Camera", ImVec2(-1, 30)))
        action = ToolAction::ResetCamera;

    return action;
}

ToolAction Toolbar::renderFaceTools() {
    ToolAction action = ToolAction::None;

    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Face Operations");
    ImGui::Separator();

    if (ImGui::Button("Sketch on Face", ImVec2(-1, 30)))
        action = ToolAction::SketchOnFace;
    if (ImGui::Button("Push / Pull", ImVec2(-1, 30)))
        action = ToolAction::PushPull;
    if (ImGui::Button("Extrude", ImVec2(-1, 30)))
        action = ToolAction::Extrude;
    if (ImGui::Button("Revolve", ImVec2(-1, 30)))
        action = ToolAction::Revolve;
    if (ImGui::Button("Offset Face", ImVec2(-1, 30)))
        action = ToolAction::OffsetFace;
    if (ImGui::Button("Shell", ImVec2(-1, 30)))
        action = ToolAction::Shell;

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "General");
    ImGui::Separator();

    if (ImGui::Button("Measure", ImVec2(-1, 30)))
        action = ToolAction::Measure;
    if (ImGui::Button("Reset Camera", ImVec2(-1, 30)))
        action = ToolAction::ResetCamera;

    return action;
}

ToolAction Toolbar::renderSketchSelectedTools() {
    ToolAction action = ToolAction::None;

    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Sketch");
    ImGui::Separator();
    ImGui::TextWrapped("Tip: hover a sketch region to highlight it, click to select, Ctrl+click to add to selection.");
    ImGui::Separator();

    if (ImGui::Button("Edit Sketch", ImVec2(-1, 30)))
        action = ToolAction::EditSketch;
    if (ImGui::Button("Extrude Sketch", ImVec2(-1, 30)))
        action = ToolAction::ExtrudeSketch;
    if (ImGui::Button("Revolve", ImVec2(-1, 30)))
        action = ToolAction::Revolve;

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "General");
    ImGui::Separator();

    if (ImGui::Button("Measure", ImVec2(-1, 30)))
        action = ToolAction::Measure;
    if (ImGui::Button("Reset Camera", ImVec2(-1, 30)))
        action = ToolAction::ResetCamera;

    return action;
}

ToolAction Toolbar::renderSketchRegionTools() {
    ToolAction action = ToolAction::None;

    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Region");
    ImGui::Separator();
    int n = m_selection ? m_selection->selectedSketchRegionCount() : 0;
    ImGui::Text("%d region%s selected", n, n == 1 ? "" : "s");
    ImGui::Spacing();

    if (ImGui::Button("Push / Pull", ImVec2(-1, 30)))
        action = ToolAction::PushPull;

    ImGui::Spacing();
    ImGui::TextWrapped("Drag positive distance to extrude, negative to cut into the body the sketch sits on.");

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "General");
    ImGui::Separator();

    if (ImGui::Button("Measure", ImVec2(-1, 30)))
        action = ToolAction::Measure;
    if (ImGui::Button("Reset Camera", ImVec2(-1, 30)))
        action = ToolAction::ResetCamera;

    return action;
}

ToolAction Toolbar::renderEdgeTools() {
    ToolAction action = ToolAction::None;

    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Edge Operations");
    ImGui::Separator();

    if (ImGui::Button("Fillet", ImVec2(-1, 30)))
        action = ToolAction::Fillet;
    if (ImGui::Button("Chamfer", ImVec2(-1, 30)))
        action = ToolAction::Chamfer;

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "General");
    ImGui::Separator();

    if (ImGui::Button("Measure", ImVec2(-1, 30)))
        action = ToolAction::Measure;
    if (ImGui::Button("Reset Camera", ImVec2(-1, 30)))
        action = ToolAction::ResetCamera;

    return action;
}

} // namespace materializr
