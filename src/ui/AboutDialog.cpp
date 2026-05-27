#include "AboutDialog.h"
#include <imgui.h>

namespace materializr {

AboutDialog::AboutDialog() = default;

void AboutDialog::setVisible(bool vis) {
    m_visible = vis;
}

bool AboutDialog::isVisible() const {
    return m_visible;
}

void AboutDialog::render() {
    if (!m_visible) return;

    ImGui::OpenPopup("About Materializr");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(360, 240), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("About Materializr", &m_visible,
                                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {

        // App name - large text
        ImGui::PushFont(nullptr); // Use default font; scaling below simulates larger text
        float origScale = ImGui::GetFont()->Scale;
        ImGui::GetFont()->Scale = 2.0f;
        ImGui::PushFont(ImGui::GetFont());
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Materializr").x) * 0.5f);
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Materializr");
        ImGui::GetFont()->Scale = origScale;
        ImGui::PopFont();
        ImGui::PopFont();

        // Version
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Version 0.1.0").x) * 0.5f);
        ImGui::Text("Version 0.1.0");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Description
        const char* desc = "Open-source parametric 3D CAD";
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(desc).x) * 0.5f);
        ImGui::Text("%s", desc);

        ImGui::Spacing();

        // Built with
        const char* builtWith = "Built with OpenCASCADE, Dear ImGui, GLFW, GLM";
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(builtWith).x) * 0.5f);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", builtWith);

        ImGui::Spacing();

        // License
        const char* license = "License: MIT";
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(license).x) * 0.5f);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", license);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Close button centered
        float buttonWidth = 100.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth) * 0.5f);
        if (ImGui::Button("Close", ImVec2(buttonWidth, 0))) {
            m_visible = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

} // namespace materializr
