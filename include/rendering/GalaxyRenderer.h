#pragma once

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_3_3_Core>

#include <thread>
#include <iostream>

#include "rendering/Camera.h"

class GalaxyRenderer : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    std::jthread m_updateThread = std::jthread([this] (std::stop_token itoken) {
        using namespace std::chrono_literals;

        int counter{0};
        while(true)
        {
            std::this_thread::sleep_for(5ms); // TODO
            if (itoken.stop_requested()) return;
            this->update();
            ++counter;
        }
    });

    QOpenGLShaderProgram m_objectShader;
    GLuint VAO;
    float m_scaledown = 10000;

    Camera m_camera;
    glm::vec2 m_lastMousePos;
    bool m_mouseDragging = false;
    int m_mouseWarping = false;
public:
    GalaxyRenderer(QWidget *parent = 0);
    ~GalaxyRenderer();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
};
