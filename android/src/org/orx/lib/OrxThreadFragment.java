package org.orx.lib;

import android.os.Bundle;
import android.support.v4.app.Fragment;

/**
 * Created by philippe on 10/31/13.
 */
public class OrxThreadFragment extends Fragment {

    public static final String TAG = "OrxThreadFragment";

    private boolean mRunning = false;

    private native void startOrx(Fragment fragment);
    private native void nativeOnPause();
    private native void nativeOnResume();
    private native void nativeOnCreate();
    private native void stopOrx();

    private final Thread mOrxThread = new Thread("OrxThread") {
        @Override
        public void run() {
            startOrx(OrxThreadFragment.this);

            Activity a = getActivity();
            if(a != null) {
                a.finish();
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setRetainInstance(true);
        nativeOnCreate();
    }

    @Override
    public void onResume() {
        super.onResume();

        if(!mRunning) {
            mOrxThread.start();
            mRunning = true;
        } else {
            nativeOnResume();
        }
    }

    @Override
    public void onPause() {
        super.onPause();

        if(mRunning) {
            nativeOnPause();
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        if(mRunning) {
            stopOrx();
            mRunning = false;
        }
    }
}
