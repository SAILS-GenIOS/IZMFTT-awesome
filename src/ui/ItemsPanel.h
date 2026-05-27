#pragma once
#include <string>
#include <vector>

class Document;
class SelectionManager;
class History;

namespace materializr {

class ItemsPanel {
public:
    ItemsPanel();

    void setDocument(Document* doc);
    void setSelectionManager(SelectionManager* sel);
    void setHistory(History* hist);

    // Returns true if a body was deleted (caller must rebuild meshes)
    bool render();

private:
    Document* m_document = nullptr;
    SelectionManager* m_selection = nullptr;
    History* m_history = nullptr;
    int m_renamingId = -1;
    char m_renameBuffer[128] = {};
    bool m_showBodies = true;
    bool m_showSketches = true;
    bool m_showPlanes = true;
    bool m_bodyDeleted = false;
};

} // namespace materializr
