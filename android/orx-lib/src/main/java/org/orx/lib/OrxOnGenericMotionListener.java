package org.orx.lib;

import android.annotation.TargetApi;
import android.os.Build;
import android.view.InputDevice;
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

        int eventSource = motionEvent.getSource();
        if ((((eventSource & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD) ||
                ((eventSource & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK))
                && motionEvent.getAction() == MotionEvent.ACTION_MOVE) {

            // int id = motionEvent.getDeviceId();

            // Process all historical movement samples in the batch.
            final int historySize = motionEvent.getHistorySize();
            for (int i = 0; i < historySize; i++) {
                processJoystickInput(motionEvent, i);
            }

            // Process the current movement sample in the batch.
            processJoystickInput(motionEvent, -1);

            return true;
        }

        return view.onGenericMotionEvent(motionEvent);
    }

    private void processJoystickInput(MotionEvent event, int historyPos) {
        // Get joystick position.
        // Many game pads with two joysticks report the position of the
        // second joystick
        // using the Z and RZ axes so we also handle those.

        InputDevice device = event.getDevice();

        float x = getCenteredAxis(event, device, MotionEvent.AXIS_X, historyPos);
        if (x == 0) {
            x = getCenteredAxis(event, device, MotionEvent.AXIS_HAT_X, historyPos);
        }
        if (x == 0) {
            x = getCenteredAxis(event, device, MotionEvent.AXIS_Z, historyPos);
        }

        float y = getCenteredAxis(event, device, MotionEvent.AXIS_Y, historyPos);
        if (y == 0) {
            y = getCenteredAxis(event, device, MotionEvent.AXIS_HAT_Y, historyPos);
        }
        if (y == 0) {
            y = getCenteredAxis(event, device, MotionEvent.AXIS_RZ, historyPos);
        }
    }

    private static float getCenteredAxis(MotionEvent event, InputDevice device,
                                         int axis, int historyPos) {
        final InputDevice.MotionRange range = device.getMotionRange(axis, event.getSource());
        if (range != null) {
            final float flat = range.getFlat();
            final float value = historyPos < 0 ? event.getAxisValue(axis)
                    : event.getHistoricalAxisValue(axis, historyPos);

            // Ignore axis values that are within the 'flat' region of the
            // joystick axis center.
            // A joystick at rest does not always report an absolute position of
            // (0,0).
            if (Math.abs(value) > flat) {
                return value;
            }
        }
        return 0;
    }
}
