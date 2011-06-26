VPATH = ../../src

SRC = \
	image_DXT.c \
	image_helper.c \
	SOIL.c \
	stb_image_aug.c
	
armeabi_OBJ = $(addprefix armeabi/,$(SRC:.c=.o))
armeabi-v7a_OBJ = $(addprefix armeabi-v7a/,$(SRC:.c=.o))
	
armeabi_OUT = armeabi/libSOIL.a
armeabi-v7a_OUT = armeabi-v7a/libSOIL.a
	
INCLUDES = -I../../include
	
CC = arm-linux-androideabi-gcc
AR = arm-linux-androideabi-ar
CFLAGS = -O2 -DANDROID
	
all: $(armeabi_OUT) $(armeabi-v7a_OUT)

armeabi/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

armeabi-v7a/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) -march=armv7-a -mfloat-abi=softfp -c $< -o $@

$(armeabi_OUT): $(armeabi_OBJ)
	$(AR) rcs $(armeabi_OUT) $(armeabi_OBJ)

$(armeabi-v7a_OUT): $(armeabi-v7a_OBJ)
	$(AR) rcs $(armeabi-v7a_OUT) $(armeabi-v7a_OBJ)

clean:
	rm -f $(armeabi_OBJ) $(armeabi_OUT) $(armeabi-v7a_OBJ) $(armeabi-v7a_OUT)

