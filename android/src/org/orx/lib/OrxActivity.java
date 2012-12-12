package org.orx.lib;

import android.app.Activity;
import android.graphics.PixelFormat;
import android.os.Bundle;
import android.view.ViewGroup;
import android.widget.FrameLayout;

public abstract class OrxActivity extends Activity {
	// ===========================================================
	// Fields
	// ===========================================================
	
	private OrxGLSurfaceView mGLSurfaceView;
	
	private boolean mAccelerometerIsEnabled = false;
	private OrxAccelerometer mAccelerometer;

	// ===========================================================
	// Constructors
	// ===========================================================

	@Override
	protected void onCreate(final Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
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
		APKFileHelper.getInstance().setContext(this);
		nativeInit();
		
		if(mGLSurfaceView == null) {
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
		}

        mGLSurfaceView.setOrxRenderer(new OrxRenderer(this));
		mAccelerometer = new OrxAccelerometer(this, mGLSurfaceView);
	}
	
    private OrxGLSurfaceView onCreateView() {
    	return new OrxGLSurfaceView(this, requireDepthBuffer());
    }
    
    protected void enableAccelerometer() {
    	mAccelerometerIsEnabled = true;
    	mAccelerometer.enable();
    }
    
    protected void setOrxGLSurfaceView(OrxGLSurfaceView orxGLSurfaceView) {
    	if(mGLSurfaceView != null)
    		throw new IllegalStateException("OrxGLSurfaceView not null!");
    	
    	mGLSurfaceView = orxGLSurfaceView;
    }
    
    private native void nativeInit();
    
    protected void runOnOrxThread(Runnable r) {
    	mGLSurfaceView.queueEvent(r);
    }
    
    protected boolean requireDepthBuffer() {
    	return false;
    }
}
