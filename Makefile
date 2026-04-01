# ── nRF5340 baremetal build (application core, Cortex-M33) ───────────
#
# Requirements:
#   arm-none-eabi-gcc   (brew install arm-none-eabi-gcc  /  ARM website)
#   nrfjprog            (Nordic nRF Command Line Tools)

TARGET = firmware

CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

CFLAGS  = -mcpu=cortex-m33 -mthumb \
          -mfloat-abi=hard -mfpu=fpv5-sp-d16 \
          -Os -Wall -Wextra \
          -ffunction-sections -fdata-sections \
          -I. -Ilib/yrm100

LDFLAGS = -mcpu=cortex-m33 -mthumb \
          -mfloat-abi=hard -mfpu=fpv5-sp-d16 \
          -T nrf5340_app.ld -nostartfiles \
          --specs=nano.specs --specs=nosys.specs \
          -Wl,--gc-sections \
          -lc -lnosys

SOURCES = main.c \
          startup_nrf5340.c \
          lib/yrm100/yrm100.c \
          lib/yrm100/yrm100_uart_nrf5340.c

OBJECTS = $(SOURCES:.c=.o)

# ── Build targets ────────────────────────────────────────────────────

all: $(TARGET).hex $(TARGET).bin
	$(SIZE) $(TARGET).elf

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# ── Flash via J-Link (nRF7002DK on-board debugger) ───────────────────

flash: $(TARGET).hex
	nrfjprog --program $(TARGET).hex -f nrf53 \
	         --coprocessor CP_APPLICATION \
	         --sectorerase --verify --reset

erase:
	nrfjprog -f nrf53 --coprocessor CP_APPLICATION --eraseall

recover:
	nrfjprog --recover -f nrf53 --coprocessor CP_APPLICATION

# ── Housekeeping ─────────────────────────────────────────────────────

clean:
	rm -f $(OBJECTS) $(TARGET).elf $(TARGET).hex $(TARGET).bin

.PHONY: all flash erase recover clean
