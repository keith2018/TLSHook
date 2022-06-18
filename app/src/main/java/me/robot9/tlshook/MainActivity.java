package me.robot9.tlshook;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


public class MainActivity extends AppCompatActivity {

    // Used to load the 'tlshook' library on application startup.
    static {
        System.loadLibrary("tlshook");
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        TextView tv = findViewById(R.id.sample_text);

        // start hook
        boolean hookOk = hookTestStart();
        tv.setText(hookOk ? "Hook Success" : "Hook Failed");

        // test hook via GLSurfaceView
        GLSurfaceView surfaceView = findViewById(R.id.surface_view);
        surfaceView.setEGLContextClientVersion(2);
        surfaceView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                GLES20.glClearColor(1.f, 0.f, 0.f, 1.f);
                GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
            }
        });
        surfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    @Override
    protected void onDestroy() {
        // destroy hook
        hookTestStop();
        super.onDestroy();
    }

    public native boolean hookTestStart();

    public native void hookTestStop();
}