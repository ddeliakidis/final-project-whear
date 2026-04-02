/*
 * IPC consumer — application core side (Zephyr).
 *
 * Receives RFID tag data from the network core via shared memory,
 * signaled by the nRF5340 IPC hardware peripheral.
 */

#include "ipc_app.h"
#include <zephyr/irq.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ipc_app, LOG_LEVEL_INF);

/* ── nRF5340 IPC peripheral (app core, secure domain) ────────────────── */

#define NRF_IPC_APP_BASE  0x5002A000UL

#define IPC_EVENTS_RECEIVE(ch) \
    (*(volatile uint32_t *)(NRF_IPC_APP_BASE + 0x100 + (ch) * 4))
#define IPC_INTENSET \
    (*(volatile uint32_t *)(NRF_IPC_APP_BASE + 0x304))
#define IPC_RECEIVE_CNF(ch) \
    (*(volatile uint32_t *)(NRF_IPC_APP_BASE + 0x590 + (ch) * 4))

/* App core IPC IRQ number (nRF5340 PS table 4.2.4) */
#define APP_IPC_IRQn  42

static K_SEM_DEFINE(ipc_tag_sem, 0, 1);

static uint32_t local_read_idx;

/* ── IPC ISR ──────────────────────────────────────────────────────────── */

static void ipc_isr(const void *arg)
{
    ARG_UNUSED(arg);

    if (IPC_EVENTS_RECEIVE(IPC_CH_NET_TO_APP)) {
        IPC_EVENTS_RECEIVE(IPC_CH_NET_TO_APP) = 0;
        k_sem_give(&ipc_tag_sem);
    }
}

/* ── Public API ───────────────────────────────────────────────────────── */

void ipc_app_init(void)
{
    local_read_idx = 0;

    /* Map IPC channel 0 receive event to generate interrupt */
    IPC_RECEIVE_CNF(IPC_CH_NET_TO_APP) = (1UL << IPC_CH_NET_TO_APP);

    /* Enable interrupt for channel 0 */
    IPC_INTENSET = (1UL << IPC_CH_NET_TO_APP);

    /* Register ISR with Zephyr */
    IRQ_CONNECT(APP_IPC_IRQn, 2, ipc_isr, NULL, 0);
    irq_enable(APP_IPC_IRQn);

    LOG_INF("IPC consumer initialized (IRQ %d)", APP_IPC_IRQn);
}

int ipc_app_wait_tag(ipc_tag_entry_t *out, k_timeout_t timeout)
{
    ipc_mailbox_t *mb = IPC_MAILBOX;

    /* If there are already unread tags, return immediately */
    __asm volatile ("dmb" ::: "memory");
    if (local_read_idx != mb->write_idx) {
        goto read_tag;
    }

    /* Otherwise wait for IPC signal */
    if (k_sem_take(&ipc_tag_sem, timeout) != 0) {
        return -1;   /* timeout */
    }

read_tag:
    __asm volatile ("dmb" ::: "memory");
    if (local_read_idx == mb->write_idx) {
        return -1;   /* spurious wakeup */
    }

    uint32_t idx = local_read_idx % IPC_RING_SIZE;
    *out = mb->tags[idx];

    __asm volatile ("dmb" ::: "memory");
    local_read_idx++;
    mb->read_idx = local_read_idx;

    return 0;
}
