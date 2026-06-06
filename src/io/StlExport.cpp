#include "StlExport.h"
#include "../core/Document.h"

#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <StlAPI_Writer.hxx>
#include <gp_Trsf.hxx>
#include <gp_Ax1.hxx>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Face.hxx>
#include <TopExp_Explorer.hxx>
#include <Poly_Triangulation.hxx>
#include <TopLoc_Location.hxx>

namespace materializr {

static int countTriangles(const TopoDS_Shape& shape) {
    int count = 0;
    for (TopExp_Explorer explorer(shape, TopAbs_FACE); explorer.More(); explorer.Next()) {
        const TopoDS_Face& face = TopoDS::Face(explorer.Current());
        TopLoc_Location location;
        Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);
        if (!triangulation.IsNull()) {
            count += triangulation->NbTriangles();
        }
    }
    return count;
}

StlExportResult StlExport::exportFile(const std::string& filePath, const Document& doc,
                                       const StlExportOptions& options) {
    StlExportResult result;

    std::vector<int> allIds = doc.getAllBodyIds();
    if (allIds.empty()) {
        result.errorMessage = "No bodies to export.";
        return result;
    }

    // Build a compound of all visible bodies
    BRep_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);

    int bodyCount = 0;
    for (int id : allIds) {
        if (doc.isBodyVisible(id)) {
            const TopoDS_Shape& shape = doc.getBody(id);
            if (!shape.IsNull()) {
                builder.Add(compound, shape);
                ++bodyCount;
            }
        }
    }

    if (bodyCount == 0) {
        result.errorMessage = "No visible bodies to export.";
        return result;
    }

    return exportShape(filePath, compound, options);
}

StlExportResult StlExport::exportShape(const std::string& filePath, const TopoDS_Shape& inShape,
                                        const StlExportOptions& options) {
    StlExportResult result;

    if (inShape.IsNull()) {
        result.errorMessage = "Cannot export a null shape.";
        return result;
    }

    // Y-up scene → Z-up file, same proper rotation StepIO::exportBodies
    // applies (a bare Y/Z swap would MIRROR the part — inside-out STLs).
    // Slicers and other CAD expect Z-up; without this, exported parts lie
    // on their side on the print bed.
    gp_Trsf yUpToZUp;
    yUpToZUp.SetRotation(gp_Ax1(gp_Pnt(0.0, 0.0, 0.0), gp_Dir(1.0, 0.0, 0.0)),
                         M_PI * 0.5);
    TopoDS_Shape shape = BRepBuilderAPI_Transform(inShape, yUpToZUp,
                                                  Standard_True).Shape();

    // Tessellate the shape
    BRepMesh_IncrementalMesh mesh(shape, options.linearDeflection, Standard_False,
                                  options.angularDeflection);
    mesh.Perform();

    if (!mesh.IsDone()) {
        result.errorMessage = "Tessellation failed.";
        return result;
    }

    // Write STL
    StlAPI_Writer writer;
    writer.ASCIIMode() = !options.binary;

    if (!writer.Write(shape, filePath.c_str())) {
        result.errorMessage = "Failed to write STL file: " + filePath;
        return result;
    }

    // Count triangles
    result.triangleCount = countTriangles(shape);
    result.success = true;
    return result;
}

} // namespace materializr
