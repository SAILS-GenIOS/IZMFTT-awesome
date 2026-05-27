#include "StepIO.h"
#include "../core/Document.h"

#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
#include <Interface_Static.hxx>
#include <IFSelect_ReturnStatus.hxx>

namespace materializr {

ImportResult StepIO::import(const std::string& filePath, Document& doc) {
    ImportResult result;

    STEPControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(filePath.c_str());

    if (status != IFSelect_RetDone) {
        result.errorMessage = "Failed to read STEP file: " + filePath;
        switch (status) {
            case IFSelect_RetError:
                result.errorMessage += " (read error)";
                break;
            case IFSelect_RetFail:
                result.errorMessage += " (read failure)";
                break;
            case IFSelect_RetVoid:
                result.errorMessage += " (file is empty)";
                break;
            default:
                result.errorMessage += " (unknown error)";
                break;
        }
        return result;
    }

    // Transfer all roots
    Standard_Integer nbRoots = reader.TransferRoots();
    if (nbRoots == 0) {
        result.errorMessage = "No shapes found in STEP file.";
        return result;
    }

    int importCount = 0;

    // Iterate over transferred shapes
    for (Standard_Integer i = 1; i <= reader.NbShapes(); ++i) {
        TopoDS_Shape shape = reader.Shape(i);

        // Explore for solids first
        bool foundSolids = false;
        for (TopExp_Explorer explorer(shape, TopAbs_SOLID); explorer.More(); explorer.Next()) {
            const TopoDS_Solid& solid = TopoDS::Solid(explorer.Current());
            ++importCount;
            std::string name = "Imported_" + std::to_string(importCount);
            doc.addBody(solid, name);
            foundSolids = true;
        }

        // If no solids, try shells
        if (!foundSolids) {
            bool foundShells = false;
            for (TopExp_Explorer explorer(shape, TopAbs_SHELL); explorer.More(); explorer.Next()) {
                const TopoDS_Shell& shell = TopoDS::Shell(explorer.Current());
                ++importCount;
                std::string name = "Imported_" + std::to_string(importCount);
                doc.addBody(shell, name);
                foundShells = true;
            }

            // If no shells, try faces
            if (!foundShells) {
                for (TopExp_Explorer explorer(shape, TopAbs_FACE); explorer.More(); explorer.Next()) {
                    const TopoDS_Face& face = TopoDS::Face(explorer.Current());
                    ++importCount;
                    std::string name = "Imported_" + std::to_string(importCount);
                    doc.addBody(face, name);
                }
            }
        }
    }

    if (importCount == 0) {
        // If no specific sub-shapes found, add the top-level shapes directly
        for (Standard_Integer i = 1; i <= reader.NbShapes(); ++i) {
            TopoDS_Shape shape = reader.Shape(i);
            if (!shape.IsNull()) {
                ++importCount;
                std::string name = "Imported_" + std::to_string(importCount);
                doc.addBody(shape, name);
            }
        }
    }

    result.success = true;
    result.bodiesImported = importCount;
    return result;
}

ExportResult StepIO::exportFile(const std::string& filePath, const Document& doc) {
    ExportResult result;

    std::vector<int> allIds = doc.getAllBodyIds();
    if (allIds.empty()) {
        result.errorMessage = "No bodies to export.";
        return result;
    }

    // Collect only visible body IDs
    std::vector<int> visibleIds;
    for (int id : allIds) {
        if (doc.isBodyVisible(id)) {
            visibleIds.push_back(id);
        }
    }

    if (visibleIds.empty()) {
        result.errorMessage = "No visible bodies to export.";
        return result;
    }

    return exportBodies(filePath, doc, visibleIds);
}

ExportResult StepIO::exportBodies(const std::string& filePath, const Document& doc,
                                   const std::vector<int>& bodyIds) {
    ExportResult result;

    if (bodyIds.empty()) {
        result.errorMessage = "No body IDs specified for export.";
        return result;
    }

    STEPControl_Writer writer;

    // Use AP214 schema for broad compatibility
    Interface_Static::SetCVal("write.step.schema", "AP214");

    for (int id : bodyIds) {
        try {
            const TopoDS_Shape& shape = doc.getBody(id);
            if (shape.IsNull()) {
                continue;
            }
            IFSelect_ReturnStatus status = writer.Transfer(shape, STEPControl_AsIs);
            if (status != IFSelect_RetDone) {
                result.errorMessage = "Failed to transfer body ID " + std::to_string(id) +
                                      " to STEP writer.";
                return result;
            }
        } catch (const std::exception& e) {
            result.errorMessage = "Error accessing body ID " + std::to_string(id) +
                                  ": " + e.what();
            return result;
        }
    }

    IFSelect_ReturnStatus writeStatus = writer.Write(filePath.c_str());
    if (writeStatus != IFSelect_RetDone) {
        result.errorMessage = "Failed to write STEP file: " + filePath;
        return result;
    }

    result.success = true;
    return result;
}

} // namespace materializr
