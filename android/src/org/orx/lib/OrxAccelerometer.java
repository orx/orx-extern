package org.orx.lib;

import android.content.Context;
import android.content.res.Configuration;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.view.Display;
import android.view.Surface;
import android.view.WindowManager;

public class OrxAccelerometer implements SensorEventListener {

	// ===========================================================
	// Fields
	// ===========================================================

	private final Context mContext;
	private final OrxGLSurfaceView mGLSurfaceView;
	private final SensorManager mSensorManager;
	private final Sensor mAccelerometer;
	private final int mNaturalOrientation;
	
	// ===========================================================
	// Constructors
	// ===========================================================

	public OrxAccelerometer(final Context pContext, final OrxGLSurfaceView pGLSurfaceView) {
		mContext = pContext;
		mGLSurfaceView = pGLSurfaceView;
		mSensorManager = (SensorManager) mContext
				.getSystemService(Context.SENSOR_SERVICE);
		mAccelerometer = mSensorManager
				.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

		final Display display = ((WindowManager) mContext
				.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
		mNaturalOrientation = display.getOrientation();
	}

	public void enable(int rate) {
		mSensorManager.registerListener(this, mAccelerometer, rate);
	}


	public void disable() {
		this.mSensorManager.unregisterListener(this);
	}

	@Override
	public void onAccuracyChanged(Sensor arg0, int arg1) {
	}

	@Override
	public void onSensorChanged(final SensorEvent pSensorEvent) {
		if (pSensorEvent.sensor.getType() != Sensor.TYPE_ACCELEROMETER) {
			return;
		}

		float tmpX = pSensorEvent.values[0];
		float tmpY = pSensorEvent.values[1];
		final float z = pSensorEvent.values[2];

		/*
		 * Because the axes are not swapped when the device's screen orientation
		 * changes. So we should swap it here. In tablets such as Motorola Xoom,
		 * the default orientation is landscape, so should consider this.
		 */
		final int orientation = mContext.getResources().getConfiguration().orientation;

		if ((orientation == Configuration.ORIENTATION_LANDSCAPE) && (this.mNaturalOrientation != Surface.ROTATION_0)) {
			final float tmp = tmpX;
			tmpX = -tmpY;
			tmpY = tmp;
		} else if ((orientation == Configuration.ORIENTATION_PORTRAIT) && (this.mNaturalOrientation != Surface.ROTATION_0)) {
			final float tmp = tmpX;
			tmpX = tmpY;
			tmpY = -tmp;
		}
		
		final float x = tmpX;
		final float y = tmpY;
		
		mGLSurfaceView.queueEvent(new Runnable() {
			
			@Override
			public void run() {
				OrxAccelerometer.onSensorChanged(-x, y, z, pSensorEvent.timestamp);
			}
		});
	}
	
	private static native void onSensorChanged(final float pX, final float pY, final float pZ, final long pTimestamp);

}
