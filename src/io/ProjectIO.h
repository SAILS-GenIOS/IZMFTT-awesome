#pragma once
#include <string>

class Document;
class History;

namespace materializr {

struct ProjectSaveResult {
    bool success = false;
    std::string errorMessage;
};

struct ProjectLoadResult {
    bool success = false;
    std::string errorMessage;
    int bodiesLoaded = 0;
};

class ProjectIO {
public:
    static ProjectSaveResult save(const std::string& filePath, const Document& doc);
    static ProjectLoadResult load(const std::string& filePath, Document& doc);
};

} // namespace materializr
