# SAVE AS: Makefile   (the indented lines MUST start with a TAB, not spaces)
ifeq ($(strip $(DEVKITARM)),)
$(error DEVKITARM not set)
endif
TARGET := neonswarm
CTR := $(DEVKITPRO)/libctru
PORT := $(DEVKITPRO)/portlibs/3ds
ARCH := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS := -g -O2 -mword-relocations -ffunction-sections $(ARCH) -D__3DS__ -I$(CTR)/include -I$(PORT)/include
LDFLAGS := -specs=3dsx.specs -g $(ARCH) -L$(CTR)/lib -L$(PORT)/lib
LIBS := -lcitro2d -lcitro3d -lctru -lm

all: $(TARGET).3dsx

$(TARGET).elf: source/main.c
	arm-none-eabi-gcc $(CFLAGS) source/main.c $(LDFLAGS) $(LIBS) -o $@

$(TARGET).smdh: icon.png
	smdhtool --create "Neon Swarm" "Survivors" "You" icon.png $@

$(TARGET).3dsx: $(TARGET).elf $(TARGET).smdh
	3dsxtool $(TARGET).elf $@ --smdh=$(TARGET).smdh

clean:
	rm -f *.elf *.3dsx *.smdh *.cia *.bnr
