package org.orx.lib;

import android.os.Bundle;
import android.os.Handler;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;

/**
    Orx Activity
*/
public class OrxActivity extends FragmentActivity implements SurfaceHolder.Callback,
    View.OnKeyListener, View.OnTouchListener, View.OnFocusChangeListener {

    private SurfaceView mSurface;
    private SurfaceHolder mCurSurfaceHolder;

    private Handler mHandler = new Handler();

    private OrxThreadFragment mOrxThreadFragment;

    // Setup
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        init();

        FragmentManager fm = getSupportFragmentManager();
        mOrxThreadFragment = (OrxThreadFragment) fm.findFragmentByTag(OrxThreadFragment.TAG);
        if (mOrxThreadFragment == null) {
            mOrxThreadFragment = new OrxThreadFragment();
            fm.beginTransaction().add(mOrxThreadFragment, OrxThreadFragment.TAG).commit();
        }
    }
    
    private void init() {
    	if(getLayoutId() == 0 || getSurfaceViewId() == 0) {
            // Set up the surface
            mSurface = new SurfaceView(getApplication());
            setContentView(mSurface);
    	} else {
			setContentView(getLayoutId());
			mSurface = (SurfaceView) findViewById(getSurfaceViewId());
		}
    	
    	mSurface.getHolder().addCallback(this);
    	mSurface.setFocusable(true);
    	mSurface.setFocusableInTouchMode(true);
    	mSurface.requestFocus();
    	mSurface.setOnKeyListener(this);
    	mSurface.setOnTouchListener(this);
        mSurface.setOnFocusChangeListener(this);
    }
    
    protected int getLayoutId() {
		/*
		 * Override this if you want to use a custom layout
		 * return the layout id
		 */
		return 0;
	}
    
    protected int getSurfaceViewId() {
		/*
		 * Override this if you want to use a custom layout
		 * return the OrxGLSurfaceView id
		 */
		return 0;
	}

	// Called when we have a valid drawing surface
	public void surfaceCreated(SurfaceHolder holder) {
	}

	// Called when we lose the surface
	public void surfaceDestroyed(SurfaceHolder holder) {
		nativeSurfaceDestroyed();
		mCurSurfaceHolder = null;
	}

	// Called when the surface is resized
	public void surfaceChanged(SurfaceHolder holder, int format, int width,	int height) {
		if(mCurSurfaceHolder != holder) {
			mCurSurfaceHolder = holder;
			nativeSurfaceChanged(holder.getSurface());
		}
	}

	// Key events
	public boolean onKey(View v, int keyCode, KeyEvent event) {

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

		return false;
	}

	// Touch events
	public boolean onTouch(View v, MotionEvent event) {
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
                onNativeTouch(touchDevId, pointerFingerId, action, x, y, p);
            }
        } else {
            onNativeTouch(touchDevId, pointerFingerId, action, x, y, p);
        }
        return true;
	}

    @Override
    public void onFocusChange(View v, boolean hasFocus) {
        if(hasFocus) {
            nativeFocusGained();
        } else {
            nativeFocusLost();
        }
    }

    // C functions we call
    native void nativeSurfaceDestroyed();
    native void nativeSurfaceChanged(Surface surface);
    native void onNativeKeyDown(int keycode);
    native void onNativeKeyUp(int keycode);
    native void onNativeTouch(int touchDevId, int pointerFingerId,
                                            int action, float x, 
                                            float y, float p);
    native void nativeFocusGained();
    native void nativeFocusLost();

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
}

