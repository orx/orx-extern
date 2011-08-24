VPATH = ../../src

SRC = \
	image_DXT.c \
	image_helper.c \
	SOIL.c \
	stb_image_aug.c
	
armeabi_OBJ = $(addprefix armeabi/,$(SRC:.c=.o))
armeabi-v7a_OBJ = $(addprefix armeabi-v7a/,$(SRC:.c=.o))
	
armeabi_OUT = armeabi/libSOIL.so
armeabi-v7a_OUT = armeabi-v7a/libSOIL.so
	
INCLUDES = -I../../include
	
CC = arm-linux-androideabi-gcc
CFLAGS = -O2 -DANDROID -fPIC
	
all: $(armeabi_OUT) $(armeabi-v7a_OUT)

armeabi/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

armeabi-v7a/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -c $< -o $@

$(armeabi_OUT): $(armeabi_OBJ)
	$(CC) -shared -Wl,-soname,$(armeabi_OUT) -o $(armeabi_OUT) $(armeabi_OBJ) -lm

$(armeabi-v7a_OUT): $(armeabi-v7a_OBJ)
	$(CC) -shared -Wl,-soname,$(armeabi-v7a_OUT) -o $(armeabi-v7a_OUT) $(armeabi-v7a_OBJ) -lm

clean:
	rm -f $(armeabi_OBJ) $(armeabi_OUT) $(armeabi-v7a_OBJ) $(armeabi-v7a_OUT)

