package org.orx.lib;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.WindowManager;

/**
    Orx Activity
*/
public class OrxActivity extends Activity {

    // Keep track of the paused state
    public static boolean mIsPaused = false;

    // Main components
    private static OrxActivity mSingleton;
    private static OrxSurface mSurface;

    // This is what Orx runs in. It invokes Orx_main(), eventually
    private static Thread mOrxThread;

    // EGL private objects
    private static EGLContext  mEGLContext;
    private static EGLSurface  mEGLSurface;
    private static EGLDisplay  mEGLDisplay;
    private static EGLConfig   mEGLConfig;
    private static int mGLMajor, mGLMinor;

    // Setup
    protected void onCreate(Bundle savedInstanceState) {
        Log.v("Orx", "onCreate()");
        super.onCreate(savedInstanceState);
        
        // So we can call stuff from static callbacks
        mSingleton = this;

        // Set up the surface
        mSurface = new OrxSurface(getApplication());

        setContentView(mSurface);

        SurfaceHolder holder = mSurface.getHolder();
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
        OrxActivity.nativePause();
    }

    protected void onResume() {
        Log.v("Orx", "onResume()");
        super.onResume();
        OrxActivity.nativeResume();
    }

    protected void onDestroy() {
        super.onDestroy();
        Log.v("Orx", "onDestroy()");
        // Send a quit message to the application
        OrxActivity.nativeQuit();

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
    public static native void nativeInit();
    public static native void nativeQuit();
    public static native void nativePause();
    public static native void nativeResume();
    public static native void nativeSurfaceDestroyed();
    public static native void nativeSurfaceCreated();
    public static native void onNativeResize(int x, int y);
    public static native void onNativeKeyDown(int keycode);
    public static native void onNativeKeyUp(int keycode);
    public static native void onNativeTouch(int touchDevId, int pointerFingerId,
                                            int action, float x, 
                                            float y, float p);

    // Java functions called from C
    
    public static int getRotation() {
    	WindowManager windowMgr = (WindowManager) mSingleton.getSystemService(WINDOW_SERVICE);
    	int rotationIndex = windowMgr.getDefaultDisplay().getRotation();
    	return rotationIndex;
    }

    public static boolean createGLContext(int majorVersion, int minorVersion, int[] attribs) {
        return initEGL(majorVersion, minorVersion, attribs);
    }

    public static void flipBuffers() {
        flipEGL();
    }

    public static Context getContext() {
        return mSingleton;
    }

    public static void startApp() {
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
                OrxActivity.nativeSurfaceCreated();
                OrxActivity.mIsPaused = false;
            }
        }
    }
    
    public static void finishApp() {
    	mSingleton.finish();
    }
    
    // EGL functions
    public static boolean initEGL(int majorVersion, int minorVersion, int[] attribs) {
        try {
            if (OrxActivity.mEGLDisplay == null) {
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

                OrxActivity.mEGLDisplay = dpy;
                OrxActivity.mEGLConfig = config;
                OrxActivity.mGLMajor = majorVersion;
                OrxActivity.mGLMinor = minorVersion;
            }
            return OrxActivity.createEGLSurface();

        } catch(Exception e) {
            Log.v("Orx", e + "");
            for (StackTraceElement s : e.getStackTrace()) {
                Log.v("Orx", s.toString());
            }
            return false;
        }
    }

    public static boolean createEGLContext() {
        EGL10 egl = (EGL10)EGLContext.getEGL();
        int EGL_CONTEXT_CLIENT_VERSION=0x3098;
        int contextAttrs[] = new int[] { EGL_CONTEXT_CLIENT_VERSION, OrxActivity.mGLMajor, EGL10.EGL_NONE };
        OrxActivity.mEGLContext = egl.eglCreateContext(OrxActivity.mEGLDisplay, OrxActivity.mEGLConfig, EGL10.EGL_NO_CONTEXT, contextAttrs);
        if (OrxActivity.mEGLContext == EGL10.EGL_NO_CONTEXT) {
            Log.e("Orx", "Couldn't create context");
            return false;
        }
        return true;
    }

    public static boolean createEGLSurface() {
        if (OrxActivity.mEGLDisplay != null && OrxActivity.mEGLConfig != null) {
            EGL10 egl = (EGL10)EGLContext.getEGL();
            if (OrxActivity.mEGLContext == null) createEGLContext();

            Log.v("Orx", "Creating new EGL Surface");
            EGLSurface surface = egl.eglCreateWindowSurface(OrxActivity.mEGLDisplay, OrxActivity.mEGLConfig, OrxActivity.mSurface, null);
            if (surface == EGL10.EGL_NO_SURFACE) {
                Log.e("Orx", "Couldn't create surface");
                return false;
            }

            if (egl.eglGetCurrentContext() != OrxActivity.mEGLContext) {
                if (!egl.eglMakeCurrent(OrxActivity.mEGLDisplay, surface, surface, OrxActivity.mEGLContext)) {
                    Log.e("Orx", "Old EGL Context doesnt work, trying with a new one");
                    // TODO: Notify the user via a message that the old context could not be restored, and that textures need to be manually restored.
                    createEGLContext();
                    if (!egl.eglMakeCurrent(OrxActivity.mEGLDisplay, surface, surface, OrxActivity.mEGLContext)) {
                        Log.e("Orx", "Failed making EGL Context current");
                        return false;
                    }
                }
            }
            OrxActivity.mEGLSurface = surface;
            return true;
        } else {
            Log.e("Orx", "Surface creation failed, display = " + OrxActivity.mEGLDisplay + ", config = " + OrxActivity.mEGLConfig);
            return false;
        }
    }

    // EGL buffer flip
    public static void flipEGL() {
        try {
            EGL10 egl = (EGL10)EGLContext.getEGL();

            egl.eglWaitNative(EGL10.EGL_CORE_NATIVE_ENGINE, null);

            // drawing here

            egl.eglWaitGL();

            egl.eglSwapBuffers(OrxActivity.mEGLDisplay, OrxActivity.mEGLSurface);


        } catch(Exception e) {
            Log.v("Orx", "flipEGL(): " + e);
            for (StackTraceElement s : e.getStackTrace()) {
                Log.v("Orx", s.toString());
            }
        }
    }
}

/**
    Simple nativeInit() runnable
*/
class OrxMain implements Runnable {
    public void run() {
        // Runs Orx_main()
        OrxActivity.nativeInit();

        Log.v("Orx", "Orx thread terminated");
        
        OrxActivity.finishApp();
    }
}