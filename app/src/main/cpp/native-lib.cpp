#include <jni.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <android/bitmap.h>
#include <android/log.h>

using namespace cv;
#define LOG_TAG "AndroidScalingalgorithmforimage"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)


extern "C" JNIEXPORT jstring JNICALL
Java_com_kpa_image_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_kpa_image_MainActivity_scaleBitmapByNearestNeighbor(JNIEnv *env, jobject thiz,
                                                             jobject inputBitmap,
                                                             jfloat scale) {
    // bitmap 转换为openCV的Mat
    AndroidBitmapInfo bitmapInfo;
    Mat inputMat;
    Mat outputMat;
    jobject outputBitmap;
    if (scale == 1.0) {
        return inputBitmap;
    }
    // 将Android中的Bitmap 对象转为OpenCV 的Mat 对象
    if (AndroidBitmap_getInfo(env, inputBitmap, &bitmapInfo) < 0) {
        LOGD("转换失败");
        return NULL;
    }
    LOGD("转换成功");
    LOGD("图片的格式 %d", bitmapInfo.format);
    void *inputPixels;
    //获取bitmap 的像素数据
    if (AndroidBitmap_lockPixels(env, inputBitmap, &inputPixels) < 0) {
        LOGD("获取像素数据失败");
        return NULL;
    }
    // 创建OpenCV的Mat对象
    LOGD("height = %d, width = %d", bitmapInfo.height, bitmapInfo.width);
    inputMat.create(bitmapInfo.height, bitmapInfo.width, CV_8UC4);
    // 将Bitmap的像素数据拷贝到Mat对象中
    memcpy(inputMat.data, inputPixels, inputMat.rows * inputMat.step);
    //解锁Bitmap的像素数据
    AndroidBitmap_unlockPixels(env, inputBitmap);
    // 将图像缩小到原来的一半大小
//    int new_width = inputMat.cols / 2;
//    int new_height = inputMat.rows / 2;
// 根据缩放比例计算输出图像的宽高
    int new_width = static_cast<int>(inputMat.cols * scale);
    int new_height = static_cast<int>(inputMat.cols * scale);
    //创建输出的Mat对象
    outputMat.create(new_height, new_width, CV_8UC4);
    // 遍历每个像素
    for (int i = 0; i < new_height; i++) {
        for (int j = 0; j < new_width; j++) {
            //获取原始像素位置
            int orig_i = static_cast<int>(i / scale);
            int orig_j = static_cast<int>(j / scale);
            //将原始像素拷贝到输出Mat对象中
            outputMat.at<Vec4b>(i, j) = inputMat.at<Vec4b>(orig_i, orig_j);
        }
    }
    LOGD("图像缩放完成");
    jclass bitmapCls = env->FindClass("android/graphics/Bitmap");
    jmethodID createBitmapMethod = env->GetStaticMethodID(bitmapCls, "createBitmap",
                                                          "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    jstring configName = env->NewStringUTF("ARGB_8888");
    jclass bitmapConfigClass = env->FindClass("android/graphics/Bitmap$Config");
    jobject java_bitmap_config = env->CallStaticObjectMethod(bitmapConfigClass,
                                                             env->GetStaticMethodID(
                                                                     bitmapConfigClass, "valueOf",
                                                                     "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;"),
                                                             configName);
    outputBitmap = env->CallStaticObjectMethod(bitmapCls,
                                               createBitmapMethod,
                                               new_width,
                                               new_height,
                                               java_bitmap_config);

    LOGD("将OpenCV的Mat对象转换成Java中的Bitmap对象完成");
    void *outputPixels;
    int result = AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels);
    if (result < 0) {
        LOGD("outputPixels执行失败 %d", result);
        return NULL;
    }
    LOGD("outputPixels执行完成");
    memcpy(outputPixels, outputMat.data, outputMat.rows * outputMat.step);
    AndroidBitmap_unlockPixels(env, outputBitmap);
    LOGD("执行完成");
    // 返回Java中的Bitmap对象
    return outputBitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_kpa_image_MainActivity_scaleBitmapByBilinear(JNIEnv *env, jobject thiz,
                                                      jobject input_bitmap, jfloat scale) {
    return input_bitmap;
}
extern "C"
JNIEXPORT jobject JNICALL
Java_com_kpa_image_MainActivity_scaleBitmapByBicubic(JNIEnv *env, jobject thiz,
                                                     jobject input_bitmap, jfloat scale) {
    return input_bitmap;
}