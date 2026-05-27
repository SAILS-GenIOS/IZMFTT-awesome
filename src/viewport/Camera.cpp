#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace materializr {

Camera::Camera()
    : m_position(5.0f, 5.0f, 5.0f)
    , m_target(0.0f, 0.0f, 0.0f)
    , m_up(0.0f, 1.0f, 0.0f)
    , m_fov(45.0f)
    , m_nearPlane(0.01f)
    , m_farPlane(1000.0f)
    , m_aspect(16.0f / 9.0f)
    , m_orbitSpeed(0.005f)
    , m_panSpeed(0.01f)
    , m_zoomSpeed(0.1f)
{
}

// Trackball rotation: yaw around the camera's CURRENT up vector, pitch around
// the camera's CURRENT right vector. Independent of any fixed world axis, so
// it behaves correctly when exiting orthographic sketch views where the camera
// is pointed straight down a non-Y face (the old Y-up spherical math hit a
// singularity there and the first drag would feel 90° off).
static void trackballRotate(glm::vec3& position, const glm::vec3& target, glm::vec3& upInOut,
                             float yaw, float pitch)
{
    glm::vec3 offset = position - target;
    if (glm::length(offset) < 1e-6f) return;

    glm::vec3 forward = -glm::normalize(offset);
    glm::vec3 right = glm::cross(forward, upInOut);
    if (glm::length(right) < 1e-6f) {
        // Degenerate (up parallel to forward); fall back to a world axis to recover.
        right = glm::cross(forward, glm::vec3(0, 1, 0));
        if (glm::length(right) < 1e-6f) right = glm::cross(forward, glm::vec3(1, 0, 0));
    }
    right = glm::normalize(right);
    glm::vec3 up = glm::normalize(glm::cross(right, forward));

    glm::mat4 rotYaw = glm::rotate(glm::mat4(1.0f), yaw, up);
    glm::mat4 rotPitch = glm::rotate(glm::mat4(1.0f), pitch, right);
    glm::mat4 rot = rotPitch * rotYaw;

    glm::vec3 newOffset = glm::vec3(rot * glm::vec4(offset, 0.0f));
    glm::vec3 newUp = glm::vec3(rot * glm::vec4(up, 0.0f));

    position = target + newOffset;
    upInOut = glm::normalize(newUp);
}

void Camera::orbit(float deltaX, float deltaY)
{
    // Orbiting implies free 3D rotation, so drop the ortho lock if it was set
    // by entering a sketch. Pan and zoom intentionally keep ortho mode on.
    m_orthographic = false;

    trackballRotate(m_position, m_target, m_up,
                    -deltaX * m_orbitSpeed, -deltaY * m_orbitSpeed);
}

void Camera::rotateAroundTarget(float yawRadians, float pitchRadians)
{
    m_orthographic = false;
    trackballRotate(m_position, m_target, m_up, -yawRadians, -pitchRadians);
}

void Camera::pan(float deltaX, float deltaY)
{
    glm::vec3 forward = glm::normalize(m_target - m_position);
    glm::vec3 right = glm::normalize(glm::cross(forward, m_up));
    glm::vec3 up = glm::normalize(glm::cross(right, forward));

    // In ortho, visible size depends on m_orthoSize (not distance), so pan should
    // track that instead — otherwise panning at distance 0.1 looks frozen while
    // panning at distance 100 throws the model off-screen.
    float scaleRef = m_orthographic ? m_orthoSize : glm::length(m_target - m_position);
    float panScale = scaleRef * m_panSpeed;

    glm::vec3 offset = -right * deltaX * panScale + up * deltaY * panScale;
    m_position += offset;
    m_target += offset;
}

void Camera::zoom(float delta)
{
    if (m_orthographic) {
        // In ortho, "zoom" scales the visible extents rather than moving the camera.
        // Multiplicative step keeps the feel consistent across scales.
        float factor = 1.0f - delta * m_zoomSpeed;
        factor = glm::clamp(factor, 0.1f, 10.0f);
        m_orthoSize = std::max(0.01f, m_orthoSize * factor);
        return;
    }

    glm::vec3 direction = m_position - m_target;
    float distance = glm::length(direction);

    // Scale zoom by current distance for consistent feel
    float zoomAmount = delta * m_zoomSpeed * distance;

    // Prevent zooming through the target
    float newDistance = distance - zoomAmount;
    newDistance = std::max(newDistance, 0.1f);

    m_position = m_target + glm::normalize(direction) * newDistance;
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(m_position, m_target, m_up);
}

glm::mat4 Camera::getProjectionMatrix() const
{
    if (m_orthographic) {
        float h = m_orthoSize;
        float w = h * m_aspect;
        return glm::ortho(-w, w, -h, h, m_nearPlane, m_farPlane);
    }
    return glm::perspective(glm::radians(m_fov), m_aspect, m_nearPlane, m_farPlane);
}

void Camera::setAspect(float aspect)
{
    m_aspect = aspect;
}

void Camera::reset()
{
    m_position = glm::vec3(5.0f, 5.0f, 5.0f);
    m_target = glm::vec3(0.0f, 0.0f, 0.0f);
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);
    m_fov = 45.0f;
    m_orthographic = false;
}

void Camera::zoomToFit(glm::vec3 min, glm::vec3 max)
{
    glm::vec3 center = (min + max) * 0.5f;
    glm::vec3 extents = max - min;
    float radius = glm::length(extents) * 0.5f;

    // Maintain current view direction
    glm::vec3 direction = glm::normalize(m_position - m_target);
    m_target = center;

    if (m_orthographic) {
        // In ortho, fit = adjust the visible extents. Add a margin so geometry
        // doesn't sit right on the edge of the viewport.
        m_orthoSize = std::max(radius * 1.1f, 0.01f);
        // Position is kept aligned along the current direction with a safe distance
        // so geometry stays between the near/far planes.
        float dist = std::max(glm::length(m_position - m_target), radius * 2.0f);
        m_position = center + direction * dist;
        return;
    }

    // Compute required distance to fit the bounding sphere in view
    float halfFov = glm::radians(m_fov) * 0.5f;
    float distance = radius / std::sin(halfFov);
    m_position = center + direction * distance;
}

void Camera::setPosition(glm::vec3 pos)
{
    m_position = pos;
}

void Camera::setTarget(glm::vec3 target)
{
    m_target = target;
}

void Camera::setUp(glm::vec3 up)
{
    m_up = up;
}

} // namespace materializr
