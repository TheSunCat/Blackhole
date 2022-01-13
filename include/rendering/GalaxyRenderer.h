#pragma once

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_3_3_Core>

#include "rendering/Camera.h"

class GalaxyRenderer : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    QOpenGLShaderProgram m_objectShader;
    GLuint VAO;

    Camera m_camera;
    glm::vec2 m_lastMousePos;
public:
    GalaxyRenderer(QWidget *parent = 0);
    ~GalaxyRenderer();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
};
