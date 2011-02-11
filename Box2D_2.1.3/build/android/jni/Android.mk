LOCAL_PATH := $(call my-dir)

SRC_FILES := 
include $(call all-subdir-makefiles) 
include $(CLEAR_VARS)

LOCAL_MODULE    := anbox2d
LOCAL_CFLAGS := -DANDROID
LOCAL_CFLAGS += -DRELEASE
LOCAL_CFLAGS += -DCHINESE1
LOCAL_CFLAGS += -I$(LOCAL_PATH)/../../../include

LOCAL_CPPFLAGS := -DANDROID
#LOCAL_CPPFLAGS := -O0 -gstabs+
LOCAL_CPPFLAGS := -O3 -gstabs+
LOCAL_CPPFLAGS += -DRELEASE
LOCAL_CPPFLAGS += -DCHINESE1
LOCAL_CPPFLAGS += -I$(LOCAL_PATH)/../../../include
LOCAL_ARM_MODE := arm
LOCAL_DEFAULT_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := ../../../src/Collision/b2CollidePolygon.cpp \
../../../src/Collision/b2Distance.cpp \
../../../src/Collision/b2CollideEdge.cpp \
../../../src/Collision/Shapes/b2CircleShape.cpp \
../../../src/Collision/Shapes/b2PolygonShape.cpp \
../../../src/Collision/Shapes/b2LoopShape.cpp \
../../../src/Collision/Shapes/b2EdgeShape.cpp \
../../../src/Collision/b2Collision.cpp \
../../../src/Collision/b2DynamicTree.cpp \
../../../src/Collision/b2CollideCircle.cpp \
../../../src/Collision/b2TimeOfImpact.cpp \
../../../src/Collision/b2BroadPhase.cpp \
../../../src/Common/b2StackAllocator.cpp \
../../../src/Common/b2Math.cpp \
../../../src/Common/b2BlockAllocator.cpp \
../../../src/Common/b2Settings.cpp \
../../../src/Dynamics/Joints/b2PrismaticJoint.cpp \
../../../src/Dynamics/Joints/b2FrictionJoint.cpp \
../../../src/Dynamics/Joints/b2DistanceJoint.cpp \
../../../src/Dynamics/Joints/b2WeldJoint.cpp \
../../../src/Dynamics/Joints/b2GearJoint.cpp \
../../../src/Dynamics/Joints/b2LineJoint.cpp \
../../../src/Dynamics/Joints/b2Joint.cpp \
../../../src/Dynamics/Joints/b2PulleyJoint.cpp \
../../../src/Dynamics/Joints/b2MouseJoint.cpp \
../../../src/Dynamics/Joints/b2RevoluteJoint.cpp \
../../../src/Dynamics/Joints/b2RopeJoint.cpp \
../../../src/Dynamics/b2Island.cpp \
../../../src/Dynamics/b2ContactManager.cpp \
../../../src/Dynamics/b2Fixture.cpp \
../../../src/Dynamics/b2World.cpp \
../../../src/Dynamics/Contacts/b2CircleContact.cpp \
../../../src/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp \
../../../src/Dynamics/Contacts/b2PolygonContact.cpp \
../../../src/Dynamics/Contacts/b2PolygonAndCircleContact.cpp \
../../../src/Dynamics/Contacts/b2LoopAndCircleContact.cpp \
../../../src/Dynamics/Contacts/b2Contact.cpp \
../../../src/Dynamics/Contacts/b2LoopAndPolygonContact.cpp \
../../../src/Dynamics/Contacts/b2EdgeAndCircleContact.cpp \
../../../src/Dynamics/Contacts/b2ContactSolver.cpp \
../../../src/Dynamics/b2Body.cpp \
../../../src/Dynamics/b2WorldCallbacks.cpp



$(info variable Local SRC_FILES $(LOCAL_SRC_FILES) )    

LOCAL_LDLIBS := -ldl -llog -lm -lgcc

include $(BUILD_STATIC_LIBRARY)

# second lib, which will depend on and include the first one
#
#include $(CLEAR_VARS)

#LOCAL_MODULE    := libtwolib-second
#LOCAL_SRC_FILES := second.c

#LOCAL_STATIC_LIBRARIES := anbox2d

#include $(BUILD_SHARED_LIBRARY)
