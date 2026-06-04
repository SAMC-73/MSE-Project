include sources.mk

EXEC = app.elf

LINKER_FILE = stm32f4.ld
CPU = cortex-m4
ARCH = armv7e-m
SPECS = nosys.specs
FPU = fpv4-sp-d16

ARCHFLAGS = -mcpu=$(CPU) -mthumb -march=$(ARCH) -mfloat-abi=hard -mfpu=$(FPU) --specs=$(SPECS)

OBJS := $(SRCS:.c=.o)

CC = arm-none-eabi-gcc

CFLAGS = \
-g \
-O0 \
-std=c99 \
-Werror \
-Wall \
-D__FPU_PRESENT=1 \
-D__FPU_USED=1 \
$(ARCHFLAGS)

LDFLAGS = -nostdlib -T $(LINKER_FILE)

%.o : %.c
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -o $@

.PHONY : build 
build : $(EXEC)

$(EXEC) : $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -lgcc -o $@

.PHONY : flash
flash : 
	openocd -f board/st_nucleo_f4.cfg -c "program $(EXEC) verify reset" -c shutdown

.PHONY : clean
clean : 
	rm -f $(OBJS) $(EXEC)