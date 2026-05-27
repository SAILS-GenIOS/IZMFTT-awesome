#pragma once
#include <string>
#include <TopoDS_Shape.hxx>

class Document;

namespace materializr {

struct StlExportOptions {
    double linearDeflection = 0.1;   // Tessellation accuracy (smaller = more triangles)
    double angularDeflection = 0.5;  // In radians
    bool binary = true;              // Binary STL (smaller, faster) vs ASCII
};

struct StlExportResult {
    bool success = false;
    std::string errorMessage;
    int triangleCount = 0;
};

class StlExport {
public:
    // Export all visible bodies to a single STL file
    static StlExportResult exportFile(const std::string& filePath, const Document& doc,
                                       const StlExportOptions& options = {});

    // Export a single shape
    static StlExportResult exportShape(const std::string& filePath, const TopoDS_Shape& shape,
                                        const StlExportOptions& options = {});
};

} // namespace materializr
