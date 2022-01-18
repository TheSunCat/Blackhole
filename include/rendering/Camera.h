#pragma once

#include <glm/glm.hpp>

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
    void moveMouse(const float mouseX, const float mouseY);


    void rotate(const float pitch, const float yaw);
    void rotateMouse(const float mouseX, const float mouseY);

    glm::vec3 m_right;
    glm::vec3 m_up;
    glm::vec3 m_front;

    glm::mat4 matrix() const;

    glm::mat4 projection() const;
    glm::mat4 view() const;

    void setDimensions(const float width, const float height);

private:
    glm::vec3 m_position{};
    glm::vec3 m_rotation{};

    // TODO allow this to be changed
    glm::vec3 m_worldUp{0, 1, 0};

    float m_moveSpeed = 1.0f;
    float m_rotateSpeed = 1.0f;

    float m_fov = 70.0f;
    float m_aspectRatio = 1.0f;
    glm::vec2 m_pixelFactor = glm::vec3(1);

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
