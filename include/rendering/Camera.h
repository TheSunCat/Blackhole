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

    void move(const glm::vec3& delta);
    void moveRel(const glm::vec3& delta);
    void rotate(const glm::vec3& axis, float angle);

    const glm::vec3 right() const;
    const glm::vec3 up() const;
    const glm::vec3 forward() const;

    glm::mat4 getViewMatrix();

    glm::mat4 translation() const;
    glm::mat4 rotation() const;

private:
    glm::vec3 m_position{};
    glm::fquat m_orientation{};

    float m_moveSpeed = 1.0f;
    float m_rotateSpeed = 1.0f;

    bool m_updateNeeded = true;
    glm::mat4 m_viewMatrix;

    // smoothing vars
    //glm::vec3 m_rotationTarget{};
    //float m_rotationDrag = 0.8f;

    glm::vec3 m_positionTarget{};
    float m_positionDrag = 0.9f;
};
