#include <jni.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <android/bitmap.h>
#include <android/log.h>

using namespace cv;
#define LOG_TAG "AndroidScalingalgorithmforimage"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)


jobject createBitmap(JNIEnv *env, int new_width, int new_height);

extern "C" JNIEXPORT jstring JNICALL
Java_com_kpa_image_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

/**
 * 创建目标位图
 * @param env
 * @param new_width 目标位图 宽
 * @param new_height 高
 * @return 目标位图
 */
jobject createBitmap(JNIEnv *env, int new_width, int new_height) {
    jobject outputBitmap;
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
    return outputBitmap;
}
/**
 * 最近邻插值算法
 */
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
    //获取bitmap 的像素数据（锁定像素缓冲区并检索指向它的指针。这样做可确保像素在应用调用 AndroidBitmap_unlockPixels 之前不会移动）
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
    outputBitmap = createBitmap(env, new_width, new_height);
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
/**
 * 双线性插值
 */
extern "C"
JNIEXPORT jobject JNICALL
Java_com_kpa_image_MainActivity_scaleBitmapByBilinear(JNIEnv *env, jobject thiz,
                                                      jobject input_bitmap, jfloat scale) {
    if (scale == 1.0) {
        return input_bitmap;
    }
    // 获取源位图和目标位图的信息
    AndroidBitmapInfo bitmapInfo;
    void *pixels = nullptr;
    //将Android中的Bitmap 对象转为OpenCV 的Mat 对象
    if (AndroidBitmap_getInfo(env, input_bitmap, &bitmapInfo) < 0) {
        return NULL;
    }
    //获取原位图的像素
    if (AndroidBitmap_lockPixels(env, input_bitmap, &pixels) < 0) {
        return NULL;
    }
    //解锁Bitmap的像素数据
    AndroidBitmap_unlockPixels(env, input_bitmap);
    LOGD("height = %d, width = %d", bitmapInfo.height, bitmapInfo.width);
    //按比例计算目标图像的宽高
    int newWidth = static_cast<int>(bitmapInfo.width * scale);
    int newHeight = static_cast<int>(bitmapInfo.height * scale);
    LOGD("newHeight = %d, newWidth = %d", newHeight, newWidth);
    // 原位图像素数组
    uint32_t *srcPixels = static_cast<uint32_t *>(pixels);
    // 目标位图像素数组
    uint32_t *dstPixels = new uint32_t[newWidth * newHeight];
    //位图的宽度和新位图的宽度都是从1开始计数的，而像素索引是从0开始计数的。因此，要计算像素索引之间的比率，需要将宽度减一。
    float xRatio = static_cast<float>(bitmapInfo.width - 1) / static_cast<float>(newWidth - 1);
    float yRatio = static_cast<float>(bitmapInfo.height - 1) / static_cast<float>(newHeight - 1);
    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            float gx = x * xRatio;
            float gy = y * yRatio;
            int gxi = static_cast<int>(gx);
            int gyi = static_cast<int>(gy);
            float fracx = gx - gxi;
            float fracy = gy - gyi;

            uint32_t c00 = srcPixels[gyi * bitmapInfo.width + gxi];
            uint32_t c10 = srcPixels[gyi * bitmapInfo.width + gxi + 1];
            uint32_t c01 = srcPixels[(gyi + 1) * bitmapInfo.width + gxi];
            uint32_t c11 = srcPixels[(gyi + 1) * bitmapInfo.width + gxi + 1];

            int r = static_cast<int>((1 - fracx) * (1 - fracy) * (c00 >> 16 & 0xff) +
                                     fracx * (1 - fracy) * (c10 >> 16 & 0xff) +
                                     (1 - fracx) * fracy * (c01 >> 16 & 0xff) +
                                     fracx * fracy * (c11 >> 16 & 0xff));

            int g = static_cast<int>((1 - fracx) * (1 - fracy) * (c00 >> 8 & 0xff) +
                                     fracx * (1 - fracy) * (c10 >> 8 & 0xff) +
                                     (1 - fracx) * fracy * (c01 >> 8 & 0xff) +
                                     fracx * fracy * (c11 >> 8 & 0xff));

            int b = static_cast<int>((1 - fracx) * (1 - fracy) * (c00 & 0xff) +
                                     fracx * (1 - fracy) * (c10 & 0xff) +
                                     (1 - fracx) * fracy * (c01 & 0xff) +
                                     fracx * fracy * (c11 & 0xff));
            dstPixels[y * newWidth + x] = 0xff000000 | (r << 16) | (g << 8) | b;
        }
    }
    jobject newBitmap = createBitmap(env, newWidth, newHeight);
    LOGD("将OpenCV的Mat对象转换成Java中的Bitmap对象完成");
    void *outputPixels;
    int result = AndroidBitmap_lockPixels(env, newBitmap, &outputPixels);
    if (result < 0) {
        LOGD("outputPixels执行失败 %d", result);
        return NULL;
    }
    LOGD("outputPixels执行完成");
    memcpy(outputPixels, dstPixels, newWidth * newHeight * 4);
    AndroidBitmap_unlockPixels(env, newBitmap);
    LOGD("执行完成");
    return newBitmap;
}
/**
 * 双三次插值
 */
extern "C"
JNIEXPORT jobject JNICALL
Java_com_kpa_image_MainActivity_scaleBitmapByBicubic(JNIEnv *env, jobject thiz,
                                                     jobject input_bitmap, jfloat scale) {
    return input_bitmap;
}