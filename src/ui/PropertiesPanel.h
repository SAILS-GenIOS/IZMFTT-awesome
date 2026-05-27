#pragma once

class History;
class Document;
class SelectionManager;

namespace materializr {

class PropertiesPanel {
public:
    PropertiesPanel();

    void setHistory(History* history);
    void setDocument(Document* doc);
    void setSelectionManager(const SelectionManager* sel);

    // Set which history step is being edited (-1 for none)
    void setEditingStep(int step);
    int getEditingStep() const;

    // Render. Returns true if a parameter was changed (needs history replay).
    bool render();

private:
    History* m_history = nullptr;
    Document* m_document = nullptr;
    const SelectionManager* m_selection = nullptr;
    int m_editingStep = -1;
};

} // namespace materializr
