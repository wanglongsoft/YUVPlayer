package com.soft.yuvplayer;

import android.Manifest;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;

import java.io.File;

public class MainActivity extends BaseActivity {

    public static final String TAG = "MainActivity";

    private final static String PATH = Environment.getExternalStorageDirectory() + File.separator
            + "filefilm" + File.separator + "raw_video_854_480.yuv";

    private final static int WIDTH = 854;
    private final static int HEIGHT = 480;

//    private final static String PATH = Environment.getExternalStorageDirectory() + File.separator
//            + "filefilm" + File.separator + "raw_video_720_1280.yuv";
//
//    private final static int WIDTH = 720;
//    private final static int HEIGHT = 1280;

    private Button mStartPlay;
    private Button mStopPlay;
    private SurfaceView m_surface_view;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        requestRunTimePermission(new String[]{
                        Manifest.permission.INTERNET,
                        Manifest.permission.READ_EXTERNAL_STORAGE,
                        Manifest.permission.WRITE_EXTERNAL_STORAGE},
                null);
        initCommon();
        initSurfaceView();
    }

    private void initCommon() {
        mStartPlay = findViewById(R.id.start_play);
        mStartPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                FunctionControl.getInstance().startPlayer(WIDTH, HEIGHT, PATH);
            }
        });

        mStopPlay = findViewById(R.id.stop_play);
        mStopPlay.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                FunctionControl.getInstance().stopPlayer();
            }
        });
    }

    private void initSurfaceView() {
        m_surface_view = findViewById(R.id.surface_view);
        m_surface_view.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                Log.d(TAG, "surfaceCreated: ");
                FunctionControl.getInstance().saveAssetManager(getAssets());
                FunctionControl.getInstance().setSurface(holder.getSurface());
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                Log.d(TAG, "surfaceChanged: ");
                FunctionControl.getInstance().setSurfaceSize(width, height);
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                Log.d(TAG, "surfaceDestroyed: ");
                FunctionControl.getInstance().releaseResources();
            }
        });
    }
}
