#pragma once
#include <functional>
#include <string>

class SelectionManager;

namespace materializr {

enum class ToolAction {
    None,
    // Sketch tools
    StartSketch, SketchOnFace, Line, Circle, Rectangle, Arc, Spline, Polygon, Trim,
    FinishSketch, EditSketch, ExtrudeSketch, PushPull, LookAtSketch,
    // 3D tools
    Extrude, Revolve, Sweep, Loft,
    Fillet, Chamfer, Shell, OffsetFace, SplitBody,
    // Boolean
    BoolUnion, BoolSubtract, BoolIntersect,
    // Transform
    Move, Rotate, Mirror, Scale,
    // Pattern
    LinearPattern, RadialPattern,
    // Other
    Import, Measure, ConstructionPlane, ResetCamera
};

class Toolbar {
public:
    Toolbar();

    // Set the selection manager to query current selection
    void setSelectionManager(const SelectionManager* sel);

    // Render the toolbar. Returns the action the user clicked (or None).
    ToolAction render();

    // Whether we're in sketch mode
    void setSketchMode(bool active);
    bool isSketchMode() const;

    // Sketch grid step (mm). Set by Application; toolbar lets user pick a new value.
    void setGridStep(float step) { m_gridStep = step; }
    float getGridStep() const { return m_gridStep; }

    // Whether the camera is currently in orthographic mode. Drives the "Look at
    // Sketch" button visibility (only shown when we're NOT ortho during a sketch).
    void setCameraOrtho(bool ortho) { m_cameraOrtho = ortho; }

    // Snap-to-grid toggle (shared with sketch + gizmo translate). Pumped from
    // Application each frame in both directions.
    void setSnapToGrid(bool snap) { m_snapToGrid = snap; }
    bool getSnapToGrid() const { return m_snapToGrid; }

private:
    const SelectionManager* m_selection = nullptr;
    bool m_sketchMode = false;
    float m_gridStep = 1.0f;
    bool m_cameraOrtho = true;
    bool m_snapToGrid = true;

    ToolAction renderSketchTools();
    ToolAction renderSketchSelectedTools();
    ToolAction renderSketchRegionTools();
    ToolAction renderNoSelectionTools();
    ToolAction renderBodyTools();
    ToolAction renderFaceTools();
    ToolAction renderEdgeTools();
};

} // namespace materializr
