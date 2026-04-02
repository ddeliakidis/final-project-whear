#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

/* Initialize WiFi subsystem and connect using static credentials.
 * Blocks until connected and IP address is obtained, or timeout.
 * Returns 0 on success, negative on failure. */
int wifi_connect(void);

/* Returns true if WiFi is connected and has an IP address. */
int wifi_is_connected(void);

#endif /* WIFI_MANAGER_H */
