#pragma once
#include <string>

namespace materializr {

struct AppSettings {
    // Units
    int unitSystem = 0; // 0=mm, 1=cm, 2=inch

    // Grid
    float gridSize = 1.0f;
    bool snapToGrid = true;
    float snapThreshold = 0.3f;

    // Defaults
    double defaultExtrudeDistance = 5.0;
    double defaultFilletRadius = 1.0;
    double defaultChamferDistance = 1.0;

    // Auto-save
    int autoSaveIntervalSec = 300;
    bool autoSaveEnabled = true;

    // Mouse
    float orbitSensitivity = 1.0f;
    float panSensitivity = 1.0f;
    float zoomSensitivity = 1.0f;

    // Display
    bool showGrid = true;
    bool showEdgeWireframe = true;
    bool showConstructionPlanes = true;
};

class SettingsPanel {
public:
    SettingsPanel();

    AppSettings& getSettings();
    const AppSettings& getSettings() const;

    // Load/save settings from/to file
    bool load(const std::string& filePath);
    bool save(const std::string& filePath) const;

    // Render the settings window. Returns true if settings changed.
    bool render();

    void setVisible(bool vis);
    bool isVisible() const;

private:
    AppSettings m_settings;
    bool m_visible = false;
};

} // namespace materializr
