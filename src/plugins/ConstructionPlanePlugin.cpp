#include "../plugin/PluginMacro.h"
#include "../plugin/PluginContext.h"
#include "../core/Document.h"
#include "../core/History.h"
#include "../modeling/ConstructionPlaneOp.h"

REGISTER_PLUGIN(ConstructionPlane, [](materializr::PluginContext& ctx) {
    // Hand off to Application's popup (XY/XZ/YZ + offset + Parallel-to-face
    // when a planar face is in the selection, with live preview).
    auto action = [](materializr::PluginContext& ctx) {
        ctx.requestInteractiveOp("ConstructionPlane");
    };
    ctx.registerToolbarButton({"Construction Plane", "Create",
        materializr::SelectionContext::Always, 50,
        action, nullptr,
        "Open the Construction Plane popup. Pick XY / XZ / YZ or Parallel-to-"
        "Face (when a planar face is selected) with a live-previewed offset. "
        "The plane lands in history; sketch on it like any face."});

    ctx.registerCommand({"New Construction Plane", "", action});
})
