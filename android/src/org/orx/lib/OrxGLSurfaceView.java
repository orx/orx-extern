package org.orx.lib;

import android.content.Context;
import android.util.AttributeSet;
import android.view.SurfaceView;

/**
 * OrxSurface. This is what we draw on, so we need to know when it's created in
 * order to do anything useful.
 * 
 * Because of this, that's where we set up the Orx thread
 */
public class OrxGLSurfaceView extends SurfaceView {

	// Startup
	public OrxGLSurfaceView(Context context) {
		super(context);
	}
	
	public OrxGLSurfaceView(final Context context, final AttributeSet attrs) {
		super(context, attrs);
	}
}