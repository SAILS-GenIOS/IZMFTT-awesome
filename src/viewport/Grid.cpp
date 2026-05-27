#include "gl_common.h"

#include "Grid.h"

#include <glm/gtc/type_ptr.hpp>
#include <cstdio>

namespace materializr {

// Embedded grid shader sources (matching shaders/grid.vert and shaders/grid.frag)
static const char* s_gridVertSource = R"(
#version 330 core

uniform mat4 u_viewProjection;
uniform mat4 u_invViewProjection;

out vec3 v_nearPoint;
out vec3 v_farPoint;

vec3 gridPlane[6] = vec3[](
    vec3( 1,  1, 0), vec3(-1, -1, 0), vec3(-1,  1, 0),
    vec3(-1, -1, 0), vec3( 1,  1, 0), vec3( 1, -1, 0)
);

vec3 unprojectPoint(float x, float y, float z) {
    vec4 unprojected = u_invViewProjection * vec4(x, y, z, 1.0);
    return unprojected.xyz / unprojected.w;
}

void main() {
    vec3 p = gridPlane[gl_VertexID];
    v_nearPoint = unprojectPoint(p.x, p.y, 0.0);
    v_farPoint  = unprojectPoint(p.x, p.y, 1.0);
    gl_Position = vec4(p, 1.0);
}
)";

static const char* s_gridFragSource = R"(
#version 330 core

in vec3 v_nearPoint;
in vec3 v_farPoint;

uniform mat4 u_viewProjection;
uniform float u_near;
uniform float u_far;

out vec4 fragColor;

float computeDepth(vec3 pos) {
    vec4 clipPos = u_viewProjection * vec4(pos, 1.0);
    return (clipPos.z / clipPos.w) * 0.5 + 0.5;
}

float computeLinearDepth(vec3 pos) {
    vec4 clipPos = u_viewProjection * vec4(pos, 1.0);
    float clipDepth = (clipPos.z / clipPos.w) * 2.0 - 1.0;
    float linearDepth = (2.0 * u_near * u_far) / (u_far + u_near - clipDepth * (u_far - u_near));
    return linearDepth / u_far;
}

vec4 grid(vec3 fragPos3D, float scale, vec4 lineColor) {
    vec2 coord = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1.0);
    float minimumx = min(derivative.x, 1.0);

    vec4 color = lineColor;
    color.a = 1.0 - min(line, 1.0);

    // X-axis highlight (red)
    if (fragPos3D.z > -0.5 * minimumz && fragPos3D.z < 0.5 * minimumz) {
        color = vec4(0.8, 0.2, 0.2, 1.0);
    }
    // Z-axis highlight (blue)
    if (fragPos3D.x > -0.5 * minimumx && fragPos3D.x < 0.5 * minimumx) {
        color = vec4(0.2, 0.2, 0.8, 1.0);
    }

    return color;
}

void main() {
    float t = -v_nearPoint.y / (v_farPoint.y - v_nearPoint.y);
    if (t < 0.0) discard;

    vec3 fragPos3D = v_nearPoint + t * (v_farPoint - v_nearPoint);
    gl_FragDepth = computeDepth(fragPos3D);

    float linearDepth = computeLinearDepth(fragPos3D);
    float fade = max(0.0, 1.0 - linearDepth);

    vec4 minorColor = grid(fragPos3D, 1.0, vec4(0.4, 0.4, 0.4, 1.0));
    vec4 majorColor = grid(fragPos3D, 0.1, vec4(0.6, 0.6, 0.6, 1.0));

    vec4 color = minorColor;
    if (majorColor.a > minorColor.a) {
        color = majorColor;
    }

    color.a *= fade;
    if (color.a < 0.001) discard;

    fragColor = color;
}
)";

Grid::Grid() {}

Grid::~Grid()
{
    if (m_shaderProgram) {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
}

bool Grid::initialize()
{
    // Compile shaders
    unsigned int vertShader = 0, fragShader = 0;
    if (!compileShader(vertShader, GL_VERTEX_SHADER, s_gridVertSource)) {
        return false;
    }
    if (!compileShader(fragShader, GL_FRAGMENT_SHADER, s_gridFragSource)) {
        glDeleteShader(vertShader);
        return false;
    }
    if (!linkProgram(vertShader, fragShader)) {
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        return false;
    }
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    // Cache uniform locations
    m_locViewProjection = glGetUniformLocation(m_shaderProgram, "u_viewProjection");
    m_locInvViewProjection = glGetUniformLocation(m_shaderProgram, "u_invViewProjection");
    m_locNear = glGetUniformLocation(m_shaderProgram, "u_near");
    m_locFar = glGetUniformLocation(m_shaderProgram, "u_far");

    // Create a dummy VAO (required for core profile, even with no vertex attributes)
    glGenVertexArrays(1, &m_vao);

    return true;
}

void Grid::render(const glm::mat4& view, const glm::mat4& projection,
                  float nearPlane, float farPlane)
{
    if (!m_shaderProgram) return;

    glm::mat4 vp = projection * view;
    glm::mat4 invVP = glm::inverse(vp);

    glUseProgram(m_shaderProgram);
    glUniformMatrix4fv(m_locViewProjection, 1, GL_FALSE, glm::value_ptr(vp));
    glUniformMatrix4fv(m_locInvViewProjection, 1, GL_FALSE, glm::value_ptr(invVP));
    glUniform1f(m_locNear, nearPlane);
    glUniform1f(m_locFar, farPlane);

    // Enable blending for grid transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw the full-screen quad (6 vertices from gl_VertexID)
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glUseProgram(0);
}

bool Grid::compileShader(unsigned int& shader, unsigned int type, const char* source)
{
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::fprintf(stderr, "Grid shader compilation failed: %s\n", infoLog);
        glDeleteShader(shader);
        shader = 0;
        return false;
    }
    return true;
}

bool Grid::linkProgram(unsigned int vertShader, unsigned int fragShader)
{
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertShader);
    glAttachShader(m_shaderProgram, fragShader);
    glLinkProgram(m_shaderProgram);

    int success = 0;
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_shaderProgram, 512, nullptr, infoLog);
        std::fprintf(stderr, "Grid shader linking failed: %s\n", infoLog);
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
        return false;
    }
    return true;
}

} // namespace materializr
