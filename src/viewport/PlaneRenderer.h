#pragma once
#include "gl_common.h"
#include <glm/glm.hpp>
#include <gp_Pln.hxx>
#include <vector>
#include <string>

namespace materializr {

class PlaneRenderer {
public:
    PlaneRenderer();
    ~PlaneRenderer();

    bool initialize();

    struct PlaneData {
        gp_Pln plane;
        std::string name;
        glm::vec4 color;
        float size;
        bool visible;
    };

    void addPlane(const gp_Pln& plane, const std::string& name,
                  glm::vec4 color = glm::vec4(0.3f, 0.5f, 0.8f, 0.15f),
                  float size = 10.0f);

    void render(const glm::mat4& view, const glm::mat4& projection);
    void clear();

private:
    bool compileShader(unsigned int& shader, unsigned int type, const char* source);
    bool linkProgram(unsigned int vertShader, unsigned int fragShader);

    std::vector<PlaneData> m_planes;
    unsigned int m_program = 0;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    int m_locMVP = -1;
    int m_locColor = -1;
};

} // namespace materializr
