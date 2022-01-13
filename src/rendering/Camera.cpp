#include "rendering/Camera.h"

#include "Util.h"
#include <glm/gtc/matrix_access.hpp>

Camera::Camera()
{
    update();
}

Camera::~Camera()
{
}

void Camera::reset()
{
     // TODO what do I reset?
     m_position = glm::vec3();

     update();
}

void Camera::update()
{
    m_position = lerp(m_position, m_positionTarget, m_positionDrag);
    //m_rotation = lerp(m_rotation, m_rotationTarget, m_rotationDrag);
}

void Camera::move(const glm::vec3& delta)
{
    m_positionTarget += delta;
    m_updateNeeded = true;
}

void Camera::moveRel(const glm::vec3& delta)
{
    glm::vec3 realDelta;

}


void Camera::rotate(const glm::vec3& axis, float angle)
{
    // TODO rotationTarget
    glm::fquat rot = glm::normalize(glm::angleAxis(angle, axis));

    m_orientation = m_orientation * rot;
    m_updateNeeded = true;
}

const glm::vec3 Camera::right() const {
    return glm::vec3(glm::row(rotation(), 0));
}

const glm::vec3 Camera::up() const {
    return glm::vec3(glm::row(rotation(), 1));
}

const glm::vec3 Camera::forward() const {
    return glm::vec3(glm::row(rotation(), 2));
}

glm::mat4 Camera::getViewMatrix()
{
    return rotation() * translation();
}

glm::mat4 Camera::translation() const {
    return glm::translate(glm::mat4(), -m_position);
}

glm::mat4 Camera::rotation() const {
    return glm::toMat4(m_orientation);
}
