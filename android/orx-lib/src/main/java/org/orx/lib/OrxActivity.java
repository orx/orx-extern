package org.orx.lib;

import android.annotation.SuppressLint;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.util.Log;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;

import org.orx.lib.inputmanagercompat.InputManagerCompat;

/**
    Orx Activity
*/
public class OrxActivity extends FragmentActivity implements SurfaceHolder.Callback,
    View.OnKeyListener, View.OnTouchListener, InputManagerCompat.InputDeviceListener {

    private SurfaceView mSurface;
    private OrxThreadFragment mOrxThreadFragment;
    private InputManagerCompat mInputManager;

    @Override
    protected void onCreate(Bundle arg0) {
    	super.onCreate(arg0);
    	
        FragmentManager fm = getSupportFragmentManager();
        mOrxThreadFragment = (OrxThreadFragment) fm.findFragmentByTag(OrxThreadFragment.TAG);
        if (mOrxThreadFragment == null) {
            mOrxThreadFragment = new OrxThreadFragment();
            fm.beginTransaction().add(mOrxThreadFragment, OrxThreadFragment.TAG).commit();
        }

        mInputManager = InputManagerCompat.Factory.getInputManager(this);
        mInputManager.registerInputDeviceListener(this, null);
    }
    
    @Override
    protected void onStart() {
    	super.onStart();
    	
    	if(mSurface == null) {
            int surfaceId = getResources().getIdentifier("id/orxSurfaceView", null, getPackageName());

            if(surfaceId != 0) {
                mSurface = (SurfaceView) findViewById(surfaceId);

                if(mSurface == null) {
                    Log.d("OrxActivity", "SurfaceView with identifier orxSurfaceView not found in layout.");
                    mSurface = new SurfaceView(getApplication());
                    setContentView(mSurface);
                }
            } else {
                Log.d("OrxActivity", "No identifier orxSurfaceView found.");
                mSurface = new SurfaceView(getApplication());
                setContentView(mSurface);
            }
    		
        	mSurface.getHolder().addCallback(this);
        	mSurface.setFocusable(true);
        	mSurface.setFocusableInTouchMode(true);
        	mSurface.requestFocus();
        	mSurface.setOnKeyListener(this);
        	mSurface.setOnTouchListener(this);

            if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB_MR1) {
                mSurface.setOnGenericMotionListener(new OrxOnGenericMotionListener(this, mInputManager));
            }
    	}
    }

    @Override
    protected void onPause() {
        super.onPause();
        mInputManager.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mInputManager.onResume();
    }

    // Called when we have a valid drawing surface
	@SuppressLint("NewApi")
	public void surfaceCreated(SurfaceHolder holder) {
        Surface s = holder.getSurface();
		nativeOnSurfaceCreated(s);
		
		if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
			s.release();
		}
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
		switch (event.getAction()) {
		case KeyEvent.ACTION_DOWN:
			nativeOnKeyDown(keyCode, event.getUnicodeChar() & KeyCharacterMap.COMBINING_ACCENT_MASK);
			break;
		case KeyEvent.ACTION_UP:
			nativeOnKeyUp(keyCode);
			break;
			
		case KeyEvent.ACTION_MULTIPLE:
			if(keyCode == KeyEvent.KEYCODE_UNKNOWN) {
				final KeyCharacterMap m = KeyCharacterMap.load(event.getDeviceId());
                final KeyEvent[] es = m.getEvents(event.getCharacters().toCharArray());
                
                if (es != null) {
                	for (KeyEvent s : es) {
                		switch(s.getAction()) {
                		case KeyEvent.ACTION_DOWN:
                    		nativeOnKeyDown(s.getKeyCode(), event.getUnicodeChar() & KeyCharacterMap.COMBINING_ACCENT_MASK);
                			break;
                		case KeyEvent.ACTION_UP:
                			nativeOnKeyUp(s.getKeyCode());
                			break;
                		}
                	}
                }
                
                return true;
			}
		}

		if (keyCode != KeyEvent.KEYCODE_VOLUME_UP
				&& keyCode != KeyEvent.KEYCODE_VOLUME_DOWN)
			return true;

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

        int x = (int) event.getX(actionPointerIndex);
        int y = (int) event.getY(actionPointerIndex);
        int p = (int) event.getPressure(actionPointerIndex);

        if (action == MotionEvent.ACTION_MOVE && pointerCount > 1) {
            // TODO send motion to every pointer if its position has
            // changed since prev event.
            for (int i = 0; i < pointerCount; i++) {
                pointerFingerId = event.getPointerId(i);
                x = (int) event.getX(i);
                y = (int) event.getY(i);
                p = (int) event.getPressure(i);
                nativeOnTouch(touchDevId, pointerFingerId, action, x, y, p);
            }
        } else {
            nativeOnTouch(touchDevId, pointerFingerId, action, x, y, p);
        }
        return true;
	}

    @Override
    public void onInputDeviceAdded(int deviceId) {
        nativeOnInputDeviceAdded(deviceId);
    }

    @Override
    public void onInputDeviceChanged(int deviceId) {
        nativeOnInputDeviceChanged(deviceId);
    }

    @Override
    public void onInputDeviceRemoved(int deviceId) {
        nativeOnInputDeviceRemoved(deviceId);
    }

    // C functions we call
    native void nativeOnSurfaceCreated(Surface surface);
    native void nativeOnSurfaceDestroyed();
    native void nativeOnSurfaceChanged(int width, int height);
    native void nativeOnKeyDown(int keycode, int unicode);
    native void nativeOnKeyUp(int keycode);
    native void nativeOnTouch(int touchDevId, int pointerFingerId,
                                            int action, int x, 
                                            int y, int p);
    native void nativeOnFocusChanged(boolean hasFocus);

    native void nativeOnInputDeviceAdded(int deviceId);
    native void nativeOnInputDeviceChanged(int deviceId);
    native void nativeOnInputDeviceRemoved(int deviceId);

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

