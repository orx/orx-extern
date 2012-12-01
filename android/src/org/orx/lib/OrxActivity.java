package org.orx.lib;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.os.Bundle;
import android.util.Log;
import android.view.ViewGroup;
import android.widget.FrameLayout;

public abstract class OrxActivity extends Activity {
	// ===========================================================
	// Fields
	// ===========================================================
	
	private OrxGLSurfaceView mGLSurfaceView;

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
	}

	@Override
	protected void onPause() {
		super.onPause();

		mGLSurfaceView.onPause();
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
	}
	
    public OrxGLSurfaceView onCreateView() {
    	return new OrxGLSurfaceView(this);
    }
    
    private native void nativeInit();

	// ===========================================================
	// Inner and Anonymous Classes
	// ===========================================================
}
