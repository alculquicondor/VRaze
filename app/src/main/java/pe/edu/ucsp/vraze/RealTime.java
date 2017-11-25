package pe.edu.ucsp.vraze;

import android.util.Log;

import com.google.android.gms.games.multiplayer.realtime.RealTimeMessage;
import com.google.android.gms.games.multiplayer.realtime.RealTimeMessageReceivedListener;

import java.nio.ByteBuffer;
import java.nio.FloatBuffer;

/**
 * Created by zafiron on 17/11/17.
 */

public class RealTime implements RealTimeMessageReceivedListener {

  private static final RealTime instance = new RealTime();
  private static final String TAG = "RealTimeMessage";
  private long nativeVRaze;

  private RealTime() {
  }

  public static RealTime getInstance() {
    return instance;
  }

  static {
    System.loadLibrary("vraze_jni");
  }

  public void setNativeVRaze(long ptr) {
    nativeVRaze = ptr;
  }

  private native void nativeMoveCar(long controllerPaintJptr, float x, float y, float dir0, float dir1);

  @Override
  public void onRealTimeMessageReceived(RealTimeMessage rtm) {
    byte[] buf = rtm.getMessageData();
    String sender = rtm.getSenderParticipantId();
    FloatBuffer fb = ByteBuffer.wrap(buf).asFloatBuffer();
    float[] data = new float[fb.capacity()];
    fb.get(data);

    Log.d(TAG, "x: " + data[0]);
    Log.d(TAG, "y: " + data[1]);
    Log.d(TAG, "dir: " + data[2] + " - " + data[3]);
    if (nativeVRaze != 0)
      nativeMoveCar(nativeVRaze, data[0], data[1], data[2], data[3]);
  }
}
