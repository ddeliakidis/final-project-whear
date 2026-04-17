---
marp: true
theme: default
paginate: true
---

# Whear
### Smart Closet Inventory — Final Demo

**Team 4** — Jefferson Ding · Dimitris Deliakidis · Carly Googel

ESE 3500

---

## System Block Diagram

```
 Tagged garments ──UHF──►  6 dBi patch antenna ──►  YRM100 (Impinj R2000)
                                                            │ USART1 @ 115200
                                                            ▼
                                   ┌─────────────────────────────────────┐
                                   │ STM32F411RE (Nucleo, bare-metal C)  │
                                   │  YRM100 driver · TTL tag table      │
                                   └──────────────────┬──────────────────┘
                                                      │ USART6 framer + READY pin
                                                      ▼
                                   ┌─────────────────────────────────────┐
                                   │ ESP32 Feather HUZZAH32 V2           │
                                   │  UART reader · Firestore reconciler │
                                   └──────────────────┬──────────────────┘
                                                      │ HTTPS
                                                      ▼
                               Google Firestore  →  SwiftUI iOS app
```

---

## Hardware Implementation

- **RFID front end:** YRM100 UHF module + 6 dBi SMA patch antenna, US region, 26 dBm TX
- **MCU:** STM32F411RE (Nucleo-64) — 3 USARTs in use
  - USART1 PA9/PA10 → YRM100
  - USART2 PA2/PA3 → ST-Link VCOM debug
  - USART6 PC6/PC7 → ESP32 bridge
  - PA8 → READY input from ESP32
- **Wi-Fi bridge:** Adafruit Feather HUZZAH32 V2 (ESP32), UART2 on GPIO14/32, READY on GPIO27
- **Power:** 5 V USB per board, on-board 3.3 V LDOs
- **Case:** 3D-printed closet-rail mount with integrated hook + external SMA antenna

---

# Thanks!

Questions?

**Repo:** github.com/upenn-embedded/final-project-whear
**iOS:** github.com/carlygoogel/Whear
