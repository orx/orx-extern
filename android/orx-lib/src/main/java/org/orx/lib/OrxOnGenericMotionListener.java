package org.orx.lib;

import android.annotation.TargetApi;
import android.os.Build;
import android.view.MotionEvent;
import android.view.View;

import org.orx.lib.inputmanagercompat.InputManagerCompat;

/**
 * Created by philippe on 29/10/14.
 */
@TargetApi(Build.VERSION_CODES.HONEYCOMB_MR1)
public class OrxOnGenericMotionListener implements View.OnGenericMotionListener {

    private InputManagerCompat mInputManager;
    private OrxActivity mOrxActivity;

    public OrxOnGenericMotionListener(OrxActivity activity, InputManagerCompat inputManager) {
        mOrxActivity = activity;
        mInputManager = inputManager;
    }

    @Override
    public boolean onGenericMotion(View view, MotionEvent motionEvent) {
        mInputManager.onGenericMotionEvent(motionEvent);
        return view.onGenericMotionEvent(motionEvent);
    }
}
