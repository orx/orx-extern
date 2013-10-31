package org.orx.lib;

import android.os.Bundle;
import android.support.v4.app.Fragment;

/**
 * Created by philippe on 10/31/13.
 */
public class OrxThreadFragment extends Fragment {

    public static final String TAG = "OrxThreadFragment";

    private boolean mRunning = false;

    private native void runOrx(Fragment fragment);
    private native void nativePause();
    private native void nativeResume();
    private native void nativeCreate();
    private native void nativeQuit();

    private final Thread mOrxThread = new Thread("OrxThread") {
        @Override
        public void run() {
            runOrx(OrxThreadFragment.this);
            getActivity().finish();
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setRetainInstance(true);
        nativeCreate();
    }

    @Override
    public void onResume() {
        super.onResume();

        if(!mRunning) {
            mOrxThread.start();
            mRunning = true;
        } else {
            nativeResume();
        }
    }

    @Override
    public void onPause() {
        super.onPause();

        if(mRunning) {
            nativePause();
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        if(mRunning) {
            nativeQuit();
            mRunning = false;
        }
    }
}
