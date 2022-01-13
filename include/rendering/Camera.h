#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

// some code from tizian/Learning-OpenGL
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

    glm::mat4 matrix() const;

    glm::mat4 projection() const;
    glm::mat4 view() const;
    glm::mat4 translation() const;
    glm::mat4 rotation() const;

private:
    glm::vec3 m_position{};
    glm::quat m_orientation;

    float m_moveSpeed = 1.0f;
    float m_rotateSpeed = 1.0f;

    float m_fov = 70.0f;
    float m_aspectRatio = 1.0f;

    float m_znear = 0.01f;
    float m_zfar = 1000.0f;

    bool m_updateNeeded = true;
    glm::mat4 m_viewMatrix;

    // smoothing vars
    //glm::quat m_orientationTarget{};
    //float m_orientationDrag = 0.8f;

    glm::vec3 m_positionTarget{};
    float m_positionDrag = 0.9f;
};
