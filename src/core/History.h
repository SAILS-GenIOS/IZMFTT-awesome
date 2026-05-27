#pragma once
#include <vector>
#include <memory>
#include "Operation.h"
#include "Document.h"

class History {
public:
    History();
    ~History() = default;

    // Add a new operation (executes it and pushes to stack)
    bool pushOperation(std::unique_ptr<Operation> op, Document& doc);

    // Add an operation whose effect is already applied to the document.
    // Used for ops where the live mutation happened externally (e.g. sketch
    // edits performed by the SketchTool); the op snapshots before/after so
    // undo/redo can swap between them without re-running the original action.
    void pushExecuted(std::unique_ptr<Operation> op);

    // Undo/Redo
    bool canUndo() const;
    bool canRedo() const;
    bool undo(Document& doc);
    bool redo(Document& doc);

    // History navigation
    int stepCount() const;
    int currentStep() const; // index of last executed step
    const Operation* getStep(int index) const;

    // Edit a historical step's parameters and replay
    bool editStep(int index, Document& doc);

    // Breakpoint: suppress all steps after this index
    void setBreakpoint(int index); // -1 = no breakpoint
    int getBreakpoint() const;

    // Replay: re-execute all enabled steps from scratch
    bool replayAll(Document& doc);

    // Clear history
    void clear();

    // Access operations for UI
    const std::vector<std::unique_ptr<Operation>>& operations() const;

private:
    std::vector<std::unique_ptr<Operation>> m_operations;
    int m_currentIndex = -1; // index of last executed operation (-1 = none)
    int m_breakpoint = -1;   // -1 = no breakpoint
};
