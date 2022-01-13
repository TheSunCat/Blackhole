#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

// some code from hmazhar/moderngl_camera under MIT
class Camera
{
public:

    enum Type
    {
        Orbit,
        Look
    };

    Camera();
    ~Camera();

    void reset();
    void update();

    void moveBy(const glm::vec3& delta);
    void rotateBy(const glm::vec3& delta);
    void distBy(float delta);

    glm::mat4 getViewMatrix();

private:
    glm::vec3 m_pos{};

    glm::fquat m_orientation{};

    glm::vec3 m_focus{};

    float m_moveSpeed = 1.0f;
    float m_rotateSpeed = 1.0f;
    float m_distance = 1.0f;

    bool m_updateNeeded = true;
    glm::mat4 m_viewMatrix;

    // smoothing vars
    float m_distTarget = 0.0f;
    float m_distDrag = 0.8f;

    glm::vec3 m_rotationTarget{};
    float m_rotationDrag = 0.8f;

    glm::vec3 m_focusTarget{};
    float m_focusDrag = 0.9f;
};
