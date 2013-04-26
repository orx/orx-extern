package org.orx.lib;

import android.app.Activity;
import android.graphics.PixelFormat;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.WindowManager;

/**
    Orx Activity
*/
public class OrxActivity extends Activity {

    // Keep track of the paused state
    boolean mIsPaused = false;
    
    // Main components
    private OrxGLSurfaceView mSurface;

    // This is what Orx runs in. It invokes Orx_main(), eventually
    private Thread mOrxThread;

    // Setup
    protected void onCreate(Bundle savedInstanceState) {
        Log.v("Orx", "onCreate()");
        super.onCreate(savedInstanceState);
        
        getWindow().setFormat(PixelFormat.RGB_565);
        init();
    }
    
    @Override
    protected void onStart() {
    	super.onStart();
    	
    	getWindow().setFormat(PixelFormat.RGB_565);
    }
    
    private void init() {
    	if(getLayoutId() == 0 || getOrxGLSurfaceViewId() == 0) {
            // Set up the surface
            mSurface = new OrxGLSurfaceView(getApplication());
            setContentView(mSurface);
    	} else {
			setContentView(getLayoutId());
			mSurface = (OrxGLSurfaceView) findViewById(getOrxGLSurfaceViewId());
		}
    	
    	mSurface.setActivity(this);
    }

    protected int getLayoutId() {
		/*
		 * Override this if you want to use a custom layout
		 * return the layout id
		 */
		return 0;
	}
    
    protected int getOrxGLSurfaceViewId() {
		/*
		 * Override this if you want to use a custom layout
		 * return the OrxGLSurfaceView id
		 */
		return 0;
	}
    
    // Events
    protected void onPause() {
        Log.v("Orx", "onPause()");
        super.onPause();
        nativePause();
    }

    protected void onResume() {
        Log.v("Orx", "onResume()");
        super.onResume();
        nativeResume();
    }

    protected void onDestroy() {
        super.onDestroy();
        Log.v("Orx", "onDestroy()");
        // Send a quit message to the application
        nativeQuit();

        // Now wait for the Orx thread to quit
        if (mOrxThread != null) {
            try {
                mOrxThread.join();
            } catch(Exception e) {
                Log.v("Orx", "Problem stopping thread: " + e);
            }
            mOrxThread = null;

            Log.v("Orx", "Finished waiting for Orx thread");
        }
    }

    // C functions we call
    native void nativeInit();
    native void nativeQuit();
    native void nativePause();
    native void nativeResume();
    native void nativeSurfaceDestroyed();
    native void nativeSurfaceCreated(Surface surface);
    native void nativeSurfaceChanged();
    native void onNativeResize(int x, int y);
    native void onNativeKeyDown(int keycode);
    native void onNativeKeyUp(int keycode);
    native void onNativeTouch(int touchDevId, int pointerFingerId,
                                            int action, float x, 
                                            float y, float p);

    // Java functions called from C
    
    public int getRotation() {
    	WindowManager windowMgr = (WindowManager) getSystemService(WINDOW_SERVICE);
    	int rotationIndex = windowMgr.getDefaultDisplay().getRotation();
    	return rotationIndex;
    }

    void startApp() {
        // Start up the C app thread
        if (mOrxThread == null) {
            mOrxThread = new Thread(new OrxMain(), "OrxThread");
            mOrxThread.start();
        }
        else {
            /*
             * Some Android variants may send multiple surfaceChanged events, so we don't need to resume every time
             * every time we get one of those events, only if it comes after surfaceDestroyed
             */
            if (mIsPaused) {
            	nativeSurfaceChanged();
                mIsPaused = false;
            }
        }
    }
    
    private void finishApp() {
    	finish();
    }
    
	/**
	 * Simple nativeInit() runnable
	 */
	class OrxMain implements Runnable {
		public void run() {
			// Runs Orx_main()
			nativeInit();

			Log.v("Orx", "Orx thread terminated");

			finishApp();
		}
	}
}

