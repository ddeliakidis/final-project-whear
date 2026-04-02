#ifndef IPC_NET_H
#define IPC_NET_H

#include "yrm100.h"

/* Initialize the IPC mailbox in shared memory */
void ipc_net_init(void);

/* Write a tag to the shared ring buffer and signal the app core */
void ipc_net_send_tag(const yrm100_tag_t *tag);

#endif /* IPC_NET_H */
