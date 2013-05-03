package org.orx.lib;

import android.app.Activity;
import android.graphics.Canvas;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.WindowManager;

/**
    Orx Activity
*/
public class OrxActivity extends Activity implements SurfaceHolder.Callback,
View.OnKeyListener, View.OnTouchListener {

    private boolean mDestroyed = false;
    private boolean mFinished = false;
    private SurfaceHolder mCurSurfaceHolder;
    
    // Main components
    private OrxGLSurfaceView mSurface;

    // This is what Orx runs in. It invokes Orx_main(), eventually
    private Thread mOrxThread;

    // Setup
    protected void onCreate(Bundle savedInstanceState) {
        Log.v("Orx", "onCreate()");
        
        nativeCreate();
        init();
		startApp();
		
        super.onCreate(savedInstanceState);
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
    	
    	mSurface.getHolder().addCallback(this);
    	mSurface.setFocusable(true);
    	mSurface.setFocusableInTouchMode(true);
    	mSurface.requestFocus();
    	mSurface.setOnKeyListener(this);
    	mSurface.setOnTouchListener(this);
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
        
        if(!mFinished)
        	nativePause();
    }

    protected void onResume() {
        Log.v("Orx", "onResume()");
        super.onResume();
        nativeResume();
    }

    protected void onDestroy() {
        Log.v("Orx", "onDestroy()");
        
        if (mCurSurfaceHolder != null && !mFinished) {
        	nativeSurfaceDestroyed();
        }
        mCurSurfaceHolder = null;
        
        // Send a quit message to the application
        if(!mFinished)
        	nativeQuit();

        mDestroyed = true;
        
        super.onDestroy();
    }

	// Called when we have a valid drawing surface
	public void surfaceCreated(SurfaceHolder holder) {
		Log.v("Orx", "surfaceCreated()");
		
		if(!mDestroyed) {
			mCurSurfaceHolder = holder;
			nativeSurfaceCreated(holder.getSurface());
		}
	}

	// Called when we lose the surface
	public void surfaceDestroyed(SurfaceHolder holder) {
		Log.v("Orx", "surfaceDestroyed()");
		
		if (!mDestroyed && !mFinished) {
			nativeSurfaceDestroyed();
		}
		mCurSurfaceHolder = null;
	}

	// Called when the surface is resized
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		Log.v("Orx", "surfaceChanged()");

		if(!mDestroyed) {
			mCurSurfaceHolder = holder;
			Log.v("Orx", "Window size:" + width + "x" + height);
			nativeSurfaceChanged();
		}
	}

	// unused
	public void onDraw(Canvas canvas) {
	}

	// Key events
	public boolean onKey(View v, int keyCode, KeyEvent event) {

		if(!mFinished) {
			switch (keyCode) {
			case KeyEvent.KEYCODE_BACK:
			case KeyEvent.KEYCODE_MENU:
				if (event.getAction() == KeyEvent.ACTION_DOWN) {
					Log.v("Orx", "key down: " + keyCode);
					onNativeKeyDown(keyCode);
					return true;
				} else if (event.getAction() == KeyEvent.ACTION_UP) {
					Log.v("Orx", "key up: " + keyCode);
					onNativeKeyUp(keyCode);
					return true;
				}
			}
		}

		return false;
	}

	// Touch events
	public boolean onTouch(View v, MotionEvent event) {
		if(!mFinished) {
			final int touchDevId = event.getDeviceId();
			final int pointerCount = event.getPointerCount();
			// touchId, pointerId, action, x, y, pressure
			int actionPointerIndex = event.getActionIndex();
			int pointerFingerId = event.getPointerId(actionPointerIndex);
			int action = event.getActionMasked();

			float x = event.getX(actionPointerIndex);
			float y = event.getY(actionPointerIndex);
			float p = event.getPressure(actionPointerIndex);

			if (action == MotionEvent.ACTION_MOVE && pointerCount > 1) {
				// TODO send motion to every pointer if its position has
				// changed since prev event.
				for (int i = 0; i < pointerCount; i++) {
					pointerFingerId = event.getPointerId(i);
					x = event.getX(i);
					y = event.getY(i);
					p = event.getPressure(i);
					onNativeTouch(touchDevId, pointerFingerId, action,
							x, y, p);
				}
			} else {
				onNativeTouch(touchDevId, pointerFingerId, action, x,
						y, p);
			}
		}
		return true;
	}

    // C functions we call
	native void nativeCreate();
    native void nativeInit();
    native void nativeQuit();
    native void nativePause();
    native void nativeResume();
    native void nativeSurfaceDestroyed();
    native void nativeSurfaceCreated(Surface surface);
    native void nativeSurfaceChanged();
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
    	mOrxThread = new Thread(new OrxMain(), "OrxThread");
        mOrxThread.start();
    }
    
    private void finishApp() {
    	mFinished = true;
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

