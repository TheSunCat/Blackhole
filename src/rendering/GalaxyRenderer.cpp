#include "rendering/GalaxyRenderer.h"

#include <QMouseEvent>

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

GalaxyRenderer::GalaxyRenderer(QWidget *parent) : QOpenGLWidget(parent)
{
//     this->timer = new QTimer();
//     QObject::connect(timer, SIGNAL(timeout()), this, SLOT(processing()));
//     timer->start( timerInterval );

    QSurfaceFormat fmt = format();
    fmt.setSamples(16); // multisampling set to 16
    setFormat(fmt);

    m_camera.move({0, 0, 10});
}

GalaxyRenderer::~GalaxyRenderer()
{

}

void GalaxyRenderer::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);

    // make shaders
    m_objectShader.addCacheableShaderFromSourceCode(QOpenGLShader::Vertex,
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "}\0"
    );

    m_objectShader.addCacheableShaderFromSourceCode(QOpenGLShader::Fragment,
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\0"
    );

    m_objectShader.link();

    // set up cube
    float vertices[] = {
        // positions          // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // top left
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    unsigned int VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // texture coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GalaxyRenderer::paintGL()
{
    m_camera.update();


    glClearColor(0.f, 0.3f, 0.2f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_objectShader.bind();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(m_objectShader.programId(), "model"), 1, GL_FALSE, glm::value_ptr(model));

    glm::mat4 projection = m_camera.projection();
    glUniformMatrix4fv(glGetUniformLocation(m_objectShader.programId(), "projection"), 1, GL_FALSE, &projection[0][0]);
    glm::mat4 view = m_camera.view();
    glUniformMatrix4fv(glGetUniformLocation(m_objectShader.programId(), "view"), 1, GL_FALSE, &view[0][0]);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void GalaxyRenderer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);

    m_camera.setDimensions(width(), height());
}

void GalaxyRenderer::mouseMoveEvent(QMouseEvent* event)
{
    glm::vec2 mousePos = glm::vec2(event->x(), event->y());
    uint32_t mouseButtons = event->buttons();

    glm::vec2 mouseDelta = mousePos - m_lastMousePos;

    if(mouseButtons & Qt::LeftButton)
    {
        m_camera.moveRel(glm::vec3(-mouseDelta.x / 1000.f, mouseDelta.y / 1000.f, 0));
    }

    if(mouseButtons & Qt::RightButton)
    {
        m_camera.rotate(-mouseDelta.y / 1000.f, mouseDelta.x / 1000.f);
    }

    // wrap mouse if it exits while dragging
    if(mouseButtons & Qt::LeftButton || mouseButtons & Qt::RightButton)
    {
        bool leftX = mousePos.x < 0 || mousePos.x > width();
        bool leftY = mousePos.y < 0 || mousePos.y > height();

        // check if mouse is exiting
        if(leftX || leftY)
        {
            if(leftX)
                mousePos.x += (mousePos.x < 0) ? width() : -width();

            if(leftY)
                mousePos.y += (mousePos.y < 0) ? height() : -height();

            m_lastMousePos = mousePos;

            QPoint globalPos = mapToGlobal(QPoint(mousePos.x, mousePos.y));

            // warp!!
            QCursor::setPos(globalPos);
        }

    }

    update();

    m_lastMousePos = mousePos;
}

void GalaxyRenderer::wheelEvent(QWheelEvent* event)
{
    float delta = event->angleDelta().y();

    delta = copysign(1, delta) * delta*delta * 0.00005f;

    m_camera.moveRel(glm::vec3(0, 0, delta));

    update();
}

void GalaxyRenderer::mousePressEvent(QMouseEvent* event)
{
    m_lastMousePos = glm::vec2(event->x(), event->y());
    m_mouseDragging = true;
}

void GalaxyRenderer::mouseReleaseEvent(QMouseEvent *event)
{
    m_mouseDragging = false;
}

void GalaxyRenderer::leaveEvent(QEvent* event)
{

}

