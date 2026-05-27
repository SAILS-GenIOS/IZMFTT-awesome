#include "CommandPalette.h"
#include <imgui.h>
#include <algorithm>
#include <cctype>
#include <cstring>

namespace materializr {

CommandPalette::CommandPalette() = default;

void CommandPalette::addCommand(const std::string& name, const std::string& shortcut,
                                std::function<void()> action) {
    m_commands.push_back({name, shortcut, std::move(action)});
}

void CommandPalette::toggle() {
    m_isOpen = !m_isOpen;
    if (m_isOpen) {
        // Reset state on open
        std::memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
        m_selectedIndex = 0;
        updateFilter();
    }
}

bool CommandPalette::isOpen() const {
    return m_isOpen;
}

void CommandPalette::render() {
    if (!m_isOpen) return;

    // Center the palette window
    ImGuiIO& io = ImGui::GetIO();
    float windowWidth = 500.0f;
    float windowX = (io.DisplaySize.x - windowWidth) * 0.5f;
    float windowY = io.DisplaySize.y * 0.2f; // 20% from top

    ImGui::SetNextWindowPos(ImVec2(windowX, windowY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(windowWidth, 0), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("##CommandPalette", &m_isOpen, flags);

    // Search input - auto-focus on first frame
    if (ImGui::IsWindowAppearing()) {
        ImGui::SetKeyboardFocusHere();
    }

    // No flags: CallbackAlways was previously set without supplying a callback
    // pointer, which crashed when ImGui tried to dispatch to it. Since we call
    // updateFilter() unconditionally below, no callback is needed.
    ImGui::InputText("##Search", m_searchBuffer, sizeof(m_searchBuffer));

    // Update filter whenever search text might have changed
    updateFilter();

    // Handle keyboard navigation
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        m_isOpen = false;
        ImGui::End();
        return;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        m_selectedIndex = std::max(0, m_selectedIndex - 1);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        m_selectedIndex = std::min(static_cast<int>(m_filteredIndices.size()) - 1,
                                   m_selectedIndex + 1);
    }

    // Execute on Enter
    if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
        if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_filteredIndices.size())) {
            int cmdIndex = m_filteredIndices[m_selectedIndex];
            // Close FIRST so the action runs after the palette window is gone.
            // This avoids any chance of the action mutating ImGui state while we're
            // still inside our Begin/End scope.
            m_isOpen = false;
            auto action = m_commands[cmdIndex].action; // copy
            ImGui::End();
            try { if (action) action(); } catch (...) {}
            return;
        }
    }

    // Command list
    ImGui::Separator();
    ImGui::BeginChild("##CommandList", ImVec2(0, 300), false);

    for (int i = 0; i < static_cast<int>(m_filteredIndices.size()); i++) {
        int cmdIndex = m_filteredIndices[i];
        const Command& cmd = m_commands[cmdIndex];

        ImGui::PushID(i);

        bool isSelected = (i == m_selectedIndex);
        if (ImGui::Selectable("##cmd", isSelected, 0, ImVec2(0, 24))) {
            // Defer the action until after we've closed our ImGui scope below.
            m_pendingAction = cmd.action;
            m_isOpen = false;
            ImGui::PopID();
            break;
        }

        // Draw command name and shortcut on the same line
        ImGui::SameLine(8.0f);
        ImGui::Text("%s", cmd.name.c_str());

        if (!cmd.shortcut.empty()) {
            // Right-align the shortcut
            float shortcutWidth = ImGui::CalcTextSize(cmd.shortcut.c_str()).x;
            float availWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SameLine(availWidth - shortcutWidth + 8.0f);
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", cmd.shortcut.c_str());
        }

        ImGui::PopID();
    }

    ImGui::EndChild();
    ImGui::End();

    // Run any deferred action OUTSIDE the palette's ImGui scope.
    if (m_pendingAction) {
        auto action = std::move(m_pendingAction);
        m_pendingAction = nullptr;
        try { action(); } catch (...) {}
    }
}

void CommandPalette::updateFilter() {
    m_filteredIndices.clear();

    std::string pattern(m_searchBuffer);

    if (pattern.empty()) {
        // Show all commands when no search text
        for (int i = 0; i < static_cast<int>(m_commands.size()); i++) {
            m_filteredIndices.push_back(i);
        }
    } else {
        for (int i = 0; i < static_cast<int>(m_commands.size()); i++) {
            if (fuzzyMatch(m_commands[i].name, pattern)) {
                m_filteredIndices.push_back(i);
            }
        }
    }

    // Clamp selected index
    if (m_filteredIndices.empty()) {
        m_selectedIndex = 0;
    } else if (m_selectedIndex >= static_cast<int>(m_filteredIndices.size())) {
        m_selectedIndex = static_cast<int>(m_filteredIndices.size()) - 1;
    }
}

bool CommandPalette::fuzzyMatch(const std::string& text, const std::string& pattern) const {
    // Check if all characters of pattern appear in order in text (case-insensitive)
    size_t patIdx = 0;
    size_t patLen = pattern.size();

    for (size_t i = 0; i < text.size() && patIdx < patLen; i++) {
        char tc = static_cast<char>(std::tolower(static_cast<unsigned char>(text[i])));
        char pc = static_cast<char>(std::tolower(static_cast<unsigned char>(pattern[patIdx])));
        if (tc == pc) {
            patIdx++;
        }
    }

    return patIdx == patLen;
}

} // namespace materializr
