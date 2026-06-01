#include "../plugin/PluginMacro.h"
#include "../plugin/PluginContext.h"
#include "../core/Document.h"
#include "../core/History.h"
#include "../core/SelectionManager.h"
#include "../modeling/Sketch.h"
#include "../modeling/LoftOp.h"
#include <TopoDS_Wire.hxx>
#include <cstdio>
#include <memory>

// Two-profile sketch loft.
//
// Selection contract:
//   * 2+ sketches (or regions of 2 sketches) selected -> the action lofts
//                              between the first two, producing a new solid.
//   * 1 sketch / region selected -> the action stashes the request as the
//                              "LoftPickSecond" interactive-op so Application
//                              can render a hint popup telling the user to
//                              Ctrl-click a second sketch and click Loft again.
//
// We pull the outer wire of each sketch's first region — for the common case
// (one closed loop per profile sketch) that's exactly what BRepOffsetAPI_
// ThruSections needs.
//
// The button is registered under both HasSketches AND HasSketchRegions: when
// the user clicks inside a closed region the toolbar switches to the Region
// panel and HasSketches no longer matches, but SketchRegion entries carry
// their parent sketch id so we can route either selection through the same
// action.
REGISTER_PLUGIN(Loft, [](materializr::PluginContext& ctx) {
    auto action = [](materializr::PluginContext& ctx) {
        // Count distinct sketches in the selection — Application reads the
        // selection itself when it begins the loft so we don't have to ship
        // wires through the plugin context.
        const auto& sel = ctx.selection().getSelection();
        std::vector<int> sketchIds;
        auto add = [&](int id) {
            if (id < 0) return;
            for (int x : sketchIds) if (x == id) return;
            sketchIds.push_back(id);
        };
        for (const auto& e : sel) {
            if ((e.type == SelectionType::Sketch ||
                 e.type == SelectionType::SketchRegion) && e.sketchId >= 0) {
                add(e.sketchId);
            }
        }

        if (sketchIds.size() < 2) {
            ctx.requestInteractiveOp("LoftPickSecond");
            return;
        }

        // Hand off to Application, which opens the Loft popup (Solid/Shell,
        // Smooth/Ruled, Reverse-B-wire, live preview, Apply / Cancel) — same
        // architecture as Linear/Radial Pattern.
        ctx.requestInteractiveOp("Loft");
    };

    const char* tooltip =
        "Loft a solid between two sketch profiles. Select two sketches or "
        "regions (Ctrl-click to add a second) and click. With one selected, "
        "you'll be prompted to pick the second.\n\n"
        "Best results when the two profiles sit on PARALLEL planes with "
        "similar topology (both rectangles, both circles, etc.). Profiles on "
        "perpendicular planes or with very different vertex counts produce a "
        "tent / pyramid surface — that's the loft algorithm being honest, not "
        "a bug. For floor-to-vertical transitions, a Sweep along a guide curve "
        "is usually what you want instead.";

    ctx.registerToolbarButton({"Loft", "Loft",
        materializr::SelectionContext::HasSketches, 400,
        action, nullptr, tooltip});

    ctx.registerToolbarButton({"Loft", "Loft",
        materializr::SelectionContext::HasSketchRegions, 400,
        action, nullptr, tooltip});
})
