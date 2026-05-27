#pragma once
#include <string>
#include <vector>
#include <functional>

namespace materializr {

struct Command {
    std::string name;
    std::string shortcut;
    std::function<void()> action;
};

class CommandPalette {
public:
    CommandPalette();

    // Register a command
    void addCommand(const std::string& name, const std::string& shortcut,
                    std::function<void()> action);

    // Toggle open/close (bound to Ctrl+K)
    void toggle();
    bool isOpen() const;

    // Render the palette overlay. Call every frame.
    void render();

private:
    bool m_isOpen = false;
    char m_searchBuffer[256] = {};
    int m_selectedIndex = 0;
    std::vector<Command> m_commands;
    std::vector<int> m_filteredIndices;
    std::function<void()> m_pendingAction; // deferred until after ImGui scope exits

    void updateFilter();
    bool fuzzyMatch(const std::string& text, const std::string& pattern) const;
};

} // namespace materializr
