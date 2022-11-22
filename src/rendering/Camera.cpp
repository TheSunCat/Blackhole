#include "rendering/Camera.h"

#include "Util.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Camera::Camera()
{
    reset();
}

Camera::~Camera()
{
}

void Camera::reset()
{
     // TODO what do I reset?
     m_position = glm::vec3(); m_positionTarget = m_position;
     m_rotation = glm::vec3();
     m_up = glm::vec3(0, 1, 0);

     update();
}

void Camera::update()
{
    m_position = Util::lerp(m_position, m_positionTarget, m_positionDrag);
    //m_orientation = glm::slerp(m_orientation, m_orientationTarget, m_orientationDrag);

    m_front.x = cos(m_rotation.y) * cos(m_rotation.x);
    m_front.y = sin(m_rotation.x);
    m_front.z = sin(m_rotation.y) * cos(m_rotation.x);
    m_front = glm::normalize(m_front);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up    = glm::normalize(glm::cross(m_right, m_front));
}

void Camera::move(const glm::vec3& delta)
{
    m_positionTarget += delta;
    m_updateNeeded = true;
}

void Camera::moveRel(const glm::vec3& delta)
{
    glm::vec3 realDelta =
        delta.x * m_right +
        delta.y * m_up +
        delta.z * m_front;

    move(realDelta);
}

void Camera::moveMouse(const float mouseX, const float mouseY)
{
    moveRel(glm::vec3(-mouseX * m_pixelFactor.x, mouseY * m_pixelFactor.y, 0));
}


void Camera::rotate(const float pitch, const float yaw)
{
    m_rotation.x += pitch;
    m_rotation.y += yaw;
    m_updateNeeded = true;
}

void Camera::rotateMouse(const float mouseX, const float mouseY)
{
    rotate(mouseY * m_pixelFactor.y, mouseX * m_pixelFactor.x);
}

glm::mat4 Camera::matrix() const
{
    return projection() * view();
}

glm::mat4 Camera::projection() const {
    return glm::perspective(m_fov, m_aspectRatio, m_znear, m_zfar);
}

glm::mat4 Camera::view() const
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

void Camera::setDimensions(const float width, const float height)
{
    m_aspectRatio = width / height;

    m_pixelFactor = float(2 * tan(m_fov * 0.5f)) * m_aspectRatio / glm::vec2(width, height);
}
