package org.orx.lib;

import android.content.Context;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.MotionEvent;

public class OrxGLSurfaceView extends GLSurfaceView {
	// ===========================================================
	// Fields
	// ===========================================================

	private OrxRenderer mOrxRenderer;

	// ===========================================================
	// Constructors
	// ===========================================================

	public OrxGLSurfaceView(final Context context) {
		super(context);
	}

	public OrxGLSurfaceView(final Context context, final AttributeSet attrs) {
		super(context, attrs);
	}

	public void initView(boolean requireDepthBuffer) {
		setFocusableInTouchMode(true);
		setEGLContextClientVersion(2);
		// force RGB565 surface
		setEGLConfigChooser(5, 6, 5, 0, requireDepthBuffer ? 16 : 0, 0);
		setPreserveEGLContextOnPause(true);
	}
	
	// ===========================================================
	// Getter & Setter
	// ===========================================================

	public void setOrxRenderer(final OrxRenderer renderer) {
		mOrxRenderer = renderer;
		setRenderer(mOrxRenderer);
	}

	// ===========================================================
	// Methods for/from SuperClass/Interfaces
	// ===========================================================

	@Override
	public void onResume() {
		super.onResume();

		queueEvent(new Runnable() {
			@Override
			public void run() {
				OrxGLSurfaceView.this.mOrxRenderer.handleOnResume();
			}
		});
	}

	@Override
	public void onPause() {
		queueEvent(new Runnable() {
			@Override
			public void run() {
				OrxGLSurfaceView.this.mOrxRenderer.handleOnPause();
			}
		});

		super.onPause();
	}

	@Override
	public boolean onTouchEvent(final MotionEvent pMotionEvent) {
		// these data are used in ACTION_MOVE and ACTION_CANCEL
		final int pointerNumber = pMotionEvent.getPointerCount();
		final int[] ids = new int[pointerNumber];
		final float[] xs = new float[pointerNumber];
		final float[] ys = new float[pointerNumber];

		for (int i = 0; i < pointerNumber; i++) {
			ids[i] = pMotionEvent.getPointerId(i);
			xs[i] = pMotionEvent.getX(i);
			ys[i] = pMotionEvent.getY(i);
		}

		switch (pMotionEvent.getAction() & MotionEvent.ACTION_MASK) {
		case MotionEvent.ACTION_POINTER_DOWN:
			final int indexPointerDown = pMotionEvent.getAction() >> MotionEvent.ACTION_POINTER_ID_SHIFT;
			final int idPointerDown = pMotionEvent
					.getPointerId(indexPointerDown);
			final float xPointerDown = pMotionEvent.getX(indexPointerDown);
			final float yPointerDown = pMotionEvent.getY(indexPointerDown);

			queueEvent(new Runnable() {
				@Override
				public void run() {
					OrxGLSurfaceView.this.mOrxRenderer.handleActionDown(
							idPointerDown, xPointerDown, yPointerDown);
				}
			});
			break;

		case MotionEvent.ACTION_DOWN:
			// there are only one finger on the screen
			final int idDown = pMotionEvent.getPointerId(0);
			final float xDown = xs[0];
			final float yDown = ys[0];

			queueEvent(new Runnable() {
				@Override
				public void run() {
					OrxGLSurfaceView.this.mOrxRenderer.handleActionDown(idDown,
							xDown, yDown);
				}
			});
			break;

		case MotionEvent.ACTION_MOVE:
			queueEvent(new Runnable() {
				@Override
				public void run() {
					OrxGLSurfaceView.this.mOrxRenderer.handleActionMove(ids,
							xs, ys);
				}
			});
			break;

		case MotionEvent.ACTION_POINTER_UP:
			final int indexPointUp = pMotionEvent.getAction() >> MotionEvent.ACTION_POINTER_ID_SHIFT;
			final int idPointerUp = pMotionEvent.getPointerId(indexPointUp);
			final float xPointerUp = pMotionEvent.getX(indexPointUp);
			final float yPointerUp = pMotionEvent.getY(indexPointUp);

			queueEvent(new Runnable() {
				@Override
				public void run() {
					OrxGLSurfaceView.this.mOrxRenderer.handleActionUp(
							idPointerUp, xPointerUp, yPointerUp);
				}
			});
			break;

		case MotionEvent.ACTION_UP:
			// there are only one finger on the screen
			final int idUp = pMotionEvent.getPointerId(0);
			final float xUp = xs[0];
			final float yUp = ys[0];

			queueEvent(new Runnable() {
				@Override
				public void run() {
					OrxGLSurfaceView.this.mOrxRenderer.handleActionUp(idUp,
							xUp, yUp);
				}
			});
			break;

		case MotionEvent.ACTION_CANCEL:
			queueEvent(new Runnable() {
				@Override
				public void run() {
					OrxGLSurfaceView.this.mOrxRenderer.handleActionCancel(ids,
							xs, ys);
				}
			});
			break;
		}

		return true;
	}

	/*
	 * This function is called before OrxRenderer.nativeInit(), so the width and
	 * height is correct.
	 */
	@Override
	protected void onSizeChanged(final int pNewSurfaceWidth,
			final int pNewSurfaceHeight, final int pOldSurfaceWidth,
			final int pOldSurfaceHeight) {
		if (!this.isInEditMode()) {
			mOrxRenderer.setScreenWidthAndHeight(pNewSurfaceWidth,
					pNewSurfaceHeight);
		}
	}

	@Override
	public boolean onKeyDown(final int pKeyCode, final KeyEvent pKeyEvent) {
		switch (pKeyCode) {
		case KeyEvent.KEYCODE_BACK:
		case KeyEvent.KEYCODE_MENU:
			queueEvent(new Runnable() {
				@Override
				public void run() {
					OrxGLSurfaceView.this.mOrxRenderer.handleKeyDown(pKeyCode);
				}
			});
			return true;
		default:
			return super.onKeyDown(pKeyCode, pKeyEvent);
		}
	}

	@Override
	public boolean onKeyUp(final int pKeyCode, KeyEvent pKeyEvent) {
		switch (pKeyCode) {
		case KeyEvent.KEYCODE_BACK:
		case KeyEvent.KEYCODE_MENU:
			queueEvent(new Runnable() {
				@Override
				public void run() {
					OrxGLSurfaceView.this.mOrxRenderer.handleKeyUp(pKeyCode);
				}
			});
			return true;
		default:
			return super.onKeyUp(pKeyCode, pKeyEvent);
		}
	}
}
