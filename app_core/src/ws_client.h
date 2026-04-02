#ifndef WS_CLIENT_H
#define WS_CLIENT_H

#include "ipc_shared.h"

/* Connect to the WebSocket server.
 * Returns 0 on success, negative on failure. */
int ws_connect(const char *server_addr, uint16_t port, const char *url_path);

/* Send an RFID tag as JSON over the WebSocket.
 * Returns 0 on success, negative on failure. */
int ws_send_tag(const ipc_tag_entry_t *tag);

/* Close the WebSocket connection. */
void ws_disconnect(void);

/* Returns true if WebSocket is connected. */
int ws_is_connected(void);

#endif /* WS_CLIENT_H */
