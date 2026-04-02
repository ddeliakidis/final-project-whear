#ifndef IPC_SHARED_H
#define IPC_SHARED_H

/*
 * Shared memory definitions for nRF5340 inter-core communication.
 *
 * Producer: network core (baremetal) — writes RFID tag data.
 * Consumer: application core (Zephyr) — reads and sends over WebSocket.
 *
 * Located in the last 1 KB of network core RAM (0x2100FC00–0x2100FFFF).
 * Both cores access it at the same physical address.
 */

#include <stdint.h>
#include <string.h>

/* ── Shared memory address ──────────────────────────────────────────── */

#define IPC_SHARED_MEM_ADDR  0x2100FC00UL
#define IPC_SHARED_MEM_SIZE  1024

/* ── IPC channel assignments (nRF5340 IPC peripheral has 16 channels) ─ */

#define IPC_CH_NET_TO_APP    0   /* net core signals "new tag data ready" */
#define IPC_CH_APP_TO_NET    1   /* app core signals (future use) */

/* ── Mailbox magic value ────────────────────────────────────────────── */

#define IPC_MAILBOX_MAGIC    0x57484552UL   /* "WHER" in ASCII */

/* ── Tag entry in shared memory ─────────────────────────────────────── */

#define IPC_MAX_EPC_LEN  12
#define IPC_RING_SIZE     8     /* number of tag slots (power of 2) */

typedef struct __attribute__((packed)) {
    int8_t   rssi;
    uint8_t  epc_len;
    uint8_t  epc[IPC_MAX_EPC_LEN];
    uint8_t  pc[2];
    uint8_t  crc[2];
    uint8_t  _pad;               /* pad to 20 bytes */
} ipc_tag_entry_t;               /* 20 bytes */

/* ── Shared mailbox structure ───────────────────────────────────────── */

typedef struct __attribute__((packed)) {
    volatile uint32_t magic;           /* IPC_MAILBOX_MAGIC when initialized */
    volatile uint32_t write_idx;       /* incremented by net core (producer) */
    volatile uint32_t read_idx;        /* incremented by app core (consumer) */
    volatile uint32_t net_status;      /* 0 = idle, 1 = scanning, 2 = error */
    volatile uint32_t total_tags;      /* running count from net core */
    uint32_t          _reserved[3];    /* pad header to 32 bytes */
    ipc_tag_entry_t   tags[IPC_RING_SIZE];  /* 8 * 20 = 160 bytes */
} ipc_mailbox_t;                       /* 32 + 160 = 192 bytes total */

/* ── Accessor (same address from both cores) ────────────────────────── */

#define IPC_MAILBOX  ((ipc_mailbox_t *)IPC_SHARED_MEM_ADDR)

#endif /* IPC_SHARED_H */
