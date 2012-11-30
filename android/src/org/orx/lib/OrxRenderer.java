package org.orx.lib;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class OrxRenderer implements GLSurfaceView.Renderer {
	// ===========================================================
	// Fields
	// ===========================================================

	private int mScreenWidth;
	private int mScreenHeight;

	// ===========================================================
	// Constructors
	// ===========================================================

	// ===========================================================
	// Getter & Setter
	// ===========================================================

	public void setScreenWidthAndHeight(final int pSurfaceWidth, final int pSurfaceHeight) {
		mScreenWidth = pSurfaceWidth;
		mScreenHeight = pSurfaceHeight;
	}

	// ===========================================================
	// Methods for/from SuperClass/Interfaces
	// ===========================================================

	@Override
	public void onSurfaceCreated(final GL10 pGL10, final EGLConfig pEGLConfig) {
		OrxRenderer.nativeInit(mScreenWidth, mScreenHeight);
	}

	@Override
	public void onSurfaceChanged(final GL10 pGL10, final int pWidth, final int pHeight) {
	}

	@Override
	public void onDrawFrame(final GL10 gl) {
		// should render a frame when onDrawFrame() is called or there is a
		// "ghost"
		OrxRenderer.nativeRender();
	}


	@Override
	public void onExit() {
		OrxRenderer.nativeExit();
	}

	// ===========================================================
	// Methods
	// ===========================================================

	private static native void nativeTouchesBegin(final int pID, final float pX, final float pY);
	private static native void nativeTouchesEnd(final int pID, final float pX, final float pY);
	private static native void nativeTouchesMove(final int[] pIDs, final float[] pXs, final float[] pYs);
	private static native void nativeTouchesCancel(final int[] pIDs, final float[] pXs, final float[] pYs);
	private static native boolean nativeKeyDown(final int pKeyCode);
	private static native boolean nativeKeyUp(final int pKeyCode);
	private static native void nativeRender();
	private static native void nativeInit(final int pWidth, final int pHeight);
	private static native void nativeExit();
	private static native void nativeOnPause();
	private static native void nativeOnResume();

	public void handleActionDown(final int pID, final float pX, final float pY) {
		OrxRenderer.nativeTouchesBegin(pID, pX, pY);
	}

	public void handleActionUp(final int pID, final float pX, final float pY) {
		OrxRenderer.nativeTouchesEnd(pID, pX, pY);
	}

	public void handleActionCancel(final int[] pIDs, final float[] pXs, final float[] pYs) {
		OrxRenderer.nativeTouchesCancel(pIDs, pXs, pYs);
	}

	public void handleActionMove(final int[] pIDs, final float[] pXs, final float[] pYs) {
		OrxRenderer.nativeTouchesMove(pIDs, pXs, pYs);
	}

	public void handleKeyDown(final int pKeyCode) {
		OrxRenderer.nativeKeyDown(pKeyCode);
	}

	public void handleKeyUp(final int pKeyCode) {
		OrxRenderer.nativeKeyUp(pKeyCode);
	}

	public void handleOnPause() {
		OrxRenderer.nativeOnPause();
	}

	public void handleOnResume() {
		OrxRenderer.nativeOnResume();
	}
}
