VPATH = ../jni

C_SRC = \
	nv_apk_file/nv_apk_file.c   \
	nv_file/nv_file.c           \
	nv_thread/nv_thread.c    
	

CPP_SRC = \
  nv_math/nv_math.cpp         \
  nv_math/nv_matrix.cpp       \
  nv_math/nv_quat.cpp         \
	nv_shader/nv_shader.cpp     \
	nv_util/nv_util.cpp         \
	nv_time/nv_time.cpp         \
	nv_event/nv_event.cpp       \
	nv_event/nv_event_queue.cpp \
	nv_hhdds/nv_hhdds.cpp       \
	nv_log/nv_log.cpp           \
	nv_glesutil/nv_images.cpp   \
	nv_sound/nv_sound.cpp
	
  
armeabi_OBJ = $(addprefix armeabi/,$(CPP_SRC:.cpp=.o)) $(addprefix armeabi/,$(C_SRC:.c=.o))
armeabi-v7a_OBJ = $(addprefix armeabi-v7a/,$(CPP_SRC:.cpp=.o)) $(addprefix armeabi-v7a/,$(C_SRC:.c=.o))

armeabi_OUT = armeabi/libNvEvent.a
armeabi-v7a_OUT = armeabi-v7a/libNvEvent.a

CXX = arm-linux-androideabi-g++
CC = arm-linux-androideabi-gcc
AR = arm-linux-androideabi-ar
CPPFLAGS = -fno-exceptions -fno-rtti -DANDROID
CFLAGS = -DANDROID
ARMV7A_FLAGS = -march=armv7-a -mfloat-abi=softfp -O2
ARMV5TE_FLAGS = -O2
	
all: $(armeabi_OUT) $(armeabi-v7a_OUT)

armeabi/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(ARMV5TE_FLAGS) -c $< -o $@

armeabi/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(ARMV5TE_FLAGS) -c $< -o $@

armeabi-v7a/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(ARMV7A_FLAGS) -c $< -o $@

armeabi-v7a/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(ARMV7A_FLAGS) -c $< -o $@

$(armeabi_OUT): $(armeabi_OBJ)
	$(AR) rcs $(armeabi_OUT) $(armeabi_OBJ)

$(armeabi-v7a_OUT): $(armeabi-v7a_OBJ)
	$(AR) rcs $(armeabi-v7a_OUT) $(armeabi-v7a_OBJ)

clean:
	rm -f $(armeabi_OBJ) $(armeabi-v7a_OBJ) $(armeabi_OUT) $(armeabi-v7a_OUT)
