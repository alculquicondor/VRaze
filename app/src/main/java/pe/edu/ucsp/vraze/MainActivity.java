/*
 * Copyright 2017 Aldo Culquicondor
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package pe.edu.ucsp.vraze;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;


import com.google.vr.ndk.base.AndroidCompat;
import com.google.vr.ndk.base.GvrLayout;


import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends Activity {

  private static final String TAG = "MainActivity";
  static {
    System.loadLibrary("vraze_jni");
  }

  private long nativeVRaze;

  private GvrLayout gvrLayout;
  private GLSurfaceView surfaceView;

  private final Runnable pauseNativeRunnable =
      new Runnable() {
        @Override
        public void run() {
          nativeOnPause(nativeVRaze);
        }
      };

  private final Runnable resumeNativeRunnable =
      new Runnable() {
        @Override
        public void run() {
          nativeOnResume(nativeVRaze);
        }
      };

  public static void messageMe(float x, float y, float dir_x, float dir_y) {
    //Log.d(TAG,"x: " + String.valueOf(x) + " y: " + String.valueOf(y));
    //Log.d(TAG,"d0: " + String.valueOf(dir_x) + " d1: " + String.valueOf(dir_y));

    float [] data = {x,y,dir_x,dir_y};
    if(Menu.mMultiplayer)
    Menu.SendToPlayers(data,false);
  }

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setImmersiveSticky();
    getWindow()
        .getDecorView()
        .setOnSystemUiVisibilityChangeListener(
            new View.OnSystemUiVisibilityChangeListener() {
              @Override
              public void onSystemUiVisibilityChange(int visibility) {
                if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                  setImmersiveSticky();
                }
              }
            });
    AndroidCompat.setVrModeEnabled(this, true);

    Intent intent = getIntent();
    boolean multiplayer = intent.getBooleanExtra("isMultiplayer", false);
    int playerNumber = intent.getIntExtra("playerNumber", 0);

    gvrLayout = new GvrLayout(this);
    if (gvrLayout.setAsyncReprojectionEnabled(true)) {
      Log.d(TAG, "Successfully enabled async reprojection.");
      AndroidCompat.setSustainedPerformanceMode(this, true);
    } else {
      Log.w(TAG, "Failed to enable async reprojection.");
    }

    surfaceView = new GLSurfaceView(this);
    surfaceView.setEGLContextClientVersion(3);
    surfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0);
    surfaceView.setRenderer(renderer);

    surfaceView.setPreserveEGLContextOnPause(true);

    gvrLayout.setPresentationView(surfaceView);

    setContentView(gvrLayout);

    AssetManager assetManager = getResources().getAssets();

    nativeVRaze =
        nativeOnCreate(getClass().getClassLoader(), getApplicationContext(), assetManager,
            gvrLayout.getGvrApi().getNativeGvrContext(), multiplayer, playerNumber);

    RealTime.getInstance().setNativeVRaze(nativeVRaze);

    getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);


  }

  @Override
  protected void onDestroy() {
    super.onDestroy();
    gvrLayout.shutdown();
    nativeOnDestroy(nativeVRaze);
    nativeVRaze = 0;
  }

  @Override
  protected void onPause() {
    surfaceView.queueEvent(pauseNativeRunnable);
    surfaceView.onPause();
    gvrLayout.onPause();
    super.onPause();
  }

  @Override
  protected void onResume() {
    super.onResume();
    gvrLayout.onResume();
    surfaceView.onResume();
    surfaceView.queueEvent(resumeNativeRunnable);
  }

  @Override
  public void onBackPressed() {
    super.onBackPressed();
    gvrLayout.onBackPressed();
  }

  @Override
  public void onWindowFocusChanged(boolean hasFocus) {
    super.onWindowFocusChanged(hasFocus);
    if (hasFocus) {
      setImmersiveSticky();
    }
  }

  @Override
  public boolean dispatchKeyEvent(KeyEvent event) {
    // Avoid accidental volume key presses while the phone is in the VR headset.
    return event.getKeyCode() == KeyEvent.KEYCODE_VOLUME_UP ||
        event.getKeyCode() == KeyEvent.KEYCODE_VOLUME_DOWN ||
        super.dispatchKeyEvent(event);
  }

  private void setImmersiveSticky() {
    getWindow()
        .getDecorView()
        .setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
  }

  private final GLSurfaceView.Renderer renderer =
      new GLSurfaceView.Renderer() {
        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
          nativeOnSurfaceCreated(nativeVRaze);
        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
          nativeOnSurfaceChanged(width, height, nativeVRaze);
        }

        @Override
        public void onDrawFrame(GL10 gl) {
          nativeOnDrawFrame(nativeVRaze);
        }
      };

  private native long nativeOnCreate(ClassLoader appClassLoader, Context context,
      AssetManager assetManager, long gvrContextPtr, boolean isMultiplayer, int playerNumber);

  private native void nativeOnDestroy(long controllerPaintJptr);

  private native void nativeOnResume(long controllerPaintJptr);

  private native void nativeOnPause(long controllerPaintJptr);

  private native void nativeOnSurfaceCreated(long controllerPaintJptr);

  private native void nativeOnSurfaceChanged(int width, int height, long controllerPaintJptr);

  private native void nativeOnDrawFrame(long controllerPaintJptr);



}
