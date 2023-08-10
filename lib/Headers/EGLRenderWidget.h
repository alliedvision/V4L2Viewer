#ifndef EGLRENDERWIDGET_H
#define EGLRENDERWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMutex>
#include "BufferWrapper.h"
#include <string>
#include <memory>

struct RenderSettings {
    std::string const& fragmentShaderSource;
    GLenum glPixelFormat;
    GLenum glPixelType;
    GLenum glInternalFormat;
    void (*uploadData)(QOpenGLTexture&, RenderSettings const&, BufferWrapper const&);
};

class EGLRenderWidget: public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT;

    QOpenGLBuffer vertices;
    std::unique_ptr<QOpenGLShaderProgram> shader;
    std::unique_ptr<QOpenGLShader> vertexShader;
    std::unique_ptr<QOpenGLShader> fragmentShader;
    std::unique_ptr<QOpenGLTexture> texture;
    int matrixUniformLocation;
    int textureUniformLocation;
    int texSizeUniformLocation;
    QMatrix4x4 windowMatrix;
    QMatrix4x4 fullMatrix;
    QMatrix4x4 frameMatrix;
    QMatrix4x4 viewMatrix;
    bool flipX = false;
    bool flipY = false;
    float scale = 1.0f;
    float scrollX = 0.0f;
    float scrollY = 0.0f;
    int frameWidth = 0;
    int frameHeight = 0;
    int textureWidth = 0;
    int textureHeight = 0;
    int pixelFormat = 0;
    bool recreatePipeline = true;
    bool npotSupported = false;

    void updateMatrix();
    void updateViewMatrix();

    std::function<void()> onDraw;

    QMutex dataMutex;
    std::function<void()> nextDoneCallback;
    BufferWrapper nextBuffer;
    RenderSettings const* currentRenderSettings = nullptr;

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void resizeEvent(QResizeEvent *) override;
public:
    void setFormat(int width, int height, int pixelformat);
    void setScale(float scale);
    void setFlip(bool flipX, bool flipY);
    EGLRenderWidget(std::function<void()> onDraw);
    ~EGLRenderWidget();
    void nextFrame(BufferWrapper const& buffer, std::function<void()> doneCallback);
    void setScroll(int x, int y);
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    static bool canRender(uint32_t pixelFormat);

signals:
    void resized();
    void requestZoom(QPointF center, bool zoomIn);
    void clicked(QPointF point);
};

#endif
