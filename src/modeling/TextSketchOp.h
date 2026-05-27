#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace materializr {
class Sketch;

struct CharOutline {
    std::vector<std::pair<glm::vec2, glm::vec2>> lines; // line segments
};

class TextSketch {
public:
    TextSketch();

    // Generate sketch lines for a text string at given position and size
    void generate(Sketch* sketch, const std::string& text,
                  glm::vec2 position, float charHeight);

private:
    CharOutline getCharOutline(char c) const;
    float m_charWidth = 0.6f; // relative to height
    float m_spacing = 0.15f;  // inter-character spacing relative to height
};

} // namespace materializr
