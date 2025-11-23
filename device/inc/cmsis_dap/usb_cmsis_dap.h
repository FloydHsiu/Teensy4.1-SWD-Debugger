#include <stdint.h>

void usb_cmsis_dap_configure(void);
int usb_cmsis_dap_recv(void *buffer, uint32_t timeout);
int usb_cmsis_dap_send(const void *buffer, uint32_t len, uint32_t timeout);
int usb_cmsis_dap_rx_available(void);