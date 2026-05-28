#include "SelectionManager.h"
#include "EventBus.h"
#include "Events.h"
#include <algorithm>

void SelectionManager::publishChanged() {
    if (m_eventBus) m_eventBus->publish(materializr::SelectionChangedEvent{});
}

void SelectionManager::clear() {
    m_selection.clear();
    publishChanged();
}

void SelectionManager::select(const SelectionEntry& entry) {
    m_selection.clear();
    m_selection.push_back(entry);
    publishChanged();
}

void SelectionManager::addToSelection(const SelectionEntry& entry) {
    if (findEntry(entry) < 0) {
        m_selection.push_back(entry);
        publishChanged();
    }
}

void SelectionManager::removeFromSelection(const SelectionEntry& entry) {
    int idx = findEntry(entry);
    if (idx >= 0) {
        m_selection.erase(m_selection.begin() + idx);
        publishChanged();
    }
}

void SelectionManager::toggleSelection(const SelectionEntry& entry) {
    int idx = findEntry(entry);
    if (idx >= 0) {
        m_selection.erase(m_selection.begin() + idx);
    } else {
        m_selection.push_back(entry);
    }
    publishChanged();
}

bool SelectionManager::hasSelection() const {
    return !m_selection.empty();
}

SelectionType SelectionManager::primaryType() const {
    if (m_selection.empty()) {
        return SelectionType::None;
    }
    return m_selection.front().type;
}

const std::vector<SelectionEntry>& SelectionManager::getSelection() const {
    return m_selection;
}

int SelectionManager::selectedBodyCount() const {
    int count = 0;
    for (const auto& entry : m_selection) {
        if (entry.type == SelectionType::Body) {
            count++;
        }
    }
    return count;
}

int SelectionManager::selectedFaceCount() const {
    int count = 0;
    for (const auto& entry : m_selection) {
        if (entry.type == SelectionType::Face) {
            count++;
        }
    }
    return count;
}

int SelectionManager::selectedEdgeCount() const {
    int count = 0;
    for (const auto& entry : m_selection) {
        if (entry.type == SelectionType::Edge) {
            count++;
        }
    }
    return count;
}

int SelectionManager::selectedSketchCount() const {
    int count = 0;
    for (const auto& entry : m_selection) {
        if (entry.type == SelectionType::Sketch) {
            count++;
        }
    }
    return count;
}

int SelectionManager::selectedSketchRegionCount() const {
    int count = 0;
    for (const auto& entry : m_selection) {
        if (entry.type == SelectionType::SketchRegion) {
            count++;
        }
    }
    return count;
}

bool SelectionManager::hasSelectedBodies() const {
    return selectedBodyCount() > 0;
}

bool SelectionManager::hasSelectedFaces() const {
    return selectedFaceCount() > 0;
}

bool SelectionManager::hasSelectedEdges() const {
    return selectedEdgeCount() > 0;
}

bool SelectionManager::hasSelectedSketches() const {
    return selectedSketchCount() > 0;
}

bool SelectionManager::hasSelectedSketchRegions() const {
    return selectedSketchRegionCount() > 0;
}

int SelectionManager::findEntry(const SelectionEntry& entry) const {
    for (int i = 0; i < static_cast<int>(m_selection.size()); ++i) {
        const auto& e = m_selection[i];
        if (e.type != entry.type || e.bodyId != entry.bodyId ||
            e.subShapeIndex != entry.subShapeIndex ||
            e.sketchId != entry.sketchId) {
            continue;
        }
        // Edge picks share subShapeIndex (-1) on the same body — and so would
        // any future selection type that doesn't carry a sub-shape index — so
        // also disambiguate by shape identity when both shapes are present.
        // Otherwise Ctrl+clicking a second edge on the same body would look
        // "already selected" and silently get dropped.
        if (!e.shape.IsNull() && !entry.shape.IsNull() &&
            !e.shape.IsSame(entry.shape)) {
            continue;
        }
        return i;
    }
    return -1;
}
