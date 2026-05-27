#include "TextSketchOp.h"
#include "Sketch.h"
#include <cctype>

namespace materializr {

TextSketch::TextSketch() = default;

void TextSketch::generate(Sketch* sketch, const std::string& text,
                          glm::vec2 position, float charHeight) {
    if (!sketch || text.empty() || charHeight <= 0.0f) return;

    float advance = (m_charWidth + m_spacing) * charHeight;

    for (size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (c == ' ') continue; // skip spaces, they just advance position

        CharOutline outline = getCharOutline(c);
        glm::vec2 charOffset = position + glm::vec2(static_cast<float>(i) * advance, 0.0f);

        for (const auto& seg : outline.lines) {
            glm::vec2 p1 = charOffset + seg.first * glm::vec2(m_charWidth * charHeight, charHeight);
            glm::vec2 p2 = charOffset + seg.second * glm::vec2(m_charWidth * charHeight, charHeight);

            int ptId1 = sketch->addPoint(p1);
            int ptId2 = sketch->addPoint(p2);
            sketch->addLine(ptId1, ptId2);
        }
    }
}

CharOutline TextSketch::getCharOutline(char c) const {
    CharOutline outline;
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

    switch (c) {
    // --- Letters A-Z ---
    case 'A':
        outline.lines = {
            {{0.0f, 0.0f}, {0.5f, 1.0f}},
            {{0.5f, 1.0f}, {1.0f, 0.0f}},
            {{0.2f, 0.5f}, {0.8f, 0.5f}}
        };
        break;
    case 'B':
        outline.lines = {
            {{0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {0.8f, 1.0f}},
            {{0.8f, 1.0f}, {0.8f, 0.5f}},
            {{0.8f, 0.5f}, {0.0f, 0.5f}},
            {{0.0f, 0.0f}, {0.8f, 0.0f}},
            {{0.8f, 0.0f}, {0.8f, 0.5f}}
        };
        break;
    case 'C':
        outline.lines = {
            {{1.0f, 1.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {0.0f, 0.0f}},
            {{0.0f, 0.0f}, {1.0f, 0.0f}}
        };
        break;
    case 'D':
        outline.lines = {
            {{0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {0.7f, 1.0f}},
            {{0.7f, 1.0f}, {1.0f, 0.7f}},
            {{1.0f, 0.7f}, {1.0f, 0.3f}},
            {{1.0f, 0.3f}, {0.7f, 0.0f}},
            {{0.7f, 0.0f}, {0.0f, 0.0f}}
        };
        break;
    case 'E':
        outline.lines = {
            {{1.0f, 1.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {0.0f, 0.0f}},
            {{0.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.0f, 0.5f}, {0.7f, 0.5f}}
        };
        break;
    case 'F':
        outline.lines = {
            {{0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {1.0f, 1.0f}},
            {{0.0f, 0.5f}, {0.7f, 0.5f}}
        };
        break;
    case 'G':
        outline.lines = {
            {{1.0f, 1.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {0.0f, 0.0f}},
            {{0.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, 0.0f}, {1.0f, 0.5f}},
            {{1.0f, 0.5f}, {0.5f, 0.5f}}
        };
        break;
    case 'H':
        outline.lines = {
            {{0.0f, 0.0f}, {0.0f, 1.0f}},
            {{1.0f, 0.0f}, {1.0f, 1.0f}},
            {{0.0f, 0.5f}, {1.0f, 0.5f}}
        };
        break;
    case 'I':
        outline.lines = {
            {{0.2f, 0.0f}, {0.8f, 0.0f}},
            {{0.5f, 0.0f}, {0.5f, 1.0f}},
            {{0.2f, 1.0f}, {0.8f, 1.0f}}
        };
        break;
    case 'J':
        outline.lines = {
            {{0.2f, 1.0f}, {1.0f, 1.0f}},
            {{0.7f, 1.0f}, {0.7f, 0.0f}},
            {{0.7f, 0.0f}, {0.0f, 0.0f}},
            {{0.0f, 0.0f}, {0.0f, 0.3f}}
        };
        break;
    case 'K':
        outline.lines = {
            {{0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.0f, 0.5f}, {1.0f, 1.0f}},
            {{0.0f, 0.5f}, {1.0f, 0.0f}}
        };
        break;
    case 'L':
        outline.lines = {
            {{0.0f, 1.0f}, {0.0f, 0.0f}},
            {{0.0f, 0.0f}, {1.0f, 0.0f}}
        };
        break;
    case 'M':
        outline.lines = {
            {{0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {0.5f, 0.5f}},
            {{0.5f, 0.5f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f}, {1.0f, 0.0f}}
        };
        break;
    case 'N':
        outline.lines = {
            {{0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f, 0.0f}, {1.0f, 1.0f}}
        };
        break;
    case 'O':
        outline.lines = {
            {{0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f, 0.0f}, {0.0f, 0.0f}}
        };
        break;
    case 'P':
        outline.lines = {
            {{0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f}, {1.0f, 0.5f}},
            {{1.0f, 0.5f}, {0.0f, 0.5f}}
        };
        break;
    case 'Q':
        outline.lines = {
            {{0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.6f, 0.3f}, {1.0f, 0.0f}}
        };
        break;
    case 'R':
        outline.lines = {
            {{0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f}, {1.0f, 0.5f}},
            {{1.0f, 0.5f}, {0.0f, 0.5f}},
            {{0.5f, 0.5f}, {1.0f, 0.0f}}
        };
        break;
    case 'S':
        outline.lines = {
            {{1.0f, 1.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {0.0f, 0.5f}},
            {{0.0f, 0.5f}, {1.0f, 0.5f}},
            {{1.0f, 0.5f}, {1.0f, 0.0f}},
            {{1.0f, 0.0f}, {0.0f, 0.0f}}
        };
        break;
    case 'T':
        outline.lines = {
            {{0.0f, 1.0f}, {1.0f, 1.0f}},
            {{0.5f, 1.0f}, {0.5f, 0.0f}}
        };
        break;
    case 'U':
        outline.lines = {
            {{0.0f, 1.0f}, {0.0f, 0.0f}},
            {{0.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, 0.0f}, {1.0f, 1.0f}}
        };
        break;
    case 'V':
        outline.lines = {
            {{0.0f, 1.0f}, {0.5f, 0.0f}},
            {{0.5f, 0.0f}, {1.0f, 1.0f}}
        };
        break;
    case 'W':
        outline.lines = {
            {{0.0f, 1.0f}, {0.0f, 0.0f}},
            {{0.0f, 0.0f}, {0.5f, 0.5f}},
            {{0.5f, 0.5f}, {1.0f, 0.0f}},
            {{1.0f, 0.0f}, {1.0f, 1.0f}}
        };
        break;
    case 'X':
        outline.lines = {
            {{0.0f, 0.0f}, {1.0f, 1.0f}},
            {{0.0f, 1.0f}, {1.0f, 0.0f}}
        };
        break;
    case 'Y':
        outline.lines = {
            {{0.0f, 1.0f}, {0.5f, 0.5f}},
            {{1.0f, 1.0f}, {0.5f, 0.5f}},
            {{0.5f, 0.5f}, {0.5f, 0.0f}}
        };
        break;
    case 'Z':
        outline.lines = {
            {{0.0f, 1.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f}, {0.0f, 0.0f}},
            {{0.0f, 0.0f}, {1.0f, 0.0f}}
        };
        break;

    // --- Digits 0-9 ---
    case '0':
        outline.lines = {
            {{0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.0f, 0.0f}, {1.0f, 1.0f}} // diagonal slash
        };
        break;
    case '1':
        outline.lines = {
            {{0.3f, 0.8f}, {0.5f, 1.0f}},
            {{0.5f, 1.0f}, {0.5f, 0.0f}},
            {{0.2f, 0.0f}, {0.8f, 0.0f}}
        };
        break;
    case '2':
        outline.lines = {
            {{0.0f, 1.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f}, {1.0f, 0.5f}},
            {{1.0f, 0.5f}, {0.0f, 0.5f}},
            {{0.0f, 0.5f}, {0.0f, 0.0f}},
            {{0.0f, 0.0f}, {1.0f, 0.0f}}
        };
        break;
    case '3':
        outline.lines = {
            {{0.0f, 1.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.3f, 0.5f}, {1.0f, 0.5f}}
        };
        break;
    case '4':
        outline.lines = {
            {{0.0f, 1.0f}, {0.0f, 0.5f}},
            {{0.0f, 0.5f}, {1.0f, 0.5f}},
            {{1.0f, 1.0f}, {1.0f, 0.0f}}
        };
        break;
    case '5':
        outline.lines = {
            {{1.0f, 1.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {0.0f, 0.5f}},
            {{0.0f, 0.5f}, {1.0f, 0.5f}},
            {{1.0f, 0.5f}, {1.0f, 0.0f}},
            {{1.0f, 0.0f}, {0.0f, 0.0f}}
        };
        break;
    case '6':
        outline.lines = {
            {{1.0f, 1.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {0.0f, 0.0f}},
            {{0.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, 0.0f}, {1.0f, 0.5f}},
            {{1.0f, 0.5f}, {0.0f, 0.5f}}
        };
        break;
    case '7':
        outline.lines = {
            {{0.0f, 1.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f}, {0.3f, 0.0f}}
        };
        break;
    case '8':
        outline.lines = {
            {{0.0f, 0.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.0f, 0.5f}, {1.0f, 0.5f}}
        };
        break;
    case '9':
        outline.lines = {
            {{1.0f, 0.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {0.0f, 0.5f}},
            {{0.0f, 0.5f}, {1.0f, 0.5f}}
        };
        break;

    // --- Basic punctuation ---
    case '.':
        outline.lines = {
            {{0.4f, 0.0f}, {0.6f, 0.0f}},
            {{0.6f, 0.0f}, {0.6f, 0.1f}},
            {{0.6f, 0.1f}, {0.4f, 0.1f}},
            {{0.4f, 0.1f}, {0.4f, 0.0f}}
        };
        break;
    case ',':
        outline.lines = {
            {{0.4f, 0.1f}, {0.6f, 0.1f}},
            {{0.6f, 0.1f}, {0.4f, -0.1f}}
        };
        break;
    case '-':
        outline.lines = {
            {{0.2f, 0.5f}, {0.8f, 0.5f}}
        };
        break;
    case '+':
        outline.lines = {
            {{0.2f, 0.5f}, {0.8f, 0.5f}},
            {{0.5f, 0.2f}, {0.5f, 0.8f}}
        };
        break;
    case '!':
        outline.lines = {
            {{0.5f, 1.0f}, {0.5f, 0.3f}},
            {{0.4f, 0.0f}, {0.6f, 0.0f}},
            {{0.6f, 0.0f}, {0.6f, 0.1f}},
            {{0.6f, 0.1f}, {0.4f, 0.1f}},
            {{0.4f, 0.1f}, {0.4f, 0.0f}}
        };
        break;
    case '?':
        outline.lines = {
            {{0.0f, 1.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f}, {1.0f, 0.5f}},
            {{1.0f, 0.5f}, {0.5f, 0.5f}},
            {{0.5f, 0.5f}, {0.5f, 0.3f}},
            {{0.4f, 0.0f}, {0.6f, 0.0f}},
            {{0.6f, 0.0f}, {0.6f, 0.1f}},
            {{0.6f, 0.1f}, {0.4f, 0.1f}},
            {{0.4f, 0.1f}, {0.4f, 0.0f}}
        };
        break;
    case ':':
        outline.lines = {
            {{0.4f, 0.6f}, {0.6f, 0.6f}},
            {{0.6f, 0.6f}, {0.6f, 0.7f}},
            {{0.6f, 0.7f}, {0.4f, 0.7f}},
            {{0.4f, 0.7f}, {0.4f, 0.6f}},
            {{0.4f, 0.0f}, {0.6f, 0.0f}},
            {{0.6f, 0.0f}, {0.6f, 0.1f}},
            {{0.6f, 0.1f}, {0.4f, 0.1f}},
            {{0.4f, 0.1f}, {0.4f, 0.0f}}
        };
        break;
    case '/':
        outline.lines = {
            {{0.0f, 0.0f}, {1.0f, 1.0f}}
        };
        break;
    case '(':
        outline.lines = {
            {{0.7f, 1.0f}, {0.3f, 0.7f}},
            {{0.3f, 0.7f}, {0.3f, 0.3f}},
            {{0.3f, 0.3f}, {0.7f, 0.0f}}
        };
        break;
    case ')':
        outline.lines = {
            {{0.3f, 1.0f}, {0.7f, 0.7f}},
            {{0.7f, 0.7f}, {0.7f, 0.3f}},
            {{0.7f, 0.3f}, {0.3f, 0.0f}}
        };
        break;
    case '=':
        outline.lines = {
            {{0.2f, 0.6f}, {0.8f, 0.6f}},
            {{0.2f, 0.4f}, {0.8f, 0.4f}}
        };
        break;
    case '#':
        outline.lines = {
            {{0.3f, 0.0f}, {0.3f, 1.0f}},
            {{0.7f, 0.0f}, {0.7f, 1.0f}},
            {{0.1f, 0.35f}, {0.9f, 0.35f}},
            {{0.1f, 0.65f}, {0.9f, 0.65f}}
        };
        break;
    case '@':
        outline.lines = {
            {{1.0f, 0.3f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f}, {0.0f, 1.0f}},
            {{0.0f, 1.0f}, {0.0f, 0.0f}},
            {{0.0f, 0.0f}, {1.0f, 0.0f}},
            {{1.0f, 0.0f}, {1.0f, 0.3f}},
            {{0.7f, 0.3f}, {0.7f, 0.7f}},
            {{0.7f, 0.7f}, {0.3f, 0.7f}},
            {{0.3f, 0.7f}, {0.3f, 0.3f}},
            {{0.3f, 0.3f}, {0.7f, 0.3f}}
        };
        break;

    default:
        // Unknown character - render a small box as placeholder
        outline.lines = {
            {{0.1f, 0.1f}, {0.9f, 0.1f}},
            {{0.9f, 0.1f}, {0.9f, 0.9f}},
            {{0.9f, 0.9f}, {0.1f, 0.9f}},
            {{0.1f, 0.9f}, {0.1f, 0.1f}}
        };
        break;
    }

    return outline;
}

} // namespace materializr
