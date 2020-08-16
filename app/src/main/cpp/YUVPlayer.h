//
// Created by 24909 on 2020/8/10.
//

#ifndef YUVPLAYER_YUVPLAYER_H
#define YUVPLAYER_YUVPLAYER_H

#include "LogUtils.h"
#include "GlobalContexts.h"
#include "EGLDisplayYUV.h"
#include "libyuv/convert.h"
#include "libyuv/scale.h"
#include "ShaderYUV.h"
#include "ShaderRGBA.h"
#include <pthread.h>
#include <unistd.h>

extern "C" {
    #include "libavutil/imgutils.h"
    #include "libswscale/swscale.h"
}

//支持两种方式渲染YUV数据, 第一种：yuv直接传给Shader，yuv转rgb在GPU完成
//对应函数：startYUVPlayerTask()
//支持两种方式渲染YUV数据, 第二种：yuv数据转rgba，再传给Shader
//对应函数：startRGBAPlayerTask()
class YUVPlayer {
public:
    YUVPlayer(GlobalContexts *global_context, const char *src_file_name);
    ~YUVPlayer();
    void startPlayer();
    void stopPlayer();
    void startRGBAPlayerTask();
    void startYUVPlayerTask();
private:
    void adjustVideoScaleResolution();//调整缩放后的视频宽高
    int loadFrame(FILE *yuv_file, uint32_t frmIdx, uint16_t width, uint16_t height);
    int yuv_data_save(FILE *fd, unsigned char *data[], int width, int height);

    GlobalContexts *global_context = NULL;
    EGLDisplayYUV *eglDisplayYuv = NULL;
    ShaderYUV * shaderYuv = NULL;
    ShaderRGBA * shaderRgba = NULL;

    char *data_source = NULL;
    pthread_t pid_start_player;
    FILE *src_fd = NULL;
    FILE *dst_fd = NULL;
    const char *dst_file_name = "/storage/emulated/0/filefilm/dst_video_854_480.yuv";
    uint8_t *frame_data[3] = {NULL};
    uint8_t *scale_frame_data[3] = {NULL};;
    uint8_t *rgba_data = NULL;
    uint64_t lumaPlaneSize;
    uint64_t chromaPlaneSize;
    uint64_t file_size = 0;
    int totalFrames = 0;
    int video_width = 0;
    int video_height = 0;
    bool is_start_play;
    int scale_video_width;//视频宽需要是8(2的3次幂)的倍数，否则视频错乱，绿条等等
    int scale_video_height;//视频高最好是2的倍数
};


#endif //YUVPLAYER_YUVPLAYER_H
