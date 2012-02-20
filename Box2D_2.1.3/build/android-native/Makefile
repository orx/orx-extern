VPATH = ../../src

SRC = \
	Dynamics/b2ContactManager.cpp \
	Dynamics/b2Fixture.cpp \
	Dynamics/b2WorldCallbacks.cpp \
	Dynamics/b2Body.cpp \
	Dynamics/b2World.cpp \
	Dynamics/Contacts/b2ContactSolver.cpp \
	Dynamics/Contacts/b2Contact.cpp \
	Dynamics/Contacts/b2PolygonAndCircleContact.cpp \
	Dynamics/Contacts/b2CircleContact.cpp \
	Dynamics/Contacts/b2EdgeAndCircleContact.cpp \
	Dynamics/Contacts/b2LoopAndCircleContact.cpp \
	Dynamics/Contacts/b2PolygonContact.cpp \
	Dynamics/Contacts/b2LoopAndPolygonContact.cpp \
	Dynamics/Contacts/b2EdgeAndPolygonContact.cpp \
	Dynamics/b2Island.cpp \
	Dynamics/Joints/b2Joint.cpp \
	Dynamics/Joints/b2PulleyJoint.cpp \
	Dynamics/Joints/b2PrismaticJoint.cpp \
	Dynamics/Joints/b2WeldJoint.cpp \
	Dynamics/Joints/b2LineJoint.cpp \
	Dynamics/Joints/b2RopeJoint.cpp \
	Dynamics/Joints/b2RevoluteJoint.cpp \
	Dynamics/Joints/b2MouseJoint.cpp \
	Dynamics/Joints/b2FrictionJoint.cpp \
	Dynamics/Joints/b2DistanceJoint.cpp \
	Dynamics/Joints/b2GearJoint.cpp \
	Collision/b2CollidePolygon.cpp \
	Collision/b2BroadPhase.cpp \
	Collision/b2CollideCircle.cpp \
	Collision/b2Distance.cpp \
	Collision/b2TimeOfImpact.cpp \
	Collision/b2DynamicTree.cpp \
	Collision/b2Collision.cpp \
	Collision/b2CollideEdge.cpp \
	Collision/Shapes/b2EdgeShape.cpp \
	Collision/Shapes/b2PolygonShape.cpp \
	Collision/Shapes/b2CircleShape.cpp \
	Collision/Shapes/b2LoopShape.cpp \
	Common/b2StackAllocator.cpp \
	Common/b2Settings.cpp \
	Common/b2BlockAllocator.cpp \
	Common/b2Math.cpp
	
armeabi_OBJ = $(addprefix armeabi/,$(SRC:.cpp=.o))
armeabi-v7a_OBJ = $(addprefix armeabi-v7a/,$(SRC:.cpp=.o))
	
armeabi_OUT = armeabi/libBox2D.2.1.3.a
armeabi-v7a_OUT = armeabi-v7a/libBox2D.2.1.3.a
	
INCLUDES = -I../../include
	
CXX = arm-linux-androideabi-g++
AR = arm-linux-androideabi-ar
CPPFLAGS = -fno-exceptions -fno-rtti -O2

all: $(armeabi_OUT) $(armeabi-v7a_OUT)
	
armeabi/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDES) $(CPPFLAGS) -c $< -o $@

armeabi-v7a/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDES) $(CPPFLAGS) -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -c $< -o $@

$(armeabi_OUT): $(armeabi_OBJ)
	$(AR) rcs $(armeabi_OUT) $(armeabi_OBJ)

$(armeabi-v7a_OUT): $(armeabi-v7a_OBJ)
	$(AR) rcs $(armeabi-v7a_OUT) $(armeabi-v7a_OBJ)

clean:
	rm -f $(armeabi_OBJ) $(armeabi_OUT) $(armeabi-v7a_OBJ) $(armeabi-v7a_OUT)

