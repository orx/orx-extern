package org.orx.lib;

import android.graphics.Canvas;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.app.FragmentActivity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.WindowManager;

/**
    Orx Activity
*/
public class OrxActivity extends FragmentActivity implements SurfaceHolder.Callback,
View.OnKeyListener, View.OnTouchListener {

    private boolean mDestroyed = false;
    private boolean mFinished = false;
    private SurfaceHolder mCurSurfaceHolder;
    
    private Handler mHandler = new Handler();
    
    // Main components
    private OrxGLSurfaceView mSurface;

    // This is what Orx runs in. It invokes Orx_main(), eventually
    private Thread mOrxThread;

    // Setup
    protected void onCreate(Bundle savedInstanceState) {
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
        super.onPause();
        
        if(!mFinished)
        	nativePause();
    }

    protected void onResume() {
        super.onResume();
        
        nativeResume();
    }

    protected void onDestroy() {
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
	}

	// Called when we lose the surface
	public void surfaceDestroyed(SurfaceHolder holder) {
		if (!mDestroyed && !mFinished) {
			nativeSurfaceDestroyed();
		}
		mCurSurfaceHolder = null;
	}

	// Called when the surface is resized
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		if(!mDestroyed && mCurSurfaceHolder != holder) {
			mCurSurfaceHolder = holder;
			nativeSurfaceChanged(holder.getSurface());
		}
	}

	// unused
	public void onDraw(Canvas canvas) {
	}

	// Key events
	public boolean onKey(View v, int keyCode, KeyEvent event) {

		if(!mFinished) {
			/* dont send VOL+ and VOL- */
			if(keyCode != KeyEvent.KEYCODE_VOLUME_UP && keyCode != KeyEvent.KEYCODE_VOLUME_DOWN) {
				switch(event.getAction()) {
				case KeyEvent.ACTION_DOWN:
					onNativeKeyDown(keyCode);
					break;
				case KeyEvent.ACTION_UP:
					onNativeKeyUp(keyCode);
					break;
				}
				
				return true;
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
    native void nativeSurfaceChanged(Surface surface);
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
    
    public void setWindowFormat(int format) {
    	final int f = format;
    	mHandler.post(new Runnable() {

			@Override
			public void run() {
				getWindow().setFormat(f);
			}
    		
    	});
    	
    }

    private void startApp() {
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
			finishApp();
		}
	}
}

