//
// Created by 24909 on 2020/8/10.
//

#include "YUVPlayer.h"

// TODO 异步 函数指针
void * customTaskStartThread(void * pVoid) {
    YUVPlayer * player = static_cast<YUVPlayer *>(pVoid);
    player->startYUVPlayerTask();
    return 0;
}

YUVPlayer::YUVPlayer(GlobalContexts *global_context, const char *src_file_name) {
    this->global_context = global_context;
    this->data_source = new char[strlen(src_file_name) + 1];
    strcpy(this->data_source, src_file_name);
    this->video_width = this->global_context->gl_video_width;
    this->video_height = this->global_context->gl_video_height;

    adjustVideoScaleResolution();

    frame_data[0] = new uint8_t[this->video_width
                                * this->video_height];
    frame_data[1] = new uint8_t[this->video_width
                                * this->video_height / 4];
    frame_data[2] = new uint8_t[this->video_width
                                * this->video_height / 4];

    scale_frame_data[0] = new uint8_t[scale_video_width
                                * scale_video_height];
    scale_frame_data[1] = new uint8_t[scale_video_width
                                * scale_video_height / 4];
    scale_frame_data[2] = new uint8_t[scale_video_width
                                * scale_video_height / 4];

    rgba_data = new uint8_t[this->video_width
                            * this->video_height * 4];

    lumaPlaneSize = this->video_width
                    * this->video_height;
    chromaPlaneSize = lumaPlaneSize >> 2;

    LOGD("YUVPlayer init video_width: %d, video_height: %d", this->video_width,
         this->video_height);

    LOGD("YUVPlayer init scale_video_width: %d, scale_video_height: %d", this->scale_video_width,
         this->scale_video_height);

//    dst_fd = fopen(this->dst_file_name, "wb");
//    if(NULL == dst_fd) {
//        LOGD("fopen file fail: %s", this->dst_file_name);
//        return;
//    }

    src_fd = fopen(this->data_source, "rb");
    if(NULL == src_fd) {
        LOGD("fopen file fail: %s", this->data_source);
        return;
    }
    LOGD("fopen file success: %s", this->data_source);

    fseek(src_fd, 0, SEEK_END);
    file_size = ftell(src_fd);

    LOGD("ftell file_size: %d", file_size);
    LOGD("ftell lumaPlaneSize: %d", lumaPlaneSize);
    LOGD("ftell chromaPlaneSize: %d", chromaPlaneSize);

    totalFrames = file_size / (lumaPlaneSize + chromaPlaneSize + chromaPlaneSize);

    this->is_start_play = false;

    LOGD("YUVPlayer init success, totalFrames: %d", totalFrames);
}

YUVPlayer::~YUVPlayer() {
    if (this->data_source != NULL) {
        delete this->data_source;
        this->data_source = NULL;
    }
    if (this->eglDisplayYuv != NULL) {
        delete this->eglDisplayYuv;
        this->eglDisplayYuv = NULL;
    }
    if (this->shaderYuv != NULL) {
        delete this->shaderYuv;
        this->shaderYuv = NULL;
    }
    if (this->frame_data[0] != NULL) {
        delete this->frame_data[0];
        this->frame_data[0] = NULL;
    }
    if (this->frame_data[1] != NULL) {
        delete this->frame_data[1];
        this->frame_data[1] = NULL;
    }
    if (this->frame_data[2] != NULL) {
        delete this->frame_data[2];
        this->frame_data[2] = NULL;
    }

    if (this->scale_frame_data[0] != NULL) {
        delete this->scale_frame_data[0];
        this->scale_frame_data[0] = NULL;
    }
    if (this->scale_frame_data[1] != NULL) {
        delete this->scale_frame_data[1];
        this->scale_frame_data[1] = NULL;
    }
    if (this->scale_frame_data[2] != NULL) {
        delete this->scale_frame_data[2];
        this->scale_frame_data[2] = NULL;
    }

    if (this->rgba_data != NULL) {
        delete this->rgba_data;
        this->rgba_data = NULL;
    }

    if (this->src_fd != NULL) {
        fclose(this->src_fd);
        this->src_fd = NULL;
    }

//    if (this->dst_fd != NULL) {
//        fflush(this->dst_fd);
//        fclose(this->dst_fd);
//        this->dst_fd = NULL;
//    }
}

void YUVPlayer::startPlayer() {
    if(NULL == src_fd) {
        LOGD("startPlayer fail, src_fd == NULL");
        return;
    }

    if(this->is_start_play) {
        LOGD("startPlayer, is_start_play == true");
        return;
    }

    this->is_start_play = true;
    pthread_create(&pid_start_player, 0, customTaskStartThread, this);
}

void YUVPlayer::stopPlayer() {
    LOGD("stopPlayer in");
    this->is_start_play = false;
    LOGD("stopPlayer out");
}

void YUVPlayer::startRGBAPlayerTask() {
    LOGD("startPlayerTask in");
    if(NULL != eglDisplayYuv) {
        delete eglDisplayYuv;
        eglDisplayYuv = NULL;
    }
    eglDisplayYuv = new EGLDisplayYUV(this->global_context->nativeWindow, this->global_context);
    eglDisplayYuv->eglOpen();

    if(NULL != shaderRgba) {
        delete shaderRgba;
        shaderRgba = NULL;
    }

    this->global_context->gl_image_width = this->video_width;
    this->global_context->gl_image_height = this->video_height;
    shaderRgba = new ShaderRGBA(this->global_context);
    shaderRgba->createProgram();

    SwsContext *sws_ctx = sws_getContext(
            this->video_width, this->video_height, AV_PIX_FMT_YUV420P,
            this->video_width, this->video_height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, NULL, NULL, NULL
    );

    if(sws_ctx == NULL) {
        LOGD("sws_getContext fail, return");
        return;
    }

    uint8_t *local_rgba_data[4];
    int rgba_linesizes[4];
    int frame_linesize[3] = {this->video_width, this->video_width >> 1,
                             this->video_width >> 1};
    int alloc_ret = av_image_alloc(local_rgba_data, rgba_linesizes, this->video_width,
                                   this->video_height, AV_PIX_FMT_RGBA, 1);

    if(alloc_ret < 0) {
        LOGD("av_image_alloc fail, return");
        return;
    }

    LOGD("frame_linesize : %d, %d, %d", frame_linesize[0], frame_linesize[1],
         frame_linesize[2]);

    LOGD("rgba_linesizes : %d, %d, %d, %d", rgba_linesizes[0], rgba_linesizes[1],
         rgba_linesizes[2], rgba_linesizes[3]);

    for (int i = 0; i < totalFrames; ++i) {
        loadFrame(src_fd, i, this->video_width, this->video_height);

        sws_scale(sws_ctx, frame_data,
                  frame_linesize, 0, this->video_height, local_rgba_data, rgba_linesizes);
        shaderRgba->render(local_rgba_data[0]);

//        libyuv::I420ToABGR(//转换数据应该是ABGB，测试结果是：RGBA
//                frame_data[0], this->video_width,
//                frame_data[1], this->video_width >> 1,
//                frame_data[2], this->video_width >> 1,
//                this->rgba_data, this->video_width << 2,
//                this->video_width, this->video_height);
//        shaderRgba->render(this->rgba_data);

        usleep(30 * 1000);
        if(!this->is_start_play) {
            break;
        }
    }
    this->is_start_play = false;
    sws_freeContext(sws_ctx);
    av_freep(&local_rgba_data[0]);
    LOGD("startPlayerTask out");
}

void YUVPlayer::startYUVPlayerTask() {
    LOGD("startYUVPlayerTask in");
    if(NULL != eglDisplayYuv) {
        delete eglDisplayYuv;
        eglDisplayYuv = NULL;
    }

    eglDisplayYuv = new EGLDisplayYUV(this->global_context->nativeWindow, this->global_context);
    eglDisplayYuv->eglOpen();


    if(NULL != shaderYuv) {
        delete shaderYuv;
        shaderYuv = NULL;
    }

    this->global_context->gl_video_width = this->scale_video_width;
    this->global_context->gl_video_height = this->scale_video_height;
    shaderYuv = new ShaderYUV(this->global_context);
    shaderYuv->createProgram();

    for (int i = 0; i < totalFrames; ++i) {
        loadFrame(src_fd, i, this->video_width, this->video_height);
        if((this->scale_video_width == this->video_width)
            && (this->scale_video_height == this->video_height)) {
            shaderYuv->render(this->frame_data);
        } else {
            libyuv::I420Scale(
                    this->frame_data[0], this->video_width,
                    this->frame_data[1], this->video_width >> 1,
                    this->frame_data[2], this->video_width >> 1,
                    this->video_width, this->video_height,
                    this->scale_frame_data[0], this->scale_video_width,
                    this->scale_frame_data[1], this->scale_video_width >> 1,
                    this->scale_frame_data[2], this->scale_video_width >> 1,
                    this->scale_video_width, this->scale_video_height,
                    libyuv::kFilterNone
            );
            shaderYuv->render(this->scale_frame_data);
        }
        usleep(20 * 1000);
        if(!this->is_start_play) {
            break;
        }
    }
    this->is_start_play = false;
    LOGD("startYUVPlayerTask out");
}

int YUVPlayer::loadFrame(FILE *yuv_file, uint32_t frmIdx, uint16_t width, uint16_t height) {
    LOGD("loadFrame frmIdx : %d", frmIdx);
    uint64_t y_data_offset;
    uint64_t u_data_offset;
    uint64_t v_data_offset;
    uint32_t result;
    uint32_t dwInFrameSize = 0;
    int anFrameSize[3] = {};

    FILE *y_data_fd = yuv_file;
    FILE *u_data_fd = yuv_file;
    FILE *v_data_fd = yuv_file;
    uint8_t *frame_data_y = frame_data[0];
    uint8_t *frame_data_u = frame_data[1];
    uint8_t *frame_data_v = frame_data[2];

    dwInFrameSize = width * height * 3 / 2;
    anFrameSize[0] = width * height;
    anFrameSize[1] = anFrameSize[2] = width * height / 4;

    //当前帧在文件中的偏移量：当前index * 每一帧的大小
    y_data_offset = (uint64_t) dwInFrameSize * frmIdx;
    u_data_offset = (uint64_t) dwInFrameSize * frmIdx + anFrameSize[0];
    v_data_offset = (uint64_t) dwInFrameSize * frmIdx + anFrameSize[0] + anFrameSize[1];

    //seek到偏移处
    result = fseek(y_data_fd, y_data_offset, SEEK_SET);
    if (result == -1)
    {
        LOGD("loadframe y_data_offset fail");
        return -1;
    }
    //把当前帧的Y、U、V数据分别读取到对应的数组中
    for (int i = 0; i < height; ++i) {
        fread(frame_data_y, 1, width, y_data_fd);
        frame_data_y += width;
    }

    result = fseek(u_data_fd, u_data_offset, SEEK_SET);
    if (result == -1)
    {
        LOGD("loadframe u_data_offset fail");
        return -1;
    }
    for (int i = 0; i < height / 2; ++i) {
        fread(frame_data_u, 1, width / 2, u_data_fd);
        frame_data_u += width / 2;
    }

    result = fseek(v_data_fd, v_data_offset, SEEK_SET);
    if (result == -1)
    {
        LOGD("loadframe v_data_offset fail");
        return -1;
    }
    for (int i = 0; i < height / 2; ++i) {
        fread(frame_data_v, 1, width / 2, v_data_fd);
        frame_data_v += width / 2;
    }
    return 0;
}

int YUVPlayer::yuv_data_save(FILE *fd, unsigned char **data, int width, int height) {
    //pitchY暂时设置width，针对AVFrame取linesize[0]
    uint32_t pitchY = width;
    uint32_t pitchU = width >> 1;
    uint32_t pitchV = width >> 1;
    uint8_t* avY = data[0];
    uint8_t* avU = data[1];
    uint8_t* avV = data[2];

    //YUV数据之Y
    for (int i = 0; i < height; i++) {
        fwrite(avY, 1, width, fd);
        avY += pitchY;
    }

    //YUV数据之U
    for (int i = 0; i < height >> 1; i++) {
        fwrite(avU, 1, width >> 1, fd);
        avU += pitchU;
    }

    //YUV数据之V
    for (int i = 0; i < height >> 1; i++) {
        fwrite(avV, 1, width >> 1, fd);
        avV += pitchV;
    }
    return 0;
}

void YUVPlayer::adjustVideoScaleResolution() {
    if(this->video_width % 8 != 0) {
        this->scale_video_width = ((this->video_width / 8) + 1) * 8;
    } else {
        this->scale_video_width = this->video_width;
    }
    if(this->video_height % 2 != 0) {
        this->scale_video_height = ((this->video_height / 2) + 1) * 2;
    } else {
        this->scale_video_height = this->video_height;
    }
}
