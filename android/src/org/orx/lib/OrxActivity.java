package org.orx.lib;

import android.app.Activity;
import android.graphics.PixelFormat;
import android.os.Bundle;
import android.util.Log;
import android.view.ViewGroup;
import android.widget.FrameLayout;

public abstract class OrxActivity extends Activity {
	// ===========================================================
	// Fields
	// ===========================================================
	
	private OrxGLSurfaceView mGLSurfaceView;
	
	private boolean mAccelerometerIsEnabled = false;
	private boolean mRequireDepthBuffer = false;
	private OrxAccelerometer mAccelerometer;

	// ===========================================================
	// Constructors
	// ===========================================================

	@Override
	protected void onCreate(final Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		getWindow().setFormat(PixelFormat.RGB_565);
    	init();
	}

	
	// ===========================================================
	// Getter & Setter
	// ===========================================================

	// ===========================================================
	// Methods for/from SuperClass/Interfaces
	// ===========================================================

	@Override
	protected void onStart() {
		super.onStart();

		getWindow().setFormat(PixelFormat.RGB_565);
	}


	@Override
	protected void onResume() {
		super.onResume();

		mGLSurfaceView.onResume();
		
		if(mAccelerometerIsEnabled)
			mAccelerometer.enable();
	}

	@Override
	protected void onPause() {
		super.onPause();

		mGLSurfaceView.onPause();
		
		if(mAccelerometerIsEnabled)
			mAccelerometer.disable();
	}

	// ===========================================================
	// Methods
	// ===========================================================
	public void init() {
		nativeInit();

    	mRequireDepthBuffer = requireDepthBuffer();
    	
		if(getLayoutId() == 0 || getOrxGLSurfaceViewId() == 0) {
	    	// FrameLayout
	        ViewGroup.LayoutParams framelayout_params =
	            new ViewGroup.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT,
	                                       ViewGroup.LayoutParams.FILL_PARENT);
	        FrameLayout framelayout = new FrameLayout(this);
	        framelayout.setLayoutParams(framelayout_params);

	        // OrxxGLSurfaceView
	        mGLSurfaceView = onCreateView();

	        // ...add to FrameLayout
	        framelayout.addView(mGLSurfaceView);
			
	        // Set framelayout as the content view
			setContentView(framelayout);
		} else {
			setContentView(getLayoutId());
			mGLSurfaceView = (OrxGLSurfaceView) findViewById(getOrxGLSurfaceViewId());
		}

        mGLSurfaceView.setOrxRenderer(new OrxRenderer(this));
		mAccelerometer = new OrxAccelerometer(this, mGLSurfaceView);
	}
	
	protected int getLayoutId() {
		return 0;
	}
	
	protected int getOrxGLSurfaceViewId() {
		return 0;
	}
	
    private OrxGLSurfaceView onCreateView() {
    	return new OrxGLSurfaceView(this, mRequireDepthBuffer);
    }
    
    protected void enableAccelerometer() {
    	mAccelerometerIsEnabled = true;
    	mAccelerometer.enable();
    }
    
    private native void nativeInit();
    
    protected void runOnOrxThread(Runnable r) {
    	mGLSurfaceView.queueEvent(r);
    }
    
    private native boolean requireDepthBuffer();
}
