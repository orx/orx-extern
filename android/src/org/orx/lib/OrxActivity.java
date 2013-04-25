package org.orx.lib;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

import android.app.Activity;
import android.graphics.PixelFormat;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;

/**
    Orx Activity
*/
public class OrxActivity extends Activity {

    // Keep track of the paused state
    boolean mIsPaused = false;
    
    // Main components
    private OrxGLSurfaceView mSurface;

    // This is what Orx runs in. It invokes Orx_main(), eventually
    private Thread mOrxThread;

    // EGL private objects
    private EGLContext  mEGLContext;
    private EGLSurface  mEGLSurface;
    private EGLDisplay  mEGLDisplay;
    private EGLConfig   mEGLConfig;
    private int mGLMajor, mGLMinor;

    // Setup
    protected void onCreate(Bundle savedInstanceState) {
        Log.v("Orx", "onCreate()");
        super.onCreate(savedInstanceState);
        
        getWindow().setFormat(PixelFormat.RGB_565);
        init();
    }
    
    @Override
    protected void onStart() {
    	super.onStart();
    	
    	getWindow().setFormat(PixelFormat.RGB_565);
    }
    
    private void init() {
    	if(getLayoutId() == 0 || getOrxGLSurfaceViewId() == 0) {
            // Set up the surface
            mSurface = new OrxGLSurfaceView(getApplication());
            setContentView(mSurface);
    	} else {
			setContentView(getLayoutId());
			mSurface = (OrxGLSurfaceView) findViewById(getOrxGLSurfaceViewId());
		}
    	
    	mSurface.setActivity(this);
    }

    protected int getLayoutId() {
		/*
		 * Override this if you want to use a custom layout
		 * return the layout id
		 */
		return 0;
	}
    
    protected int getOrxGLSurfaceViewId() {
		/*
		 * Override this if you want to use a custom layout
		 * return the OrxGLSurfaceView id
		 */
		return 0;
	}
    
    // Events
    protected void onPause() {
        Log.v("Orx", "onPause()");
        super.onPause();
        nativePause();
    }

    protected void onResume() {
        Log.v("Orx", "onResume()");
        super.onResume();
        nativeResume();
    }

    protected void onDestroy() {
        super.onDestroy();
        Log.v("Orx", "onDestroy()");
        // Send a quit message to the application
        nativeQuit();

        // Now wait for the Orx thread to quit
        if (mOrxThread != null) {
            try {
                mOrxThread.join();
            } catch(Exception e) {
                Log.v("Orx", "Problem stopping thread: " + e);
            }
            mOrxThread = null;

            Log.v("Orx", "Finished waiting for Orx thread");
        }
    }

    // C functions we call
    native void nativeInit();
    native void nativeQuit();
    native void nativePause();
    native void nativeResume();
    native void nativeSurfaceDestroyed();
    native void nativeSurfaceCreated();
    native void onNativeResize(int x, int y);
    native void onNativeKeyDown(int keycode);
    native void onNativeKeyUp(int keycode);
    native void onNativeTouch(int touchDevId, int pointerFingerId,
                                            int action, float x, 
                                            float y, float p);

    // Java functions called from C
    
    public int getRotation() {
    	WindowManager windowMgr = (WindowManager) getSystemService(WINDOW_SERVICE);
    	int rotationIndex = windowMgr.getDefaultDisplay().getRotation();
    	return rotationIndex;
    }

    public boolean createGLContext(int majorVersion, int minorVersion, int[] attribs) {
        return initEGL(majorVersion, minorVersion, attribs);
    }

    public void flipBuffers() {
        flipEGL();
    }

    void startApp() {
        // Start up the C app thread
        if (mOrxThread == null) {
            mOrxThread = new Thread(new OrxMain(), "OrxThread");
            mOrxThread.start();
        }
        else {
            /*
             * Some Android variants may send multiple surfaceChanged events, so we don't need to resume every time
             * every time we get one of those events, only if it comes after surfaceDestroyed
             */
            if (mIsPaused) {
                nativeSurfaceCreated();
                mIsPaused = false;
            }
        }
    }
    
    private void finishApp() {
    	finish();
    }
    
    // EGL functions
    private boolean initEGL(int majorVersion, int minorVersion, int[] attribs) {
        try {
            if (mEGLDisplay == null) {
                Log.v("Orx", "Starting up OpenGL ES " + majorVersion + "." + minorVersion);

                EGL10 egl = (EGL10)EGLContext.getEGL();

                EGLDisplay dpy = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);

                int[] version = new int[2];
                egl.eglInitialize(dpy, version);

                EGLConfig[] configs = new EGLConfig[1];
                int[] num_config = new int[1];
                if (!egl.eglChooseConfig(dpy, attribs, configs, 1, num_config) || num_config[0] == 0) {
                    Log.e("Orx", "No EGL config available");
                    return false;
                }
                EGLConfig config = configs[0];

                mEGLDisplay = dpy;
                mEGLConfig = config;
                mGLMajor = majorVersion;
                mGLMinor = minorVersion;
            }
            return createEGLSurface();

        } catch(Exception e) {
            Log.v("Orx", e + "");
            for (StackTraceElement s : e.getStackTrace()) {
                Log.v("Orx", s.toString());
            }
            return false;
        }
    }

    private boolean createEGLContext() {
        EGL10 egl = (EGL10)EGLContext.getEGL();
        int EGL_CONTEXT_CLIENT_VERSION=0x3098;
        int contextAttrs[] = new int[] { EGL_CONTEXT_CLIENT_VERSION, mGLMajor, EGL10.EGL_NONE };
        mEGLContext = egl.eglCreateContext(mEGLDisplay, mEGLConfig, EGL10.EGL_NO_CONTEXT, contextAttrs);
        if (mEGLContext == EGL10.EGL_NO_CONTEXT) {
            Log.e("Orx", "Couldn't create context");
            return false;
        }
        return true;
    }

    private boolean createEGLSurface() {
        if (mEGLDisplay != null && mEGLConfig != null) {
            EGL10 egl = (EGL10)EGLContext.getEGL();
            if (mEGLContext == null) createEGLContext();

            Log.v("Orx", "Creating new EGL Surface");
            EGLSurface surface = egl.eglCreateWindowSurface(mEGLDisplay, mEGLConfig, mSurface, null);
            if (surface == EGL10.EGL_NO_SURFACE) {
                Log.e("Orx", "Couldn't create surface");
                return false;
            }

            if (egl.eglGetCurrentContext() != mEGLContext) {
                if (!egl.eglMakeCurrent(mEGLDisplay, surface, surface, mEGLContext)) {
                    Log.e("Orx", "Old EGL Context doesnt work, trying with a new one");
                    // TODO: Notify the user via a message that the old context could not be restored, and that textures need to be manually restored.
                    createEGLContext();
                    if (!egl.eglMakeCurrent(mEGLDisplay, surface, surface, mEGLContext)) {
                        Log.e("Orx", "Failed making EGL Context current");
                        return false;
                    }
                }
            }
            mEGLSurface = surface;
            return true;
        } else {
            Log.e("Orx", "Surface creation failed, display = " + mEGLDisplay + ", config = " + mEGLConfig);
            return false;
        }
    }

    // EGL buffer flip
    public void flipEGL() {
        try {
            EGL10 egl = (EGL10)EGLContext.getEGL();

            egl.eglWaitNative(EGL10.EGL_CORE_NATIVE_ENGINE, null);

            // drawing here

            egl.eglWaitGL();

            egl.eglSwapBuffers(mEGLDisplay, mEGLSurface);


        } catch(Exception e) {
            Log.v("Orx", "flipEGL(): " + e);
            for (StackTraceElement s : e.getStackTrace()) {
                Log.v("Orx", s.toString());
            }
        }
    }
    
	/**
	 * Simple nativeInit() runnable
	 */
	class OrxMain implements Runnable {
		public void run() {
			// Runs Orx_main()
			nativeInit();

			Log.v("Orx", "Orx thread terminated");

			finishApp();
		}
	}
}

