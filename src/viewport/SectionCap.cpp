#include "SectionCap.h"

#include <BRepAlgoAPI_Common.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <Poly_Triangulation.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS_Face.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Vec.hxx>
#include <gp_Trsf.hxx>

namespace materializr {

bool computeSectionCap(const TopoDS_Shape& shape, const gp_Pln& cuttingPlane,
                       std::vector<float>& outPositions) {
    if (shape.IsNull()) return false;

    const size_t startSize = outPositions.size();

    try {
        // ShapeRenderer clips away the +normal half (discard dot>0), so the
        // solid it keeps is the -normal half-space. Intersect the body with
        // that half-space and fill the new planar faces sitting on the cut
        // plane, otherwise a solid body reads as a hollow shell.
        gp_Dir n = cuttingPlane.Axis().Direction();
        gp_Pnt loc = cuttingPlane.Location();
        const double L = 1.0e5; // plane extent; must bisect any body
        TopoDS_Face planeFace =
            BRepBuilderAPI_MakeFace(cuttingPlane, -L, L, -L, L).Face();
        gp_Pnt refPnt = loc.Translated(gp_Vec(n) * -1.0); // kept (solid) side
        TopoDS_Shape halfSpace =
            BRepPrimAPI_MakeHalfSpace(planeFace, refPnt).Solid();

        BRepAlgoAPI_Common common(shape, halfSpace);
        common.Build();
        if (!common.IsDone()) return false;
        const TopoDS_Shape& capped = common.Shape();
        if (capped.IsNull()) return false;

        BRepMesh_IncrementalMesh mesher(capped, 0.1);
        mesher.Perform();

        const double planeTol = 1.0e-3;
        for (TopExp_Explorer fexp(capped, TopAbs_FACE); fexp.More(); fexp.Next()) {
            const TopoDS_Face& face = TopoDS::Face(fexp.Current());
            BRepAdaptor_Surface surf(face);
            if (surf.GetType() != GeomAbs_Plane) continue;
            gp_Pln fp = surf.Plane();
            // Cap faces lie ON the cut plane: parallel normal + point on plane.
            if (!fp.Axis().Direction().IsParallel(n, 0.01)) continue;
            if (cuttingPlane.Distance(fp.Location()) > planeTol) continue;

            TopLoc_Location tloc;
            Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, tloc);
            if (tri.IsNull()) continue;
            const gp_Trsf& trsf = tloc.Transformation();

            for (int t = 1; t <= tri->NbTriangles(); ++t) {
                int a = 0, b = 0, c = 0;
                tri->Triangle(t).Get(a, b, c);
                const gp_Pnt verts[3] = {
                    tri->Node(a).Transformed(trsf),
                    tri->Node(b).Transformed(trsf),
                    tri->Node(c).Transformed(trsf)};
                for (const gp_Pnt& p : verts) {
                    outPositions.push_back(static_cast<float>(p.X()));
                    outPositions.push_back(static_cast<float>(p.Y()));
                    outPositions.push_back(static_cast<float>(p.Z()));
                }
            }
        }
    } catch (...) {
        return outPositions.size() > startSize;
    }

    return outPositions.size() > startSize;
}

} // namespace materializr
