/*
 * IPC producer — network core side.
 *
 * Writes RFID tag data into a shared-memory ring buffer and signals
 * the application core via the nRF5340 IPC hardware peripheral.
 */

#include "ipc_net.h"
#include "nrf5340_net.h"
#include "ipc_shared.h"
#include <string.h>

/* IPC_Handler — placeholder for future app→net commands */
void IPC_Handler(void)
{
    if (NRF_IPC->EVENTS_RECEIVE[IPC_CH_APP_TO_NET]) {
        NRF_IPC->EVENTS_RECEIVE[IPC_CH_APP_TO_NET] = 0;
        /* TODO: handle commands from app core */
    }
}

void ipc_net_init(void)
{
    ipc_mailbox_t *mb = IPC_MAILBOX;
    mb->magic      = IPC_MAILBOX_MAGIC;
    mb->write_idx  = 0;
    mb->read_idx   = 0;
    mb->net_status = 0;
    mb->total_tags = 0;

    /* Configure SEND_CNF[0]: task SEND[0] triggers channel 0 event on app core */
    NRF_IPC->SEND_CNF[IPC_CH_NET_TO_APP] = (1UL << IPC_CH_NET_TO_APP);

    /* Optionally enable receive interrupt for app→net channel (future use) */
    NRF_IPC->RECEIVE_CNF[IPC_CH_APP_TO_NET] = (1UL << IPC_CH_APP_TO_NET);
    NRF_IPC->INTENSET = (1UL << IPC_CH_APP_TO_NET);
    NVIC_EnableIRQ(IPC_IRQn);
}

void ipc_net_send_tag(const yrm100_tag_t *tag)
{
    ipc_mailbox_t *mb = IPC_MAILBOX;

    /* Check if ring buffer is full */
    if ((mb->write_idx - mb->read_idx) >= IPC_RING_SIZE) {
        return;   /* buffer full — drop tag */
    }

    /* Write tag into next slot */
    uint32_t idx = mb->write_idx % IPC_RING_SIZE;
    ipc_tag_entry_t *entry = &mb->tags[idx];

    entry->rssi    = tag->rssi;
    entry->epc_len = (tag->epc_len > IPC_MAX_EPC_LEN)
                         ? IPC_MAX_EPC_LEN : tag->epc_len;
    memcpy(entry->epc, tag->epc, entry->epc_len);
    entry->pc[0]   = tag->pc[0];
    entry->pc[1]   = tag->pc[1];
    entry->crc[0]  = tag->crc[0];
    entry->crc[1]  = tag->crc[1];

    /* Ensure tag data is visible before index update */
    __DMB();

    mb->write_idx++;
    mb->total_tags++;

    /* Signal app core */
    NRF_IPC->TASKS_SEND[IPC_CH_NET_TO_APP] = 1;
}
