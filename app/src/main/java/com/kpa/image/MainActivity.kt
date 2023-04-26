package com.kpa.image

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Bundle
import android.util.Log
import android.widget.SeekBar
import androidx.appcompat.app.AppCompatActivity
import com.kpa.image.databinding.ActivityMainBinding
import com.kpa.image.utils.ScalingAlgorithmType
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.MainScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.text.DecimalFormat

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private var currentType = ScalingAlgorithmType.NEAREST_NEIGHBOR
    private val TAG = "MainActivity-Scale"
    private var lastScale = 0.0F
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        var bitmap = BitmapFactory.decodeResource(resources, R.drawable.test)
        val df = DecimalFormat("#.#")
        var value = (binding.seekbar.progress / 100.0) ?: 0.1
        var scale = df.format(value).toFloat()
        if (scale <= 0.0f) {
            scale = 0.1f
        }
        if (lastScale != scale) {
            lastScale = scale
        }
        binding.seekbar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                var value = (progress / 100.0) ?: 0.1
                var scale = df.format(value).toFloat()
                if (scale <= 0.0f) {
                    scale = 0.1f
                }
                if (lastScale != scale) {
                    Log.d("MainActivity", "scale = $scale")
                    binding.scale.text = "当前缩放：$scale"
                    updateImg(bitmap, scale.toFloat(), currentType)
                    lastScale = scale
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
            }
        })
        binding.group.setOnCheckedChangeListener { group, checkedId ->
            when (checkedId) {
                R.id.radio1 -> {
                    currentType = ScalingAlgorithmType.NEAREST_NEIGHBOR
                }

                R.id.radio2 -> {
                    currentType = ScalingAlgorithmType.BILINEAR_INTERPOLATION
                }

                R.id.radio3 -> {
                    currentType = ScalingAlgorithmType.BICUBIC_INTERPOLATION
                }
            }
            updateImg(bitmap, lastScale, currentType)
        }
    }

    private fun updateImg(bitmap: Bitmap, scale: Float, type: ScalingAlgorithmType) {
        Log.d(TAG, "scale = $scale")
        MainScope().launch(Dispatchers.IO) {
            var scaleBitmap = when (type) {
                ScalingAlgorithmType.NEAREST_NEIGHBOR -> {
                    scaleBitmapByNearestNeighbor(bitmap, scale)
                }

                ScalingAlgorithmType.BILINEAR_INTERPOLATION -> {
                    scaleBitmapByBilinear(bitmap, scale)
                }

                else -> {
                    scaleBitmapByBicubic(bitmap, scale)
                }
            }

            withContext(Dispatchers.Main) {
                if (scaleBitmap != null) {
                    binding.imageView.setImageBitmap(scaleBitmap)
                }
            }
        }
    }

    /**
     * A native method that is implemented by the 'image' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    /**
     * 缩放图像
     * @param inputBitmap 目标图像
     * @param scale 缩放因子
     * @return 处理后的图片
     */
    external fun scaleBitmapByNearestNeighbor(inputBitmap: Bitmap, scale: Float): Bitmap

    external fun scaleBitmapByBilinear(inputBitmap: Bitmap, scale: Float): Bitmap

    external fun scaleBitmapByBicubic(inputBitmap: Bitmap, scale: Float): Bitmap

    companion object {
        // Used to load the 'image' library on application startup.
        init {
            System.loadLibrary("image")
        }
    }
}
