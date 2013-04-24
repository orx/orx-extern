package org.orx.lib;

import android.content.Context;
import android.graphics.Canvas;
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
public class OrxSurface extends SurfaceView implements SurfaceHolder.Callback,
		View.OnKeyListener, View.OnTouchListener {

	// Keep track of the surface size to normalize touch events
	private static float mWidth, mHeight;

	// Startup
	public OrxSurface(Context context) {
		super(context);
		getHolder().addCallback(this);

		setFocusable(true);
		setFocusableInTouchMode(true);
		requestFocus();
		setOnKeyListener(this);
		setOnTouchListener(this);

		// Some arbitrary defaults to avoid a potential division by zero
		mWidth = 1.0f;
		mHeight = 1.0f;
	}

	// Called when we have a valid drawing surface
	public void surfaceCreated(SurfaceHolder holder) {
		Log.v("Orx", "surfaceCreated()");
	}

	// Called when we lose the surface
	public void surfaceDestroyed(SurfaceHolder holder) {
		Log.v("Orx", "surfaceDestroyed()");
		if (!OrxActivity.mIsPaused) {
			OrxActivity.mIsPaused = true;
			OrxActivity.nativeSurfaceDestroyed();
		}
	}

	// Called when the surface is resized
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		Log.v("Orx", "surfaceChanged()");

		mWidth = (float) width;
		mHeight = (float) height;
		OrxActivity.onNativeResize(width, height);
		Log.v("Orx", "Window size:" + width + "x" + height);

		OrxActivity.startApp();
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
				OrxActivity.onNativeKeyDown(keyCode);
				return true;
			} else if (event.getAction() == KeyEvent.ACTION_UP) {
				Log.v("Orx", "key up: " + keyCode);
				OrxActivity.onNativeKeyUp(keyCode);
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
				OrxActivity.onNativeTouch(touchDevId, pointerFingerId, action,
						x, y, p);
			}
		} else {
			OrxActivity.onNativeTouch(touchDevId, pointerFingerId, action, x,
					y, p);
		}
		return true;
	}
}