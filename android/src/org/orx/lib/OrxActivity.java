package org.orx.lib;

import android.os.Bundle;
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
import android.view.inputmethod.InputMethodManager;

/**
    Orx Activity
*/
public class OrxActivity extends FragmentActivity implements SurfaceHolder.Callback,
    View.OnKeyListener, View.OnTouchListener {

    private SurfaceView mSurface;

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
        nativeOnSurfaceCreated(holder.getSurface());
	}

	// Called when we lose the surface
	public void surfaceDestroyed(SurfaceHolder holder) {
		nativeOnSurfaceDestroyed();
	}

	// Called when the surface is resized
	public void surfaceChanged(SurfaceHolder holder, int format, int width,	int height) {
		nativeOnSurfaceChanged(width, height);
	}

	// Key events
	public boolean onKey(View v, int keyCode, KeyEvent event) {
		
		Log.e("OrxActivity", "keyCode = " + keyCode + ", event = " + event.getAction());

		/* dont send VOL+ and VOL- */
        if(keyCode != KeyEvent.KEYCODE_VOLUME_UP && keyCode != KeyEvent.KEYCODE_VOLUME_DOWN) {
            switch(event.getAction()) {
                case KeyEvent.ACTION_DOWN:
                    nativeOnKeyDown(keyCode);
                    break;
                case KeyEvent.ACTION_UP:
                    nativeOnKeyUp(keyCode);
                    break;
            }

            return true;
        }

		return false;
	}

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        nativeOnFocusChanged(hasFocus);
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
                nativeOnTouch(touchDevId, pointerFingerId, action, x, y, p);
            }
        } else {
            nativeOnTouch(touchDevId, pointerFingerId, action, x, y, p);
        }
        return true;
	}

    // C functions we call
    native void nativeOnSurfaceCreated(Surface surface);
    native void nativeOnSurfaceDestroyed();
    native void nativeOnSurfaceChanged(int width, int height);
    native void nativeOnKeyDown(int keycode);
    native void nativeOnKeyUp(int keycode);
    native void nativeOnTouch(int touchDevId, int pointerFingerId,
                                            int action, float x, 
                                            float y, float p);
    native void nativeOnFocusChanged(boolean hasFocus);

    // Java functions called from C
    
    public int getRotation() {
    	WindowManager windowMgr = (WindowManager) getSystemService(WINDOW_SERVICE);
    	int rotationIndex = windowMgr.getDefaultDisplay().getRotation();
    	return rotationIndex;
    }
    
    public void setWindowFormat(final int format) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                getWindow().setFormat(format);
            }
        });
    }
    
    public void showKeyboard(final boolean show) {
    	runOnUiThread(new Runnable() {
			
			@Override
			public void run() {
		        InputMethodManager imm = (InputMethodManager) getSystemService(INPUT_METHOD_SERVICE);

		        if (show) {
		        	imm.showSoftInput(mSurface, InputMethodManager.SHOW_IMPLICIT);
		        } else {
		        	imm.hideSoftInputFromWindow(mSurface.getWindowToken(), InputMethodManager.HIDE_IMPLICIT_ONLY);
		        }
			}
		});
    }
}

