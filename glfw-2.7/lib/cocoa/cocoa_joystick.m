//========================================================================
// GLFW - An OpenGL framework
// File:        macosx_joystick.m
// Platform:    Mac OS X
// API Version: 2.7
// WWW:         http://glfw.sourceforge.net
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Camilla Berglund
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include "internal.h"
#import <Foundation/Foundation.h>

//************************************************************************
//****               Platform implementation functions                ****
//************************************************************************

static void _joystickConnected( void *context, IOReturn res, void *sender, 
                                IOHIDDeviceRef device ) 
{
    int i,j;
    
    for ( i = 0; i <= GLFW_JOYSTICK_LAST; i++ ) 
    {
        if ( !_glfwJoy[ i ].Present ) 
        {
            _glfwJoy[ i ].Present = GL_TRUE;
            _glfwJoy[ i ].device = device;

            NSArray* elements = (NSArray*) IOHIDDeviceCopyMatchingElements( 
                device, NULL, kIOHIDOptionsTypeNone );
    
            NSMutableArray* buttons = [[NSMutableArray alloc] init];
            NSMutableArray* axes = [[NSMutableArray alloc] init];

            NSNumber* min = [[NSNumber alloc] initWithDouble:-1.0];
            NSNumber* max = [[NSNumber alloc] initWithDouble:1.0];

            for ( id element in elements ) 
            {
                switch( IOHIDElementGetUsagePage( (IOHIDElementRef) element ) ) 
                {
                case kHIDPage_Button:
                    [buttons addObject:element];
                    break;
                case kHIDPage_GenericDesktop:
                    switch( IOHIDElementGetUsage( (IOHIDElementRef) element ) ) 
                    {
                    case kHIDUsage_GD_X:
                    case kHIDUsage_GD_Y:
                    case kHIDUsage_GD_Z:
                    case kHIDUsage_GD_Rx:
                    case kHIDUsage_GD_Ry:
                    case kHIDUsage_GD_Rz:
                        IOHIDElementSetProperty( 
                            (IOHIDElementRef) element, 
                            CFSTR( kIOHIDElementCalibrationMinKey ), min );
                        IOHIDElementSetProperty( 
                            (IOHIDElementRef) element,
                            CFSTR( kIOHIDElementCalibrationMaxKey ), max );
                        [axes addObject:element];
                        break;
                    }
                }
            }

            [min release];
            [max release];
            [elements release];
            
            _glfwJoy[ i ].NumButtons = buttons.count;
            _glfwJoy[ i ].Button = malloc( 
                buttons.count * sizeof( IOHIDElementRef ) );

            for ( j = 0; j < buttons.count; j++ ) 
            {
                _glfwJoy[ i ].Button[ j ] = (IOHIDElementRef) [buttons objectAtIndex:j];
            }
            [buttons release];
            
            _glfwJoy[ i ].NumAxes = axes.count;
            _glfwJoy[ i ].Axis = malloc( 
                axes.count * sizeof( IOHIDElementRef ) );
            for ( j = 0; j < axes.count; j++ ) 
            {
                _glfwJoy[ i ].Axis[ j ] = (IOHIDElementRef) [axes objectAtIndex:j];
            }
            [axes release];
            break;
        }
    }
}

static void _joystickRemoved( void *context, IOReturn res, void *sender, 
                              IOHIDDeviceRef device ) 
{
    int i, j;
    
    for ( i = 0; i <= GLFW_JOYSTICK_LAST; i++ ) 
    {
        if ( _glfwJoy[ i ].Present && _glfwJoy[ i ].device == device ) 
        {
            _glfwJoy[ i ].Present = GL_FALSE;
            for ( j = 0; j < _glfwJoy[ i ].NumButtons; j++)
                CFRelease(_glfwJoy[ i ].Button[ j ]);
            for ( j = 0; j < _glfwJoy[ i ].NumAxes; j++)
                CFRelease(_glfwJoy[ i ].Axis[ j ]);
            CFRelease(_glfwJoy[ i ].device);
            free(_glfwJoy[ i ].Axis);
            free(_glfwJoy[ i ].Button);
            _glfwJoy[ i ].device = NULL;
            break;
        }
    }
}

void _glfwInitJoysticks( void )
{
    IOHIDManagerRef mgr = IOHIDManagerCreate( kCFAllocatorDefault, 0L );
    
    if ( CFGetTypeID( mgr ) != IOHIDManagerGetTypeID() ) 
    {
        return;
    }

    // Setup array of matching-dicts for manager
    NSNumber* page = [[NSNumber alloc] 
                         initWithUnsignedInt:kHIDPage_GenericDesktop];
    NSNumber* js_usage = [[NSNumber alloc] 
                             initWithUnsignedInt:kHIDUsage_GD_Joystick];
    NSNumber* gp_usage = [[NSNumber alloc] 
                             initWithUnsignedInt:kHIDUsage_GD_GamePad];
    NSString* page_key = [[NSString alloc] 
                             initWithString:@kIOHIDDeviceUsagePageKey];
    NSString* usage_key = [[NSString alloc] 
                              initWithString:@kIOHIDDeviceUsageKey];
    NSDictionary* js = [[NSDictionary alloc] 
                           initWithObjectsAndKeys:page, page_key, 
                           js_usage, usage_key, nil];
    NSDictionary* gp = [[NSDictionary alloc] 
                           initWithObjectsAndKeys:page, page_key, 
                           gp_usage, usage_key, nil];

    NSArray* match_dicts = [[NSArray alloc] initWithObjects:js, gp, nil];

    IOHIDManagerSetDeviceMatchingMultiple( mgr, (CFArrayRef) match_dicts );

    [page release];
    [js_usage release];
    [gp_usage release];
    [page_key release];
    [usage_key release];
    [js release];
    [gp release];
    [match_dicts release];

    // Bind add/remove device callbacks
    IOHIDManagerRegisterDeviceMatchingCallback( mgr, &_joystickConnected, NULL );
    IOHIDManagerRegisterDeviceRemovalCallback( mgr, &_joystickRemoved, NULL );

    // HIDManager needs to be attached to runloop to recieve events.
    IOHIDManagerScheduleWithRunLoop( mgr, CFRunLoopGetCurrent(),
                                     kCFRunLoopDefaultMode );

    IOHIDManagerOpen( mgr, kIOHIDOptionsTypeNone );
}

//========================================================================
// Determine joystick capabilities
//========================================================================

int _glfwPlatformGetJoystickParam( int joy, int param )
{
    // Is joystick present?
    if( !_glfwJoy[ joy ].Present )
    {
        return 0;
    }

    switch( param )
    {
    case GLFW_PRESENT:
        return GL_TRUE;

    case GLFW_AXES:
        return _glfwJoy[ joy ].NumAxes;

    case GLFW_BUTTONS:
        return _glfwJoy[ joy ].NumButtons;

    default:
        break;
    }

    return 0;
}

//========================================================================
// Get joystick axis positions
//========================================================================

int _glfwPlatformGetJoystickPos( int joy, float *pos, int numaxes )
{
    int i;
    IOHIDValueRef value;
    if ( numaxes > _glfwJoy[ joy ].NumAxes ) 
        numaxes = _glfwJoy[ joy ].NumAxes;
    for ( i = 0; i < numaxes; i++ ) 
    {
        IOHIDDeviceGetValue( _glfwJoy[ joy ].device , _glfwJoy[ joy ].Axis[ i ],
                             &value );
        pos[ i ] = (float) IOHIDValueGetScaledValue( 
            value, kIOHIDValueScaleTypeCalibrated );
    }
    return numaxes;
}


//========================================================================
// Get joystick button states
//========================================================================

int _glfwPlatformGetJoystickButtons( int joy, unsigned char *buttons, int numbuttons )
{
    int i;
    IOHIDValueRef value;
    if ( numbuttons > _glfwJoy[ joy ].NumButtons ) 
        numbuttons = _glfwJoy[ joy ].NumButtons;
    for ( i = 0; i < numbuttons; i++ ) 
    {
        IOHIDDeviceGetValue( _glfwJoy[ joy ].device , _glfwJoy[ joy ].Button[ i ], 
                             &value );
        buttons[ i ] = (unsigned char) IOHIDValueGetIntegerValue( value );
    }
    return numbuttons;
}

