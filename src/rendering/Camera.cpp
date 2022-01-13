#include "rendering/Camera.h"

#include "Util.h"

Camera::Camera() : m_front(0, 0, -1)
{
    update();
}

Camera::~Camera()
{
}

void Camera::reset()
{
     // TODO what do I reset?
     m_pos = glm::vec3();
     m_focus = glm::vec3(1, 0, 0);
     m_up = glm::vec3(0, 1, 0);

     update();
}

void Camera::update()
{
    m_focus = lerp(m_focus, m_focusTarget, m_focusDrag);
    m_rotation = lerp(m_rotation, m_rotationTarget, m_rotationDrag);
    m_distance = lerp(m_distance, m_distTarget, m_distDrag);

    // calculate camera position

    glm::toMat4(m_orientation);
    //m_front = Util::rotToVec3(mYaw, mPitch);
    m_right + glm::normalize(glm::cross(m_front, m_worldUp));
    m_up + glm::normalize(glm::cross(m_right, m_front));

    m_pos = m_focus - (m_front * m_distance);
}

void Camera::moveBy(const glm::vec3& delta)
{
    m_posTarget += delta;
    m_updateNeeded = true;
}

void Camera::rotateBy(const glm::vec3& delta)
{
    m_rotationTarget += delta;
    m_updateNeeded = true;
}

void Camera::distBy(float delta)
{
    m_distTarget += delta;
    m_updateNeeded = true;
}


glm::mat4 Camera::getViewMatrix()
{
    return glm::lookAt(m_pos, m_pos + m_front, m_up);
}
