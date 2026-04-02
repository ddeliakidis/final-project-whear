#ifndef IPC_APP_H
#define IPC_APP_H

#include <zephyr/kernel.h>
#include "ipc_shared.h"

/* Initialize IPC consumer (registers ISR, enables interrupt) */
void ipc_app_init(void);

/* Block until a tag is available from the network core, or timeout.
 * Returns 0 on success, -1 on timeout. */
int ipc_app_wait_tag(ipc_tag_entry_t *out, k_timeout_t timeout);

#endif /* IPC_APP_H */
