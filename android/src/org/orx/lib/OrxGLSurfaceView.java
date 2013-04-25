package org.orx.lib;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

/**
 * OrxSurface. This is what we draw on, so we need to know when it's created in
 * order to do anything useful.
 * 
 * Because of this, that's where we set up the Orx thread
 */
public class OrxGLSurfaceView extends SurfaceView implements SurfaceHolder.Callback,
		View.OnKeyListener, View.OnTouchListener {

	private OrxActivity mOrxActivity;

	// Startup
	public OrxGLSurfaceView(Context context) {
		super(context);
		init();
	}
	
	public OrxGLSurfaceView(final Context context, final AttributeSet attrs) {
		super(context, attrs);
		init();
	}

	private void init() {
		getHolder().addCallback(this);
		setFocusable(true);
		setFocusableInTouchMode(true);
		requestFocus();
		setOnKeyListener(this);
		setOnTouchListener(this);
	}
	
	public void setActivity(OrxActivity activity) {
		mOrxActivity = activity;
	}

	// Called when we have a valid drawing surface
	public void surfaceCreated(SurfaceHolder holder) {
		Log.v("Orx", "surfaceCreated()");
	}

	// Called when we lose the surface
	public void surfaceDestroyed(SurfaceHolder holder) {
		Log.v("Orx", "surfaceDestroyed()");
		if (!mOrxActivity.mIsPaused) {
			mOrxActivity.mIsPaused = true;
			mOrxActivity.nativeSurfaceDestroyed();
		}
	}

	// Called when the surface is resized
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		Log.v("Orx", "surfaceChanged()");

		mOrxActivity.onNativeResize(width, height);
		Log.v("Orx", "Window size:" + width + "x" + height);

		mOrxActivity.startApp();
	}

	// unused
	public void onDraw(Canvas canvas) {
	}

	// Key events
	public boolean onKey(View v, int keyCode, KeyEvent event) {

		switch (keyCode) {
		case KeyEvent.KEYCODE_BACK:
		case KeyEvent.KEYCODE_MENU:
			if (event.getAction() == KeyEvent.ACTION_DOWN) {
				Log.v("Orx", "key down: " + keyCode);
				mOrxActivity.onNativeKeyDown(keyCode);
				return true;
			} else if (event.getAction() == KeyEvent.ACTION_UP) {
				Log.v("Orx", "key up: " + keyCode);
				mOrxActivity.onNativeKeyUp(keyCode);
				return true;
			}
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
				mOrxActivity.onNativeTouch(touchDevId, pointerFingerId, action,
						x, y, p);
			}
		} else {
			mOrxActivity.onNativeTouch(touchDevId, pointerFingerId, action, x,
					y, p);
		}
		return true;
	}
}