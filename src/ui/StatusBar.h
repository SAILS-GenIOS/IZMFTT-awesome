#pragma once
#include <string>

class Document;
class SelectionManager;

namespace materializr {

class StatusBar {
public:
    StatusBar();

    void setDocument(const Document* doc);
    void setSelectionManager(const SelectionManager* sel);
    void setCurrentTool(const std::string& tool);
    void setSketchMode(bool active);
    void setMessage(const std::string& msg); // transient status message

    void render();

private:
    const Document* m_document = nullptr;
    const SelectionManager* m_selection = nullptr;
    std::string m_currentTool = "Select";
    bool m_sketchMode = false;
    std::string m_message;
};

} // namespace materializr
