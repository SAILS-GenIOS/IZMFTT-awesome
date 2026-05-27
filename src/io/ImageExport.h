#pragma once
#include "gl_common.h"
#include <string>
#include <functional>

namespace materializr {

struct ImageExportOptions {
    int width = 1920;
    int height = 1080;
    bool transparentBackground = false;
    int samples = 1; // MSAA samples (1 = no AA)
};

struct ImageExportResult {
    bool success = false;
    std::string errorMessage;
};

class ImageExport {
public:
    // Export the current viewport to a TGA file from an existing FBO texture
    static ImageExportResult exportPNG(const std::string& filePath,
                                        unsigned int fboTexture,
                                        int fboWidth, int fboHeight);

    // Export at custom resolution by creating a temporary FBO
    // The caller provides a render callback that draws the scene
    static ImageExportResult exportAtResolution(const std::string& filePath,
                                                 int width, int height,
                                                 std::function<void()> renderCallback);
};

} // namespace materializr
