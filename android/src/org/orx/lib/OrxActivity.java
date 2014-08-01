package org.orx.lib;

import android.annotation.SuppressLint;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;

/**
    Orx Activity
*/
public class OrxActivity extends FragmentActivity implements SurfaceHolder.Callback,
    View.OnKeyListener, View.OnTouchListener {

    private SurfaceView mSurface;
    private OrxThreadFragment mOrxThreadFragment;
    
    @Override
    protected void onCreate(Bundle arg0) {
    	super.onCreate(arg0);
    	
        FragmentManager fm = getSupportFragmentManager();
        mOrxThreadFragment = (OrxThreadFragment) fm.findFragmentByTag(OrxThreadFragment.TAG);
        if (mOrxThreadFragment == null) {
            mOrxThreadFragment = new OrxThreadFragment();
            fm.beginTransaction().add(mOrxThreadFragment, OrxThreadFragment.TAG).commit();
        }    	
    }
    
    @Override
    protected void onStart() {
    	super.onStart();
    	
    	if(mSurface == null) {
    		ViewGroup root = (ViewGroup) findViewById(android.R.id.content);
    		
    		if(root.getChildCount() == 0) {
    			mSurface = new SurfaceView(getApplication());
        		setContentView(mSurface);
    		} else {
            	int surfaceId = getResources().getIdentifier("id/orxSurfaceView", null, getPackageName());
            	
            	if(surfaceId != 0) {
            		mSurface = (SurfaceView) root.findViewById(surfaceId);
            		
            		if(mSurface == null) {
            			throw new RuntimeException("SurfaceView with identifier orxSurfaceView not found in layout.");
            		}
            	} else {
            		throw new RuntimeException("No identifier orxSurfaceView found. Define a SurfaceView with android:id=\"@+id/orxSurfaceView\" in your layout.");
            	}
    		}
    		
        	mSurface.getHolder().addCallback(this);
        	mSurface.setFocusable(true);
        	mSurface.setFocusableInTouchMode(true);
        	mSurface.requestFocus();
        	mSurface.setOnKeyListener(this);
        	mSurface.setOnTouchListener(this);    		
    	}
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
    native void nativeOnKeyDown(int keycode, int unicode);
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

