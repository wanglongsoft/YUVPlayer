# YUVPlayer
基于Android平台的YUV视频原始数据播放器
#### 编译环境
FFmpeg版本： **4.2.2**
NDK版本：**r17c**
#### 运行环境
* x86(模拟器)
* arm64-v8a(64位手机)
#### 功能点
* 从文件中读取YUV数据，需要调用者提供YUV数据的宽高信息
* 在NDK环境下，使用OpenGLES3.0渲染视频
* 集成libyuv和FFmpeg库，进行缩放和格式转换
* 支持CPU和GPU(推荐)进行YUV转RGBA进行视频渲染
* NDK环境下，开启视频渲染子线程
#### 运行效果(工程根目录images文件夹)
![渲染效果](https://github.com/wanglongsoft/YUVPlayer/tree/master/image/menu.jpg)