#include "ProjectIO.h"
#include "../core/Document.h"

#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS_Shape.hxx>

#include <fstream>
#include <sstream>
#include <vector>

namespace materializr {

ProjectSaveResult ProjectIO::save(const std::string& filePath, const Document& doc) {
    ProjectSaveResult result;

    std::ofstream ofs(filePath, std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) {
        result.errorMessage = "Failed to open file for writing: " + filePath;
        return result;
    }

    std::vector<int> bodyIds = doc.getAllBodyIds();

    ofs << "MATERIALIZR_PROJECT v1\n";
    ofs << "BODY_COUNT " << static_cast<int>(bodyIds.size()) << "\n";

    for (int id : bodyIds) {
        std::string name = doc.getBodyName(id);
        bool visible = doc.isBodyVisible(id);

        ofs << "BODY_START " << id << " \"" << name << "\" " << (visible ? 1 : 0) << "\n";

        try {
            const TopoDS_Shape& shape = doc.getBody(id);
            std::ostringstream brepStream;
            BRepTools::Write(shape, brepStream);
            ofs << brepStream.str();
        } catch (const std::exception& e) {
            result.errorMessage = "Failed to write BREP data for body " +
                                  std::to_string(id) + ": " + e.what();
            return result;
        }

        ofs << "\nBODY_END\n";
    }

    ofs << "END\n";

    if (!ofs.good()) {
        result.errorMessage = "I/O error while writing file: " + filePath;
        return result;
    }

    ofs.close();
    result.success = true;
    return result;
}

ProjectLoadResult ProjectIO::load(const std::string& filePath, Document& doc) {
    ProjectLoadResult result;

    std::ifstream ifs(filePath, std::ios::in);
    if (!ifs.is_open()) {
        result.errorMessage = "Failed to open file for reading: " + filePath;
        return result;
    }

    // Read and verify header
    std::string headerLine;
    if (!std::getline(ifs, headerLine) || headerLine != "MATERIALIZR_PROJECT v1") {
        result.errorMessage = "Invalid project file header. Expected 'MATERIALIZR_PROJECT v1'.";
        return result;
    }

    // Read body count
    std::string countLine;
    if (!std::getline(ifs, countLine)) {
        result.errorMessage = "Unexpected end of file reading body count.";
        return result;
    }

    int bodyCount = 0;
    {
        std::istringstream iss(countLine);
        std::string label;
        iss >> label >> bodyCount;
        if (label != "BODY_COUNT" || iss.fail()) {
            result.errorMessage = "Invalid body count line: " + countLine;
            return result;
        }
    }

    // Clear the document before loading
    doc.clear();

    BRep_Builder builder;
    int loadedCount = 0;

    for (int i = 0; i < bodyCount; ++i) {
        // Read BODY_START line
        std::string startLine;
        if (!std::getline(ifs, startLine)) {
            result.errorMessage = "Unexpected end of file reading body " + std::to_string(i + 1);
            return result;
        }

        // Parse: BODY_START id "name" visible
        int bodyId = 0;
        std::string bodyName;
        int visible = 1;

        {
            std::istringstream iss(startLine);
            std::string label;
            iss >> label;
            if (label != "BODY_START") {
                result.errorMessage = "Expected BODY_START, got: " + label;
                return result;
            }
            iss >> bodyId;

            // Parse quoted name
            std::string rest;
            std::getline(iss, rest);
            // Find the first and last quotes
            auto firstQuote = rest.find('"');
            auto lastQuote = rest.rfind('"');
            if (firstQuote != std::string::npos && lastQuote != std::string::npos &&
                firstQuote != lastQuote) {
                bodyName = rest.substr(firstQuote + 1, lastQuote - firstQuote - 1);
                // Parse visible flag after the closing quote
                std::string afterName = rest.substr(lastQuote + 1);
                std::istringstream afterIss(afterName);
                afterIss >> visible;
            } else {
                bodyName = "Body " + std::to_string(bodyId);
            }
        }

        // Read BREP data until BODY_END
        std::ostringstream brepData;
        std::string line;
        bool foundEnd = false;
        while (std::getline(ifs, line)) {
            if (line == "BODY_END") {
                foundEnd = true;
                break;
            }
            brepData << line << "\n";
        }

        if (!foundEnd) {
            result.errorMessage = "Missing BODY_END marker for body " + std::to_string(bodyId);
            return result;
        }

        // Reconstruct shape from BREP data
        TopoDS_Shape shape;
        std::istringstream brepStream(brepData.str());
        BRepTools::Read(shape, brepStream, builder);

        if (shape.IsNull()) {
            result.errorMessage = "Failed to read BREP data for body " + std::to_string(bodyId);
            return result;
        }

        int newId = doc.addBody(shape, bodyName);
        doc.setBodyVisible(newId, visible != 0);
        ++loadedCount;
    }

    // Verify END marker
    std::string endLine;
    if (std::getline(ifs, endLine) && endLine != "END") {
        // Non-fatal: file may have trailing content
    }

    ifs.close();
    result.success = true;
    result.bodiesLoaded = loadedCount;
    return result;
}

} // namespace materializr
