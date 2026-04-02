#include "yrm100.h"
#include <string.h>

/* ── Internal helpers ─────────────────────────────────────────────── */

uint8_t
yrm100_checksum(const uint8_t *data, uint16_t len)
{
    uint16_t sum = 0;
    for (uint16_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return (uint8_t) (sum & 0xFF);
}

static uint16_t
build_frame(uint8_t *buf, uint8_t command, const uint8_t *params, uint16_t param_len)
{
    uint16_t idx = 0;

    buf[idx++] = YRM100_HEADER;
    buf[idx++] = YRM100_TYPE_CMD;
    buf[idx++] = command;
    buf[idx++] = (uint8_t) (param_len >> 8);
    buf[idx++] = (uint8_t) (param_len & 0xFF);

    for (uint16_t i = 0; i < param_len; i++) {
        buf[idx++] = params[i];
    }

    /* checksum covers Type .. last param byte (indices 1 .. idx-1) */
    buf[idx] = yrm100_checksum(&buf[1], idx - 1);
    idx++;

    buf[idx++] = YRM100_END;

    return idx;
}

/* ── Init ─────────────────────────────────────────────────────────── */

void
yrm100_init(yrm100_t *dev, const yrm100_uart_t *uart, uint32_t baud)
{
    memset(dev, 0, sizeof(*dev));
    dev->uart = *uart;
    dev->uart.uart_init(baud);
}

/* ── Send / Receive ───────────────────────────────────────────────── */

yrm100_status_t
yrm100_send_cmd(yrm100_t *dev, uint8_t command, const uint8_t *params, uint16_t param_len)
{
    uint8_t frame[YRM100_MAX_FRAME_LEN];
    uint16_t len = build_frame(frame, command, params, param_len);
    dev->uart.uart_send(frame, len);
    return YRM100_OK;
}

yrm100_status_t
yrm100_recv_response(yrm100_t *dev, yrm100_response_t *resp, uint16_t timeout_ms)
{
    memset(resp, 0, sizeof(*resp));

    /* wait for header byte */
    uint8_t b = dev->uart.uart_recv_byte(timeout_ms);
    if (b != YRM100_HEADER) {
        return YRM100_ERR_TIMEOUT;
    }

    /* type */
    resp->type = dev->uart.uart_recv_byte(timeout_ms);

    /* command */
    resp->command = dev->uart.uart_recv_byte(timeout_ms);

    /* param length (2 bytes big-endian) */
    uint8_t pl_msb = dev->uart.uart_recv_byte(timeout_ms);
    uint8_t pl_lsb = dev->uart.uart_recv_byte(timeout_ms);
    resp->param_len = ((uint16_t) pl_msb << 8) | pl_lsb;

    if (resp->param_len > sizeof(resp->params)) {
        return YRM100_ERR_BAD_FRAME;
    }

    /* read parameter bytes */
    for (uint16_t i = 0; i < resp->param_len; i++) {
        resp->params[i] = dev->uart.uart_recv_byte(timeout_ms);
    }

    /* checksum */
    uint8_t recv_cksum = dev->uart.uart_recv_byte(timeout_ms);

    /* end byte */
    uint8_t end = dev->uart.uart_recv_byte(timeout_ms);
    if (end != YRM100_END) {
        return YRM100_ERR_BAD_FRAME;
    }

    /* verify checksum: covers type + command + pl_msb + pl_lsb + params */
    uint8_t check_buf[4 + YRM100_MAX_FRAME_LEN];
    check_buf[0] = resp->type;
    check_buf[1] = resp->command;
    check_buf[2] = pl_msb;
    check_buf[3] = pl_lsb;
    memcpy(&check_buf[4], resp->params, resp->param_len);
    uint8_t calc_cksum = yrm100_checksum(check_buf, 4 + resp->param_len);

    if (calc_cksum != recv_cksum) {
        return YRM100_ERR_CHECKSUM;
    }

    /* check for error response */
    if (resp->command == YRM100_CMD_ERROR && resp->param_len >= 1) {
        resp->error_code = resp->params[0];
        return YRM100_ERR_MODULE;
    }

    return YRM100_OK;
}

/* helper: send command and wait for one response */
static yrm100_status_t
send_and_recv(yrm100_t *dev, uint8_t command, const uint8_t *params,
              uint16_t param_len, yrm100_response_t *resp, uint16_t timeout_ms)
{
    yrm100_status_t s = yrm100_send_cmd(dev, command, params, param_len);
    if (s != YRM100_OK) return s;
    return yrm100_recv_response(dev, resp, timeout_ms);
}

/* ── Module information ───────────────────────────────────────────── */

static yrm100_status_t
get_info(yrm100_t *dev, uint8_t info_type, char *buf, uint8_t buf_len)
{
    yrm100_response_t resp;
    yrm100_status_t s = send_and_recv(dev, YRM100_CMD_GET_INFO,
                                      &info_type, 1, &resp, 500);
    if (s != YRM100_OK) return s;

    /* first param byte is info type echo, rest is ASCII string */
    uint16_t str_len = resp.param_len > 1 ? resp.param_len - 1 : 0;
    if (str_len >= buf_len) str_len = buf_len - 1;
    memcpy(buf, &resp.params[1], str_len);
    buf[str_len] = '\0';
    return YRM100_OK;
}

yrm100_status_t
yrm100_get_hw_version(yrm100_t *dev, char *buf, uint8_t buf_len)
{
    return get_info(dev, YRM100_INFO_HW_VERSION, buf, buf_len);
}

yrm100_status_t
yrm100_get_sw_version(yrm100_t *dev, char *buf, uint8_t buf_len)
{
    return get_info(dev, YRM100_INFO_SW_VERSION, buf, buf_len);
}

yrm100_status_t
yrm100_get_manufacturer(yrm100_t *dev, char *buf, uint8_t buf_len)
{
    return get_info(dev, YRM100_INFO_MANUFACTURER, buf, buf_len);
}

/* ── Inventory ────────────────────────────────────────────────────── */

static yrm100_status_t
parse_inventory_notice(const yrm100_response_t *resp, yrm100_tag_t *tag)
{
    if (resp->type != YRM100_TYPE_NOTICE || resp->command != YRM100_CMD_SINGLE_INVENTORY) {
        return YRM100_ERR_BAD_FRAME;
    }
    if (resp->param_len < 5) return YRM100_ERR_BAD_FRAME;

    /* RSSI (signed byte, twos complement) */
    tag->rssi = (int8_t) resp->params[0];

    /* PC (2 bytes) */
    tag->pc[0] = resp->params[1];
    tag->pc[1] = resp->params[2];

    /* EPC length is total param_len - 1(rssi) - 2(pc) - 2(crc) */
    uint16_t epc_len = resp->param_len - 5;
    if (epc_len > sizeof(tag->epc)) epc_len = sizeof(tag->epc);
    tag->epc_len = (uint8_t) epc_len;
    memcpy(tag->epc, &resp->params[3], epc_len);

    /* CRC (last 2 param bytes) */
    tag->crc[0] = resp->params[resp->param_len - 2];
    tag->crc[1] = resp->params[resp->param_len - 1];

    return YRM100_OK;
}

yrm100_status_t
yrm100_single_inventory(yrm100_t *dev, yrm100_tag_t *tag)
{
    yrm100_response_t resp;
    yrm100_status_t s = send_and_recv(dev, YRM100_CMD_SINGLE_INVENTORY,
                                      NULL, 0, &resp, 1000);
    if (s != YRM100_OK) return s;

    return parse_inventory_notice(&resp, tag);
}

yrm100_status_t
yrm100_start_multi_inventory(yrm100_t *dev, uint16_t count)
{
    uint8_t params[3];
    params[0] = 0x22;   /* reserved byte per protocol */
    params[1] = (uint8_t) (count >> 8);
    params[2] = (uint8_t) (count & 0xFF);

    return yrm100_send_cmd(dev, YRM100_CMD_MULTI_INVENTORY, params, 3);
}

yrm100_status_t
yrm100_stop_inventory(yrm100_t *dev)
{
    yrm100_response_t resp;
    return send_and_recv(dev, YRM100_CMD_STOP_INVENTORY, NULL, 0, &resp, 500);
}

yrm100_status_t
yrm100_poll_inventory(yrm100_t *dev, yrm100_tag_t *tag)
{
    yrm100_response_t resp;
    yrm100_status_t s = yrm100_recv_response(dev, &resp, 100);
    if (s != YRM100_OK) return s;

    /* might be an error frame (no tag read) -- skip it */
    if (resp.command == YRM100_CMD_ERROR) {
        return YRM100_ERR_MODULE;
    }

    return parse_inventory_notice(&resp, tag);
}

/* ── Select ───────────────────────────────────────────────────────── */

yrm100_status_t
yrm100_set_select(yrm100_t *dev, uint8_t sel_param,
                  uint32_t ptr, uint8_t mask_len,
                  uint8_t truncate,
                  const uint8_t *mask, uint8_t mask_bytes)
{
    uint8_t params[7 + 62];   /* max mask size */
    uint16_t idx = 0;

    params[idx++] = sel_param;
    params[idx++] = (uint8_t) (ptr >> 24);
    params[idx++] = (uint8_t) (ptr >> 16);
    params[idx++] = (uint8_t) (ptr >> 8);
    params[idx++] = (uint8_t) (ptr & 0xFF);
    params[idx++] = mask_len;
    params[idx++] = truncate;

    for (uint8_t i = 0; i < mask_bytes; i++) {
        params[idx++] = mask[i];
    }

    yrm100_response_t resp;
    return send_and_recv(dev, YRM100_CMD_SET_SELECT, params, idx, &resp, 500);
}

yrm100_status_t
yrm100_set_select_mode(yrm100_t *dev, uint8_t mode)
{
    yrm100_response_t resp;
    return send_and_recv(dev, YRM100_CMD_SET_SELECT_MODE, &mode, 1, &resp, 500);
}

/* ── Read tag memory ──────────────────────────────────────────────── */

yrm100_status_t
yrm100_read_tag(yrm100_t *dev, uint32_t access_password,
                uint8_t membank, uint16_t addr,
                uint16_t word_count,
                uint8_t *data_out, uint16_t *data_len)
{
    uint8_t params[9];
    params[0] = (uint8_t) (access_password >> 24);
    params[1] = (uint8_t) (access_password >> 16);
    params[2] = (uint8_t) (access_password >> 8);
    params[3] = (uint8_t) (access_password & 0xFF);
    params[4] = membank;
    params[5] = (uint8_t) (addr >> 8);
    params[6] = (uint8_t) (addr & 0xFF);
    params[7] = (uint8_t) (word_count >> 8);
    params[8] = (uint8_t) (word_count & 0xFF);

    yrm100_response_t resp;
    yrm100_status_t s = send_and_recv(dev, YRM100_CMD_READ, params, 9, &resp, 2000);
    if (s != YRM100_OK) return s;

    /*
     * Response params layout:
     *   [0]          = UL (PC+EPC length)
     *   [1..UL]      = PC + EPC
     *   [UL+1..end]  = read data
     */
    if (resp.param_len < 1) return YRM100_ERR_BAD_FRAME;
    uint8_t ul = resp.params[0];

    uint16_t data_start = 1 + ul;
    if (data_start > resp.param_len) return YRM100_ERR_BAD_FRAME;

    uint16_t len = resp.param_len - data_start;
    if (data_out) memcpy(data_out, &resp.params[data_start], len);
    if (data_len) *data_len = len;

    return YRM100_OK;
}

/* ── Write tag memory ─────────────────────────────────────────────── */

yrm100_status_t
yrm100_write_tag(yrm100_t *dev, uint32_t access_password,
                 uint8_t membank, uint16_t addr,
                 uint16_t word_count,
                 const uint8_t *data)
{
    uint8_t params[9 + 64];   /* 9 header + up to 32 words = 64 bytes */
    uint16_t idx = 0;

    params[idx++] = (uint8_t) (access_password >> 24);
    params[idx++] = (uint8_t) (access_password >> 16);
    params[idx++] = (uint8_t) (access_password >> 8);
    params[idx++] = (uint8_t) (access_password & 0xFF);
    params[idx++] = membank;
    params[idx++] = (uint8_t) (addr >> 8);
    params[idx++] = (uint8_t) (addr & 0xFF);
    params[idx++] = (uint8_t) (word_count >> 8);
    params[idx++] = (uint8_t) (word_count & 0xFF);

    uint16_t byte_count = word_count * 2;
    memcpy(&params[idx], data, byte_count);
    idx += byte_count;

    yrm100_response_t resp;
    return send_and_recv(dev, YRM100_CMD_WRITE, params, idx, &resp, 2000);
}

/* ── Lock ─────────────────────────────────────────────────────────── */

yrm100_status_t
yrm100_lock_tag(yrm100_t *dev, uint32_t access_password,
                uint8_t ld_msb, uint8_t ld_mid, uint8_t ld_lsb)
{
    uint8_t params[7];
    params[0] = (uint8_t) (access_password >> 24);
    params[1] = (uint8_t) (access_password >> 16);
    params[2] = (uint8_t) (access_password >> 8);
    params[3] = (uint8_t) (access_password & 0xFF);
    params[4] = ld_msb;
    params[5] = ld_mid;
    params[6] = ld_lsb;

    yrm100_response_t resp;
    return send_and_recv(dev, YRM100_CMD_LOCK, params, 7, &resp, 2000);
}

/* ── Kill ─────────────────────────────────────────────────────────── */

yrm100_status_t
yrm100_kill_tag(yrm100_t *dev, uint32_t kill_password)
{
    uint8_t params[4];
    params[0] = (uint8_t) (kill_password >> 24);
    params[1] = (uint8_t) (kill_password >> 16);
    params[2] = (uint8_t) (kill_password >> 8);
    params[3] = (uint8_t) (kill_password & 0xFF);

    yrm100_response_t resp;
    return send_and_recv(dev, YRM100_CMD_KILL, params, 4, &resp, 2000);
}

/* ── Baud rate ────────────────────────────────────────────────────── */

yrm100_status_t
yrm100_set_baud(yrm100_t *dev, uint16_t baud_param)
{
    uint8_t params[2];
    params[0] = (uint8_t) (baud_param >> 8);
    params[1] = (uint8_t) (baud_param & 0xFF);

    yrm100_response_t resp;
    return send_and_recv(dev, YRM100_CMD_SET_BAUD, params, 2, &resp, 500);
}

/* ── Region ───────────────────────────────────────────────────────── */

yrm100_status_t
yrm100_set_region(yrm100_t *dev, uint8_t region)
{
    yrm100_response_t resp;
    return send_and_recv(dev, YRM100_CMD_SET_REGION, &region, 1, &resp, 500);
}

yrm100_status_t
yrm100_get_region(yrm100_t *dev, uint8_t *region)
{
    yrm100_response_t resp;
    yrm100_status_t s = send_and_recv(dev, YRM100_CMD_GET_REGION, NULL, 0, &resp, 500);
    if (s != YRM100_OK) return s;
    if (resp.param_len >= 1) *region = resp.params[0];
    return YRM100_OK;
}

/* ── Channel ──────────────────────────────────────────────────────── */

yrm100_status_t
yrm100_set_channel(yrm100_t *dev, uint8_t channel)
{
    yrm100_response_t resp;
    return send_and_recv(dev, YRM100_CMD_SET_CHANNEL, &channel, 1, &resp, 500);
}

yrm100_status_t
yrm100_get_channel(yrm100_t *dev, uint8_t *channel)
{
    yrm100_response_t resp;
    yrm100_status_t s = send_and_recv(dev, YRM100_CMD_GET_CHANNEL, NULL, 0, &resp, 500);
    if (s != YRM100_OK) return s;
    if (resp.param_len >= 1) *channel = resp.params[0];
    return YRM100_OK;
}

/* ── Frequency hopping ────────────────────────────────────────────── */

yrm100_status_t
yrm100_set_freq_hop(yrm100_t *dev)
{
    uint8_t param = 0xFF;   /* enable auto frequency hopping */
    yrm100_response_t resp;
    return send_and_recv(dev, YRM100_CMD_SET_FREQ_HOP, &param, 1, &resp, 500);
}

/* ── TX power ─────────────────────────────────────────────────────── */

yrm100_status_t
yrm100_set_tx_power(yrm100_t *dev, uint16_t power)
{
    uint8_t params[2];
    params[0] = (uint8_t) (power >> 8);
    params[1] = (uint8_t) (power & 0xFF);

    yrm100_response_t resp;
    return send_and_recv(dev, YRM100_CMD_SET_TX_POWER, params, 2, &resp, 500);
}

yrm100_status_t
yrm100_get_tx_power(yrm100_t *dev, uint16_t *power)
{
    yrm100_response_t resp;
    yrm100_status_t s = send_and_recv(dev, YRM100_CMD_GET_TX_POWER, NULL, 0, &resp, 500);
    if (s != YRM100_OK) return s;
    if (resp.param_len >= 2) {
        *power = ((uint16_t) resp.params[0] << 8) | resp.params[1];
    }
    return YRM100_OK;
}

/* ── Power management ─────────────────────────────────────────────── */

yrm100_status_t
yrm100_sleep(yrm100_t *dev)
{
    yrm100_response_t resp;
    return send_and_recv(dev, YRM100_CMD_SLEEP, NULL, 0, &resp, 500);
}

yrm100_status_t
yrm100_set_idle_time(yrm100_t *dev, uint8_t minutes)
{
    yrm100_response_t resp;
    return send_and_recv(dev, YRM100_CMD_SET_IDLE_TIME, &minutes, 1, &resp, 500);
}
