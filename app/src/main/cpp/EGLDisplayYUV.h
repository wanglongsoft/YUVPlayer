//
// Created by 24909 on 2020/8/10.
//

#ifndef YUVPLAYER_EGLDISPLAYYUV_H
#define YUVPLAYER_EGLDISPLAYYUV_H

#include "LogUtils.h"
#include "GlobalContexts.h"
#include <EGL/egl.h>

class EGLDisplayYUV {
public:
    EGLDisplayYUV(ANativeWindow * window, GlobalContexts *context);
    ~EGLDisplayYUV();
    int eglOpen();
    int eglClose();
    ANativeWindow * nativeWindow;
    GlobalContexts *global_context;
};


#endif //YUVPLAYER_EGLDISPLAYYUV_H
