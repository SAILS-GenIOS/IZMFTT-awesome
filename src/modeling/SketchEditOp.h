#pragma once
#include "../core/Operation.h"
#include "../core/Document.h"
#include "Sketch.h"
#include <memory>

namespace materializr {

// Snapshot-based undo for sketch mutations (drawing elements, dimension input).
// Holds a shared_ptr to the live Sketch (same instance the Document and the
// active editing session share) plus before/after snapshots. Undo/redo swap
// the live sketch's contents in place, so the change is visible regardless of
// whether the sketch is in the document yet (a fresh sketch isn't added until
// exitSketchMode).
class SketchEditOp : public Operation {
public:
    SketchEditOp(std::shared_ptr<Sketch> liveSketch,
                 std::shared_ptr<Sketch> beforeSnapshot,
                 std::shared_ptr<Sketch> afterSnapshot);

    bool execute(Document& doc) override;
    bool undo(Document& doc) override;
    std::string name() const override { return "Sketch Edit"; }
    std::string description() const override;
    void renderProperties() override {}
    std::string typeId() const override { return "sketchedit"; }

private:
    std::shared_ptr<Sketch> m_target;
    std::shared_ptr<Sketch> m_before;
    std::shared_ptr<Sketch> m_after;
};

} // namespace materializr
