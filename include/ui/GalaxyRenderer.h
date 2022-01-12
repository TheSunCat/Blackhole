#pragma once

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>

class GalaxyRenderer : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    GalaxyRenderer(QWidget *parent = 0);
    ~GalaxyRenderer();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
};
