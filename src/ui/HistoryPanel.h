#pragma once

class History;
class Document;

namespace materializr {

class HistoryPanel {
public:
    HistoryPanel();

    void setHistory(History* history);
    void setDocument(Document* doc);

    // Render the panel. Returns true if history was modified (undo/redo/edit).
    bool render();

private:
    History* m_history = nullptr;
    Document* m_document = nullptr;
    int m_editingStep = -1;
    bool m_showProperties = false;
};

} // namespace materializr
