#pragma once
#include <glm/glm.hpp>
#include <string>

namespace materializr {

class DimensionInput {
public:
    DimensionInput();

    // Show/hide
    void show(const std::string& label, double currentValue, glm::vec2 screenPos);
    void hide();
    bool isVisible() const;

    // Render the floating input. Returns true if user confirmed a value.
    bool render();

    // Get the confirmed value
    double getValue() const;

    // Optional: show angle too
    void showWithAngle(double length, double angleDeg, glm::vec2 screenPos);
    double getAngle() const;

private:
    bool m_visible = false;
    std::string m_label;
    char m_valueBuffer[64] = {};
    char m_angleBuffer[64] = {};
    double m_value = 0.0;
    double m_angle = 0.0;
    glm::vec2 m_screenPos{0};
    bool m_confirmed = false;
    bool m_showAngle = false;
    bool m_firstFrame = true;
};

} // namespace materializr
