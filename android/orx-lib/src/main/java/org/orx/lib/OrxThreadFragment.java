package org.orx.lib;

import android.app.Activity;
import android.os.Bundle;
import android.support.v4.app.Fragment;

import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Created by philippe on 10/31/13.
 */
public class OrxThreadFragment extends Fragment {

    public static final String TAG = "OrxThreadFragment";

    private AtomicBoolean mRunning = new AtomicBoolean(false);

    private native void startOrx(Fragment fragment);
    private native void nativeOnPause();
    private native void nativeOnResume();
    private native void stopOrx();

    private final Thread mOrxThread = new Thread("OrxThread") {
        @Override
        public void run() {
            startOrx(OrxThreadFragment.this);
            mRunning.set(false);

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
    }

    @Override
    public void onResume() {
        super.onResume();

        if(!mRunning.getAndSet(true)) {
            mOrxThread.start();
        } else {
            nativeOnResume();
        }
    }

    @Override
    public void onPause() {
        super.onPause();

        if(mRunning.get()) {
            nativeOnPause();
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        if(mRunning.get()) {
            stopOrx();
        }
    }
}
