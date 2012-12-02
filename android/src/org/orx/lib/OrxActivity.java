package org.orx.lib;

import android.app.Activity;
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
		
    	// FrameLayout
        ViewGroup.LayoutParams framelayout_params =
            new ViewGroup.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT,
                                       ViewGroup.LayoutParams.FILL_PARENT);
        FrameLayout framelayout = new FrameLayout(this);
        framelayout.setLayoutParams(framelayout_params);

        // Cocos2dxGLSurfaceView
        mGLSurfaceView = onCreateView();

        // ...add to FrameLayout
        framelayout.addView(mGLSurfaceView);

        mGLSurfaceView.setOrxRenderer(new OrxRenderer(this));

        // Set framelayout as the content view
		setContentView(framelayout);
		
		mAccelerometer = new OrxAccelerometer(this, mGLSurfaceView);
	}
	
    public OrxGLSurfaceView onCreateView() {
    	return new OrxGLSurfaceView(this);
    }
    
    public void enableAccelerometer() {
    	mAccelerometerIsEnabled = true;
    	mAccelerometer.enable();
    }
    
    private native void nativeInit();
}
