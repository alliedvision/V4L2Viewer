#include "EGLRenderWidget.h"
#include <QOpenGLContext>
#include <QWheelEvent>
#include <iostream>
#include <QMutexLocker>
#include <QOpenGLPixelTransferOptions>
#include <QOffscreenSurface>
#include <unordered_map>

struct VertexData {
    QVector2D pos;
    QVector2D uv;
};

namespace {
    template<unsigned bytesPerPixel>
    void uploadRaw(QOpenGLTexture& texture, RenderSettings const& settings, BufferWrapper const& buffer) {
       /*
       QOpenGLPixelTransferOptions options;
       options.setRowLength(buffer.bytesPerLine);
       texture.setData(0, 0, 0, buffer.width, buffer.height, 1,
                       QOpenGLTexture::PixelFormat(settings.glPixelFormat),
                       QOpenGLTexture::PixelType(settings.glPixelType),
                       buffer.data,
                       &options);
       */

       // Doing this with Qt functionality somehow fails on the Mali-400 GPU
       // in the Xilinx UltraScale+.
       // So we're doing it the classic way instead.
       texture.bind();
       auto & gl = *QOpenGLContext::currentContext()->functions();
       gl.glPixelStorei(GL_UNPACK_ROW_LENGTH, buffer.bytesPerLine/bytesPerPixel);
       gl.glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, buffer.width, buffer.height, settings.glPixelFormat, settings.glPixelType, buffer.data);
    }

    // Always explicitly return A=1.0 to avoid issues with AR24 and XR24 formats
    static std::string const shaderRGB = "return vec4(texture(image, v_uv).rgb, 1.0);";
    static std::string const shaderBGR = "return vec4(texture(image, v_uv).bgr, 1.0);";

    static std::string const shaderMono8 = "float v = texture(image, v_uv).r; return vec4(v,v,v,1.0);";

    // Note: On i.MX8MP, the least significant bits are 0-padding, putting the 8 most significant bits entirely into the
    //       second byte, which get uploaded right to the green channel. So no expensive float-int conversions and no
    //       bitwise operations (or tricky floating point maths) to merge the channels.
    //       The v4l2 spec for Y10 and Y12 says the most significant bits should be padding, so this may have to be
    //       changed for other SoCs.
    static std::string const shaderMono10_12_NXP = "float v = texture(image, v_uv).g; return vec4(v,v,v,1.0);";

    // Very simple demosaicing: Only linear interpolation of adjacent cells and no special treatment for first/last row/column.
    // This will incorrectly interpolate a 1 texel wide border around the image but for the purpose of the viewer this
    // should be okay.
    // All Bayer patterns are the same, just shifted by one pixel in x and/or y direction. We're
    // using RGGB as the baseline and define all others as shifted variants of that.
    static std::string const shaderBaseRGGB = R"eof(
        vec2 px = floor(texSize * v_uv);
        vec2 cuv = px / texSize;
        vec2 stepX = vec2(1.0 / texSize.x, 0.0);
        vec2 stepY = vec2(0.0, 1.0 / texSize.y);
        vec2 m = mod(px + bayerOffset, vec2(2.0, 2.0));

        float v = texture(image, cuv).channel;
        if(m.x == 0.0 && m.y == 0.0) { // Red pixel
            float g = 0.5 * (texture(image, cuv - stepX).channel + texture(image, cuv + stepX).channel);
            float b = 0.5 * (texture(image, cuv - stepX - stepY).channel + texture(image, cuv + stepX + stepY).channel);
            return vec4(v, g, b, 1.0);
        } else if(m.x == 1.0 && m.y == 1.0) { // Blue
            float g = 0.5 * (texture(image, cuv - stepX).channel + texture(image, cuv + stepX).channel);
            float r = 0.5 * (texture(image, cuv - stepX - stepY).channel + texture(image, cuv + stepX + stepY).channel);
            return vec4(r, g, v, 1.0);
        } else if(m.x == 0.0 && m.y == 1.0) { // Green in blue row
            float r = 0.5 * (texture(image, cuv - stepY).channel + texture(image, cuv + stepY).channel);
            float b = 0.5 * (texture(image, cuv - stepX).channel + texture(image, cuv + stepX).channel);
            return vec4(r, v, b, 1.0);
        } else { // Green in red row
            float b = 0.5 * (texture(image, cuv - stepY).channel + texture(image, cuv + stepY).channel);
            float r = 0.5 * (texture(image, cuv - stepX).channel + texture(image, cuv + stepX).channel);
            return vec4(r, v, b, 1.0);
        }
    )eof";

    static std::string const shaderRGGB8 = R"eof(
        #define channel r
        #define bayerOffset vec2(0.0, 0.0)
    )eof" + shaderBaseRGGB;

    static std::string const shaderGRBG8 = R"eof(
        #define channel r
        #define bayerOffset vec2(1.0, 0.0)
    )eof" + shaderBaseRGGB;

    static std::string const shaderGBRG8 = R"eof(
        #define channel r
        #define bayerOffset vec2(0.0, 1.0)
    )eof" + shaderBaseRGGB;

    static std::string const shaderBGGR8 = R"eof(
        #define channel r
        #define bayerOffset vec2(1.0, 1.0)
    )eof" + shaderBaseRGGB;

    // The same alignment issue as with Mono10+12 also exists with Bayer10+12 on NXP,
    // hence the most significant 8 bits are all in the green channel.
    // Another caveat: Interpolation is done only the most significant 8 bit in 10 and 12 bit formats.
    // Should be hardly noticeable in the viewer but saves a lot of time.
    static std::string const shaderRGGB10_12_NXP = R"eof(
        #define channel g
        #define bayerOffset vec2(0.0, 0.0)
    )eof" + shaderBaseRGGB;

    static std::string const shaderGRBG10_12_NXP = R"eof(
        #define channel g
        #define bayerOffset vec2(1.0, 0.0)
    )eof" + shaderBaseRGGB;

    static std::string const shaderGBRG10_12_NXP = R"eof(
        #define channel g
        #define bayerOffset vec2(0.0, 1.0)
    )eof" + shaderBaseRGGB;

    static std::string const shaderBGGR10_12_NXP = R"eof(
        #define channel g
        #define bayerOffset vec2(1.0, 1.0)
    )eof" + shaderBaseRGGB;

    static std::string const shaderBaseYUV = R"eof(
        vec2 px = floor(texSize * v_uv);
        vec2 cuv = px / texSize;
        vec2 stepX = vec2(1.0 / texSize.x, 0.0);
        float m = mod(px.x, 2.0);

        vec2 center = texture(image, cuv).rg;
        vec2 left   = texture(image, cuv - stepX).rg;
        vec2 right  = texture(image, cuv + stepX).rg;

        float u = (1.0 - m) * center.channelUV + m * left.channelUV   - 0.5;
        float v = (1.0 - m) * right.channelUV  + m * center.channelUV - 0.5;
        float y = center.channelY;

        return vec4(
            y + (1.403 * v),
            y - (0.344 * u) - (0.714 * v),
            y + (1.770 * u),
            1.0
        );
    )eof";

    static std::string const shaderYUYV = R"eof(
        #define channelY r
        #define channelUV g
    )eof" + shaderBaseYUV;

    static std::string const shaderUYVY = R"eof(
        #define channelY g
        #define channelUV r
    )eof" + shaderBaseYUV;

    static std::unordered_map<uint32_t, RenderSettings> const renderSettings {
        { V4L2_PIX_FMT_RGB24,   { shaderRGB, GL_RGB, GL_UNSIGNED_BYTE, GL_RGB8, uploadRaw<3> } },
        { V4L2_PIX_FMT_BGR24,   { shaderBGR, GL_RGB, GL_UNSIGNED_BYTE, GL_RGB8, uploadRaw<3> } },
        { V4L2_PIX_FMT_XBGR32,  { shaderBGR, GL_RGBA, GL_UNSIGNED_BYTE, GL_RGBA8, uploadRaw<4> } },
        { V4L2_PIX_FMT_ABGR32,  { shaderBGR, GL_RGBA, GL_UNSIGNED_BYTE, GL_RGBA8, uploadRaw<4> } },
        { V4L2_PIX_FMT_RGB565,  { shaderRGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, GL_RGB565, uploadRaw<2> } },
        { V4L2_PIX_FMT_GREY,    { shaderMono8, GL_RED, GL_UNSIGNED_BYTE, GL_R8, uploadRaw<1> } },

        { V4L2_PIX_FMT_SRGGB8,  { shaderRGGB8, GL_RED, GL_UNSIGNED_BYTE, GL_R8, uploadRaw<1> } },
        { V4L2_PIX_FMT_SGRBG8,  { shaderGRBG8, GL_RED, GL_UNSIGNED_BYTE, GL_R8, uploadRaw<1> } },
        { V4L2_PIX_FMT_SGBRG8,  { shaderGBRG8, GL_RED, GL_UNSIGNED_BYTE, GL_R8, uploadRaw<1> } },
        { V4L2_PIX_FMT_SBGGR8,  { shaderBGGR8, GL_RED, GL_UNSIGNED_BYTE, GL_R8, uploadRaw<1> } },

        { V4L2_PIX_FMT_UYVY,    { shaderUYVY, GL_RG, GL_UNSIGNED_BYTE, GL_RG8, uploadRaw<2> } },
        { V4L2_PIX_FMT_YUYV,    { shaderYUYV, GL_RG, GL_UNSIGNED_BYTE, GL_RG8, uploadRaw<2> } },

        // Note: The i.MX8 Vivante GPU really doesn't like integer textures. Uploading 16 bit integer values
        //       and using GL_RED_INTEGER / GL_UNSIGNED_SHORT / GL_R16UI led to glTexSubImage taking more than 100x
        //       as long as just uploading to a GL_RG8 target.
        //       GL_R16 is not supported by GLES3 at all, so instead we're going with an RG format and extract the
        //       necessary parts from the two channels in the fragment shader.
        { V4L2_PIX_FMT_Y10,     { shaderMono10_12_NXP, GL_RG, GL_UNSIGNED_BYTE, GL_RG8, uploadRaw<2> } },
        { V4L2_PIX_FMT_Y12,     { shaderMono10_12_NXP, GL_RG, GL_UNSIGNED_BYTE, GL_RG8, uploadRaw<2> } },

        { V4L2_PIX_FMT_SRGGB10, { shaderRGGB10_12_NXP, GL_RG, GL_UNSIGNED_BYTE, GL_RG8, uploadRaw<2> } },
        { V4L2_PIX_FMT_SGRBG10, { shaderGRBG10_12_NXP, GL_RG, GL_UNSIGNED_BYTE, GL_RG8, uploadRaw<2> } },
        { V4L2_PIX_FMT_SGBRG10, { shaderGBRG10_12_NXP, GL_RG, GL_UNSIGNED_BYTE, GL_RG8, uploadRaw<2> } },
        { V4L2_PIX_FMT_SBGGR10, { shaderBGGR10_12_NXP, GL_RG, GL_UNSIGNED_BYTE, GL_RG8, uploadRaw<2> } },

        { V4L2_PIX_FMT_SRGGB12, { shaderRGGB10_12_NXP, GL_RG, GL_UNSIGNED_BYTE, GL_RG8, uploadRaw<2> } },
        { V4L2_PIX_FMT_SGRBG12, { shaderGRBG10_12_NXP, GL_RG, GL_UNSIGNED_BYTE, GL_RG8, uploadRaw<2> } },
        { V4L2_PIX_FMT_SGBRG12, { shaderGBRG10_12_NXP, GL_RG, GL_UNSIGNED_BYTE, GL_RG8, uploadRaw<2> } },
        { V4L2_PIX_FMT_SBGGR12, { shaderBGGR10_12_NXP, GL_RG, GL_UNSIGNED_BYTE, GL_RG8, uploadRaw<2> } },
    };

    static char const pixelShaderFramework[] = R"eof(
        #define texture texture2D
        precision highp float;
        precision highp int;
         uniform sampler2D image;
        uniform vec2 texSize;
        varying vec2 v_uv;
        vec4 convert() {
            %1
        }
        void main() {
            gl_FragColor = convert();
        }
    )eof";
}

bool EGLRenderWidget::canRender(uint32_t pixelFormat) {
    static bool const rgTextureSupported = [] {
        QOpenGLContext ctx;
        ctx.create();
        QOffscreenSurface surface;
        surface.create();
        ctx.makeCurrent(&surface);
        int v;
        ctx.functions()->glGetIntegerv(GL_MAJOR_VERSION, &v);
        bool const err = ctx.functions()->glGetError();
        bool const rgExt = ctx.hasExtension("GL_EXT_texture_rg");
        ctx.doneCurrent();
        return ((err == GL_NO_ERROR) && v >= 3) || rgExt;
    }();
    auto const renderSettingsIt = renderSettings.find(pixelFormat);
    return (renderSettingsIt != renderSettings.end())
           && ((renderSettingsIt->second.glPixelFormat != GL_RG) || rgTextureSupported);
}

void EGLRenderWidget::initializeGL() {
    initializeOpenGLFunctions();

    npotSupported = context()->hasExtension("GL_ARB_texture_non_power_of_two");

    vertexShader = std::make_unique<QOpenGLShader>(QOpenGLShader::Vertex);
    bool const compiled = vertexShader->compileSourceCode(R"eof(
        precision mediump float;
        uniform mat4 matrix;
        attribute vec2 a_uv;
        attribute vec2 a_pos;
        varying vec2 v_uv;
        void main() {
            gl_Position = matrix * vec4(a_pos, 0.0, 1.0);
            v_uv = a_uv;
        }
        )eof");

    if(!compiled) {
      std::cerr << "Vertex shader compilation error (bug!):\n" << vertexShader->log().toStdString() << std::endl;
      abort();
    }

    vertices.create();
    vertices.bind();
    vertices.allocate(4 * sizeof(VertexData));
}

EGLRenderWidget::EGLRenderWidget(std::function<void()> onDraw)
  : onDraw(onDraw) {
}

EGLRenderWidget::~EGLRenderWidget() {
    vertices.destroy();
    makeCurrent();
    shader.reset();
    texture.reset();
    fragmentShader.reset();
    vertexShader.reset();
    doneCurrent();
}


void EGLRenderWidget::paintGL() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    if(frameWidth == 0) {
        return;
    }

    if(recreatePipeline) {
        recreatePipeline = false;
       
        auto const renderSettingsIt = renderSettings.find(pixelFormat);
        if(renderSettingsIt == renderSettings.end()) {
            std::cerr << "Pixel format implementation missing (bug!)" << std::endl;
            abort();
        }

        if(npotSupported) {
          textureWidth = frameWidth;
          textureHeight = frameHeight;
        } else {
          auto const roundToNextPowerOfTwo = [](int v) {
            v--;
            v |= v >> 1;
            v |= v >> 2;
            v |= v >> 4;
            v |= v >> 8;
            v |= v >> 16;
            v++;
            return v;
          };

          textureWidth = roundToNextPowerOfTwo(frameWidth);
          textureHeight = roundToNextPowerOfTwo(frameHeight);
        }

        float const uvX = float(frameWidth) / float(textureWidth);
        float const uvY = float(frameHeight) / float(textureHeight);
        VertexData const verts[] = {
            { QVector2D(-1.0, -1.0), QVector2D(0.0, uvY) },
            { QVector2D(-1.0,  1.0), QVector2D(0.0, 0.0) },
            { QVector2D( 1.0, -1.0), QVector2D(uvX, uvY) },
            { QVector2D( 1.0,  1.0), QVector2D(uvX, 0.0) }
        };

        vertices.bind();
        vertices.write(0, verts, sizeof(verts));


        currentRenderSettings = &renderSettingsIt->second;

        texture = std::make_unique<QOpenGLTexture>(QOpenGLTexture::Target2D);
        texture->setSize(textureWidth, textureHeight);
        texture->setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);

        texture->setFormat(QOpenGLTexture::TextureFormat(currentRenderSettings->glInternalFormat));
        texture->allocateStorage(QOpenGLTexture::PixelFormat(currentRenderSettings->glPixelFormat), QOpenGLTexture::PixelType(currentRenderSettings->glPixelType));

        fragmentShader = std::make_unique<QOpenGLShader>(QOpenGLShader::Fragment);
        bool const compiled = fragmentShader->compileSourceCode(QString(pixelShaderFramework).arg(currentRenderSettings->fragmentShaderSource.c_str()));
        if(not compiled) {
            std::cerr << "Fragment shader compilation error (bug!):\n" << fragmentShader->log().toStdString() << std::endl;
            abort();
        }

        shader = std::make_unique<QOpenGLShaderProgram>();
        shader->removeAllShaders();
        shader->addShader(vertexShader.get());
        shader->addShader(fragmentShader.get());
        bool const linked = shader->link();
        if(!linked) {
            std::cerr << "Shader link error (bug!):\n" << shader->log().toStdString() << std::endl;
            abort();
        }

        auto const pos_loc = shader->attributeLocation("a_pos");
        auto const uv_loc = shader->attributeLocation("a_uv");
        shader->setAttributeBuffer(pos_loc, GL_FLOAT, offsetof(VertexData, pos), 2, sizeof(VertexData));
        shader->setAttributeBuffer(uv_loc, GL_FLOAT, offsetof(VertexData, uv), 2, sizeof(VertexData));
        shader->enableAttributeArray(pos_loc);
        shader->enableAttributeArray(uv_loc);

        matrixUniformLocation = shader->uniformLocation("matrix");
        textureUniformLocation = shader->uniformLocation("image");
        texSizeUniformLocation = shader->uniformLocation("texSize");
    }

    {
        QMutexLocker locker(&dataMutex);
        if(nextDoneCallback) {
            currentRenderSettings->uploadData(*texture, *currentRenderSettings, nextBuffer);
            nextDoneCallback();
            nextDoneCallback = std::function<void()>();
        }
    }
    
    glDisable(GL_DEPTH_TEST);

    shader->bind();
    vertices.bind();
    shader->setUniformValue(matrixUniformLocation, fullMatrix);
    shader->setUniformValue(textureUniformLocation, 0);
    shader->setUniformValue(texSizeUniformLocation, QVector2D(textureWidth, textureHeight));
    texture->bind(0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    onDraw();
}


void EGLRenderWidget::updateMatrix() {
    fullMatrix = windowMatrix * viewMatrix * frameMatrix;
    update();
}

void EGLRenderWidget::updateViewMatrix() {
    viewMatrix.setToIdentity();
    viewMatrix.translate(-scrollX, scrollY);
    viewMatrix.scale(flipX ? -scale : scale, flipY ? -scale : scale);
    updateMatrix();
}

void EGLRenderWidget::resizeGL(int w, int h) {
    windowMatrix.setToIdentity();
    windowMatrix.scale(1.0f / float(w), 1.0f / float(h));

    updateMatrix();
}

void EGLRenderWidget::setFormat(int width, int height, int pixelformat) {
    frameWidth = width;
    frameHeight = height;
    pixelFormat = pixelformat;
    recreatePipeline = true;

    frameMatrix.setToIdentity();
    frameMatrix.scale(float(width), float(height));

    updateMatrix();
}

void EGLRenderWidget::setScale(float scale) {
    this->scale = scale;
    updateViewMatrix();
}

void EGLRenderWidget::setFlip(bool flipX, bool flipY) {
    this->flipX = flipX;
    this->flipY = flipY;
    updateViewMatrix();
}

void EGLRenderWidget::nextFrame(BufferWrapper const& buffer, std::function<void()> doneCallback) {
    QMutexLocker locker(&dataMutex);
    if(nextDoneCallback) {
      nextDoneCallback();
    }
    nextDoneCallback = doneCallback;
    nextBuffer = buffer;
    locker.unlock();
    update();
}

void EGLRenderWidget::resizeEvent(QResizeEvent* event) {
    emit resized();
    QOpenGLWidget::resizeEvent(event);
}

void EGLRenderWidget::setScroll(int x, int y) {
    scrollX = x;
    scrollY = y;
    updateViewMatrix();
}

void EGLRenderWidget::wheelEvent(QWheelEvent *event)
{
    // Center point is not currently used, can be 0,0
#if 0
    auto const point = 
        #if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
          event->position().toPoint()
        #else
          event->pos()
        #endif
    ;
#endif
    emit requestZoom(QPointF(0.0f, 0.0f), event->angleDelta().y() > 0);
}

void EGLRenderWidget::mousePressEvent(QMouseEvent *event)
{

    QPoint mousePos = event->pos();
    auto const inverted = fullMatrix.inverted();
    auto const relPoint = QPointF(float(mousePos.x()) / float(width()) * 2.0f - 1.0f,
                                  float(mousePos.y()) / float(height()) * 2.0f - 1.0f);
    auto const point = (inverted.map(relPoint) + QPointF(1.0f, 1.0f)) * 0.5f;

    emit clicked(QPointF(point.x() * frameWidth, point.y() * frameHeight));
}
