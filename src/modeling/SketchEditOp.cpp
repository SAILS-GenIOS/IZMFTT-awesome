#include "SketchEditOp.h"

namespace materializr {

SketchEditOp::SketchEditOp(std::shared_ptr<Sketch> liveSketch,
                           std::shared_ptr<Sketch> beforeSnapshot,
                           std::shared_ptr<Sketch> afterSnapshot)
    : m_target(std::move(liveSketch)),
      m_before(std::move(beforeSnapshot)),
      m_after(std::move(afterSnapshot)) {}

bool SketchEditOp::execute(Document& /*doc*/) {
    if (!m_target || !m_after) return false;
    *m_target = *m_after;
    return true;
}

bool SketchEditOp::undo(Document& /*doc*/) {
    if (!m_target || !m_before) return false;
    *m_target = *m_before;
    return true;
}

std::string SketchEditOp::description() const {
    if (!m_before || !m_after) return "Sketch edit";
    int delta = m_after->elementCount() - m_before->elementCount();
    if (delta > 0) return "Add sketch element";
    if (delta < 0) return "Remove sketch element";
    return "Modify sketch";
}

} // namespace materializr
