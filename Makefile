# ── Whear — nRF5340 dual-core build system ───────────────────────────
#
# Targets:
#   make            Build both cores (net core + Zephyr app core)
#   make net        Build network core only (baremetal RFID)
#   make app        Build Zephyr app core only (WiFi + WebSocket)
#   make flash      Flash both cores (net core first, then app core)
#   make flash-net  Flash network core only
#   make flash-app  Flash Zephyr app core only
#   make erase      Erase both cores
#   make recover    Full recovery of both cores
#   make clean      Clean all build artifacts
#   make monitor    Open serial monitor (115200 baud)
#
# Requirements:
#   arm-none-eabi-gcc, nrfjprog, west
#   NCS v3.0.2 at /opt/nordic/ncs/v3.0.2

# ── NCS / Zephyr environment ────────────────────────────────────────

export ZEPHYR_BASE              := /opt/nordic/ncs/v3.0.2/zephyr
export ZEPHYR_TOOLCHAIN_VARIANT := zephyr
export ZEPHYR_SDK_INSTALL_DIR   := /opt/nordic/ncs/toolchains/ef4fc6722e/opt/zephyr-sdk
NCS_BIN := /opt/nordic/ncs/toolchains/ef4fc6722e/bin
WEST    := $(NCS_BIN)/west

BOARD   := nrf7002dk/nrf5340/cpuapp

# ── Paths ────────────────────────────────────────────────────────────

NET_DIR := net_core
APP_DIR := app_core

NET_HEX := $(NET_DIR)/net_firmware.hex
APP_HEX := $(APP_DIR)/build/app_core/zephyr/zephyr.hex

# ── Baremetal network core ─────────────────────────────────────────

NET_CC      := arm-none-eabi-gcc
NET_OBJCOPY := arm-none-eabi-objcopy
NET_SIZE    := arm-none-eabi-size

NET_CFLAGS  := -mcpu=cortex-m33+nodsp -mthumb \
               -mfloat-abi=soft \
               -Os -Wall -Wextra \
               -ffunction-sections -fdata-sections \
               -I$(NET_DIR) -I$(NET_DIR)/lib/yrm100 -Ishared

NET_LDFLAGS := -mcpu=cortex-m33+nodsp -mthumb \
               -mfloat-abi=soft \
               -T $(NET_DIR)/nrf5340_net.ld -nostartfiles \
               --specs=nano.specs --specs=nosys.specs \
               -Wl,--gc-sections -lc -lnosys

NET_SRCS := $(NET_DIR)/main.c \
            $(NET_DIR)/startup_nrf5340_net.c \
            $(NET_DIR)/ipc_net.c \
            $(NET_DIR)/lib/yrm100/yrm100.c \
            $(NET_DIR)/lib/yrm100/yrm100_uart_nrf5340_net.c

NET_OBJS := $(NET_SRCS:.c=.o)

# ══════════════════════════════════════════════════════════════════════
# Top-level targets
# ══════════════════════════════════════════════════════════════════════

.PHONY: all net app flash flash-net flash-app \
        erase erase-net erase-app recover \
        clean clean-net clean-app monitor

all: net app

# ── Build ────────────────────────────────────────────────────────────

net: $(NET_HEX)
	$(NET_SIZE) $(NET_DIR)/net_firmware.elf

app:
	PATH="$(NCS_BIN):$$PATH" $(WEST) build \
		-b $(BOARD) \
		--build-dir $(APP_DIR)/build \
		-d $(APP_DIR)/build \
		$(APP_DIR) \
		-- -DNCS_TOOLCHAIN_VERSION=NONE

# ── Flash ────────────────────────────────────────────────────────────

flash: net app
	nrfjprog -f nrf53 --coprocessor CP_NETWORK --eraseall
	nrfjprog -f nrf53 --coprocessor CP_APPLICATION --eraseall
	nrfjprog --program $(NET_HEX) -f nrf53 \
	         --coprocessor CP_NETWORK --sectorerase --verify
	nrfjprog --program $(APP_HEX) -f nrf53 \
	         --coprocessor CP_APPLICATION --sectorerase --verify
	nrfjprog --reset -f nrf53
	@echo "=== Both cores flashed ==="

flash-net: net
	nrfjprog --program $(NET_HEX) -f nrf53 \
	         --coprocessor CP_NETWORK --sectorerase --verify --reset

flash-app: app
	nrfjprog --program $(APP_HEX) -f nrf53 \
	         --coprocessor CP_APPLICATION --sectorerase --verify --reset

# ── Erase / recover ─────────────────────────────────────────────────

erase:
	nrfjprog -f nrf53 --coprocessor CP_NETWORK --eraseall
	nrfjprog -f nrf53 --coprocessor CP_APPLICATION --eraseall

erase-net:
	nrfjprog -f nrf53 --coprocessor CP_NETWORK --eraseall

erase-app:
	nrfjprog -f nrf53 --coprocessor CP_APPLICATION --eraseall

recover:
	nrfjprog --recover -f nrf53 --coprocessor CP_NETWORK
	nrfjprog --recover -f nrf53 --coprocessor CP_APPLICATION

# ── Serial monitor ───────────────────────────────────────────────────

monitor:
	@echo "Opening serial monitor (Ctrl+Q to exit)..."
	@tio $$(ls /dev/tty.usbmodem* 2>/dev/null | head -1) -b 115200 \
	 || echo "No USB serial device found. Check connection."

# ── Clean ────────────────────────────────────────────────────────────

clean: clean-net clean-app

clean-net:
	rm -f $(NET_OBJS) $(NET_DIR)/net_firmware.elf \
	      $(NET_DIR)/net_firmware.hex $(NET_DIR)/net_firmware.bin

clean-app:
	rm -rf $(APP_DIR)/build

# ══════════════════════════════════════════════════════════════════════
# Network core build rules
# ══════════════════════════════════════════════════════════════════════

$(NET_DIR)/%.o: $(NET_DIR)/%.c
	$(NET_CC) $(NET_CFLAGS) -c -o $@ $<

$(NET_DIR)/lib/yrm100/%.o: $(NET_DIR)/lib/yrm100/%.c
	$(NET_CC) $(NET_CFLAGS) -c -o $@ $<

$(NET_DIR)/net_firmware.elf: $(NET_OBJS)
	$(NET_CC) $(NET_LDFLAGS) -o $@ $^

$(NET_DIR)/net_firmware.hex: $(NET_DIR)/net_firmware.elf
	$(NET_OBJCOPY) -O ihex $< $@

$(NET_DIR)/net_firmware.bin: $(NET_DIR)/net_firmware.elf
	$(NET_OBJCOPY) -O binary $< $@
