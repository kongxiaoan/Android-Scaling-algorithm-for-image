package com.kpa.image

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Bundle
import android.widget.SeekBar
import androidx.appcompat.app.AppCompatActivity
import com.kpa.image.databinding.ActivityMainBinding
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.MainScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.text.DecimalFormat

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        var bitmap = BitmapFactory.decodeResource(resources, R.drawable.test)
        val df = DecimalFormat("#.#")

        binding.seekbar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                var value = progress / 100.0
                var scale = df.format(value)
                updateImg(bitmap, scale)
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
            }
        })
    }

    private fun updateImg(bitmap: Bitmap, scale: String) {
        MainScope().launch(Dispatchers.IO) {
            var scaleBitmap = scaleBitmap(bitmap, scale.toFloat())
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
    external fun scaleBitmap(inputBitmap: Bitmap, scale: Float): Bitmap

    companion object {
        // Used to load the 'image' library on application startup.
        init {
            System.loadLibrary("image")
        }
    }
}
