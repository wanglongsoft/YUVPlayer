#include <jni.h>
#include <string>
#include <pthread.h>
#include <android/native_window_jni.h> // 是为了 渲染到屏幕支持的
#include <android/asset_manager_jni.h>

#include "LogUtils.h"
#include "GlobalContexts.h"
#include "YUVPlayer.h"

YUVPlayer *player = NULL;
ANativeWindow * nativeWindow = NULL;
GlobalContexts *global_context = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

extern "C"
JNIEXPORT void JNICALL
Java_com_soft_yuvplayer_FunctionControl_setSurface(JNIEnv *env, jobject thiz, jobject surface) {
    // TODO: implement setSurface()
    LOGD("setSurface in");
    pthread_mutex_lock(&mutex);

    if (nativeWindow) {
        ANativeWindow_release(nativeWindow);
        nativeWindow = 0;
    }

    // 创建新的窗口用于视频显示
    nativeWindow = ANativeWindow_fromSurface(env, surface);
    if(NULL == global_context) {
        global_context = new GlobalContexts();
    }
    global_context->nativeWindow = nativeWindow;
    pthread_mutex_unlock(&mutex);
    LOGD("setSurface out");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_soft_yuvplayer_FunctionControl_setSurfaceSize(JNIEnv *env, jobject thiz, jint width,
                                                       jint height) {
    // TODO: implement setSurfaceSize()
    LOGD("setSurfaceSize in");
    pthread_mutex_lock(&mutex);
    if(NULL == global_context) {
        global_context = new GlobalContexts();
    }
    global_context->gl_window_width = width;
    global_context->gl_window_height = height;
    pthread_mutex_unlock(&mutex);
    LOGD("setSurfaceSize out");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_soft_yuvplayer_FunctionControl_saveAssetManager(JNIEnv *env, jobject thiz,
                                                         jobject manager) {
    // TODO: implement saveAssetManager()
    LOGD("saveAssetManager in");
    pthread_mutex_lock(&mutex);
    AAssetManager *mgr = AAssetManager_fromJava(env, manager);
    if(NULL == global_context) {
        global_context = new GlobalContexts();
    }
    global_context->assetManager = mgr;
    pthread_mutex_unlock(&mutex);
    LOGD("saveAssetManager out");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_soft_yuvplayer_FunctionControl_startPlayer(JNIEnv *env, jobject thiz, jint width,
                                                    jint height, jstring file_path) {
    // TODO: implement startPlayer()
    LOGD("startPlayer in");
    const char *data = env->GetStringUTFChars(file_path, NULL);
    if(NULL == global_context) {
        global_context = new GlobalContexts();
    }

    global_context->gl_video_width = width;
    global_context->gl_video_height = height;

    if(NULL == player) {
        player = new YUVPlayer(global_context, data);
    }
    player->startPlayer();
    env->ReleaseStringUTFChars(file_path, data);
    LOGD("startPlayer out");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_soft_yuvplayer_FunctionControl_stopPlayer(JNIEnv *env, jobject thiz) {
    // TODO: implement stopPlayer()
    if(NULL != player) {
        player->stopPlayer();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_soft_yuvplayer_FunctionControl_releaseResources(JNIEnv *env, jobject thiz) {
    // TODO: implement releaseResources()
    pthread_mutex_lock(&mutex);
    if(NULL != player) {
        delete player;
        player = NULL;
    }
    if(NULL != global_context) {
        delete global_context;
        global_context = NULL;
    }
    pthread_mutex_unlock(&mutex);
}