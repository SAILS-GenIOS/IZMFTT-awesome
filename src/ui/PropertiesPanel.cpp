#include "PropertiesPanel.h"
#include "../core/History.h"
#include "../core/Document.h"
#include "../core/SelectionManager.h"
#include "../core/Operation.h"
#include <imgui.h>
#include <cstdio>
#include <cstring>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

namespace materializr {

PropertiesPanel::PropertiesPanel() = default;

void PropertiesPanel::setHistory(History* history) {
    m_history = history;
}

void PropertiesPanel::setDocument(Document* doc) {
    m_document = doc;
}

void PropertiesPanel::setSelectionManager(const SelectionManager* sel) {
    m_selection = sel;
}

void PropertiesPanel::setEditingStep(int step) {
    m_editingStep = step;
}

int PropertiesPanel::getEditingStep() const {
    return m_editingStep;
}

bool PropertiesPanel::render() {
    bool modified = false;

    ImGui::Begin("Properties");

    // Case 1: Editing a history operation
    if (m_history && m_editingStep >= 0 && m_editingStep < m_history->stepCount()) {
        const Operation* op = m_history->getStep(m_editingStep);
        if (op) {
            // Operation header
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "%s", op->name().c_str());
            ImGui::Separator();

            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", op->description().c_str());
            ImGui::Spacing();

            // Render the operation's parameter controls
            const_cast<Operation*>(op)->renderProperties();

            ImGui::Spacing();
            ImGui::Separator();

            if (ImGui::Button("Apply Changes", ImVec2(-1, 0))) {
                if (m_document) {
                    m_history->editStep(m_editingStep, *m_document);
                    modified = true;
                }
            }

            ImGui::Spacing();

            // Enabled/disabled toggle
            bool enabled = op->isEnabled();
            if (ImGui::Checkbox("Enabled", &enabled)) {
                const_cast<Operation*>(op)->setEnabled(enabled);
                if (m_document) {
                    m_history->replayAll(*m_document);
                    modified = true;
                }
            }

            // Step info
            ImGui::Spacing();
            ImGui::Separator();
            char stepInfo[64];
            std::snprintf(stepInfo, sizeof(stepInfo), "Step %d of %d",
                          m_editingStep + 1, m_history->stepCount());
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", stepInfo);

            // Clear selection button
            if (ImGui::Button("Deselect", ImVec2(-1, 0))) {
                m_editingStep = -1;
            }
        }
    }
    // Case 2: A body is selected (but no operation being edited)
    else if (m_selection && m_selection->hasSelection() && m_document &&
             m_selection->primaryType() == SelectionType::Body) {
        const auto& sel = m_selection->getSelection();
        int bodyId = sel[0].bodyId;

        // Header
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Body Properties");
        ImGui::Separator();

        // Body name (editable)
        std::string bodyName = m_document->getBodyName(bodyId);
        static char nameBuffer[128];
        std::strncpy(nameBuffer, bodyName.c_str(), sizeof(nameBuffer) - 1);
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';

        ImGui::Text("Name:");
        ImGui::SameLine();
        if (ImGui::InputText("##BodyName", nameBuffer, sizeof(nameBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
            m_document->setBodyName(bodyId, nameBuffer);
        }

        // Body ID
        char idText[32];
        std::snprintf(idText, sizeof(idText), "ID: %d", bodyId);
        ImGui::Text("%s", idText);

        // Visibility toggle
        bool visible = m_document->isBodyVisible(bodyId);
        if (ImGui::Checkbox("Visible", &visible)) {
            m_document->setBodyVisible(bodyId, visible);
        }

        ImGui::Spacing();
        ImGui::Separator();

        // Bounding box dimensions
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Dimensions");

        const TopoDS_Shape& shape = m_document->getBody(bodyId);
        if (!shape.IsNull()) {
            Bnd_Box bbox;
            BRepBndLib::Add(shape, bbox);

            if (!bbox.IsVoid()) {
                Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
                bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

                double width  = xmax - xmin;
                double height = ymax - ymin;
                double depth  = zmax - zmin;

                char dimText[128];
                std::snprintf(dimText, sizeof(dimText), "%.2f x %.2f x %.2f",
                              width, height, depth);
                ImGui::Text("Size: %s", dimText);

                ImGui::Spacing();

                // Individual axis values
                std::snprintf(dimText, sizeof(dimText), "X: %.2f to %.2f (%.2f)",
                              xmin, xmax, width);
                ImGui::Text("%s", dimText);

                std::snprintf(dimText, sizeof(dimText), "Y: %.2f to %.2f (%.2f)",
                              ymin, ymax, height);
                ImGui::Text("%s", dimText);

                std::snprintf(dimText, sizeof(dimText), "Z: %.2f to %.2f (%.2f)",
                              zmin, zmax, depth);
                ImGui::Text("%s", dimText);
            } else {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Empty shape");
            }
        } else {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No shape data");
        }

        // If multiple bodies selected, show count
        if (m_selection->selectedBodyCount() > 1) {
            ImGui::Spacing();
            ImGui::Separator();
            char multiText[64];
            std::snprintf(multiText, sizeof(multiText), "%d bodies selected",
                          m_selection->selectedBodyCount());
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", multiText);
        }
    }
    // Case 3: Other selection types
    else if (m_selection && m_selection->hasSelection()) {
        const char* typeName = "Object";
        int count = static_cast<int>(m_selection->getSelection().size());

        switch (m_selection->primaryType()) {
            case SelectionType::Face:
                typeName = "Face";
                count = m_selection->selectedFaceCount();
                break;
            case SelectionType::Edge:
                typeName = "Edge";
                count = m_selection->selectedEdgeCount();
                break;
            case SelectionType::Vertex:
                typeName = "Vertex";
                break;
            case SelectionType::Sketch:
                typeName = "Sketch";
                break;
            case SelectionType::Plane:
                typeName = "Plane";
                break;
            default:
                break;
        }

        char selText[128];
        std::snprintf(selText, sizeof(selText), "%d %s(s) selected", count, typeName);
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "%s", selText);
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                           "Sub-shape properties not yet available.");
    }
    // Case 4: Nothing selected
    else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                           "Select an object or operation");
    }

    ImGui::End();
    return modified;
}

} // namespace materializr
