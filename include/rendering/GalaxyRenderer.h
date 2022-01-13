#pragma once

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>

class GalaxyRenderer : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
public:
    GalaxyRenderer(QWidget *parent = 0);
    ~GalaxyRenderer();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
};
