//
// Created by 24909 on 2020/8/10.
//

#ifndef YUVPLAYER_SHADERRGBA_H
#define YUVPLAYER_SHADERRGBA_H

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <math.h>
#include "GlobalContexts.h"
#include "LogUtils.h"
#include "ShaderUtils.h"
#include <string>

class ShaderRGBA {
public:
    ShaderRGBA(GlobalContexts *global_context);
    ~ShaderRGBA();
    GLuint LoadShader(GLenum type, const char *shaderSrc);
    GLuint LoadProgram(const char *vShaderStr, const char *fShaderStr);
    int createProgram();
    void render(unsigned char *data);
    void setImageSize(int width, int height);
    void setWindowSize(int width, int height);
    void initDefMatrix();
    void orthoM(float m[], int mOffset,
                float left, float right, float bottom, float top,
                float near, float far);

    float vertex_coords[12] = {//世界坐标
            -1, -1, 0, // left bottom
            1, -1, 0, // right bottom
            -1, 1, 0,  // left top
            1, 1, 0,   // right top
    };

    float fragment_coords[8] = {//纹理坐标
            0, 1,//left bottom
            1, 1,//right bottom
            0, 0,//left top
            1, 0,//right top
    };

    float matrix_scale[16];

    GLint gl_program;
    GLint gl_position;
    GLint gl_textCoord;
    GLint gl_uMatrix;

    GLint gl_image_width;
    GLint gl_image_height;
    GLint gl_window_width;
    GLint gl_window_height;

    GLuint gl_texture_id;
    std::string *vertex_shader_graphical = NULL;
    std::string *fragment_shader_graphical = NULL;

    uint8_t *rgba_data;
    GlobalContexts *context;
};


#endif //YUVPLAYER_SHADERRGBA_H
