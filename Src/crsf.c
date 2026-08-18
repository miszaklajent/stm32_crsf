#include "crsf.h"

#define RX_BUF_SIZE 128
uint8_t rx_buffer[RX_BUF_SIZE];
uint8_t tx_buffer[64];

UART_HandleTypeDef *crsf_uart_handle = NULL;

static uint8_t crc8_table[256] = {0};

void generate_CRC(uint8_t poly) {
    for (int idx = 0; idx < 256; ++idx)
    {
        uint8_t crc = idx;
        for (int shift = 0; shift < 8; ++shift)
        {
            crc = (crc << 1) ^ ((crc & 0x80) ? poly : 0);
        }
        crc8_table[idx] = crc & 0xff;
    }
}

uint8_t crc8(const uint8_t *data, uint8_t len) {
  uint8_t crc = 0;
  while (len--)
  {
    crc = crc8_table[crc ^ *data++];
  }

  return crc;
}

void crsf_init(UART_HandleTypeDef *huart) {
    crsf_uart_handle = huart;

    HAL_UARTEx_ReceiveToIdle_DMA(crsf_uart_handle, rx_buffer, RX_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(crsf_uart_handle->hdmarx, DMA_IT_HT);

    
    generate_CRC(0xD5);
}

void crsf_re_init() {
    #ifdef HALF_DUPLEX
    HAL_HalfDuplex_EnableReceiver(crsf_uart_handle);
    #endif
    HAL_UARTEx_ReceiveToIdle_DMA(crsf_uart_handle, rx_buffer, RX_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(crsf_uart_handle->hdmarx, DMA_IT_HT);
}

void process_crsf_packet(Packet_t *Packet) {
    uint8_t len = rx_buffer[1];
    uint8_t checksum = crc8(&rx_buffer[2], len - 1);

    if (checksum == rx_buffer[len + 1]) {
        Packet->sync = rx_buffer[0];
        Packet->length = rx_buffer[1];
        Packet->type = rx_buffer[2];
        Packet->payload_length = Packet->length - 2;
        memset(Packet->payload, 0, sizeof(Packet->payload));
        for (int i = 0; i < len - 2; i++)
        {
          Packet->payload[i] = rx_buffer[i + 3];
        }
    }
    memset((void*)rx_buffer, 0, RX_BUF_SIZE);
    crsf_re_init();
}


void transmit_crsf_packet(Packet_t *Packet) {
    uint8_t packet_len = 0;
    switch (Packet->type) {
        case TYPE_LQ:       packet_len = sizeof(crsf_link_t); break;
        case TYPE_CHANNELS: packet_len = sizeof(crsf_channels_t); break;
        case TYPE_BATTERY:  packet_len = sizeof(crsf_battery_t); break;
        case TYPE_GPS:      packet_len = sizeof(crsf_gps_t); break;
        default:            return;
    }
    tx_buffer[0] = Packet->sync;
    tx_buffer[1] = packet_len + 2;
    tx_buffer[2] = Packet->type;
    for (int i = 0; i < packet_len; i++) {
        tx_buffer[i + 3] = Packet->payload[i];
    }
    tx_buffer[packet_len + 3] = crc8(&tx_buffer[2], packet_len + 1);
    HAL_UART_Transmit_DMA(crsf_uart_handle, tx_buffer, packet_len + 4);
}


#ifdef HALF_DUPLEX
void transmit_crsf_channels_and_receive_telemetry(uint8_t sync, crsf_channels_t *payload){
    HAL_UART_DMAStop(crsf_uart_handle); 
    HAL_HalfDuplex_EnableTransmitter(crsf_uart_handle);
    CRSF_send_channels_data(sync, payload);
}
#endif

void CRSF_send_channels_data(uint8_t sync, crsf_channels_t *payload)
{
    crsf_channels_t payload_proc = *payload;
    Packet_t Packet;
    Packet.sync = sync;
    Packet.type = TYPE_CHANNELS;
    Packet.payload_length = sizeof(crsf_channels_t);

    memcpy(Packet.payload, &payload_proc, sizeof(crsf_channels_t));
    transmit_crsf_packet(&Packet);
}

void CRSF_send_gps_data(uint8_t sync, const crsf_gps_t *payload)
{
    crsf_gps_t payload_proc = *payload;
    Packet_t Packet;
    Packet.sync = sync;
    Packet.type = TYPE_GPS;
    Packet.payload_length = sizeof(crsf_gps_t);

    payload_proc.latitude    = (int32_t)__builtin_bswap32((uint32_t)payload_proc.latitude);
    payload_proc.longitude   = (int32_t)__builtin_bswap32((uint32_t)payload_proc.longitude);
    payload_proc.groundspeed = __builtin_bswap16(payload_proc.groundspeed);
    payload_proc.heading     = __builtin_bswap16(payload_proc.heading);
    payload_proc.altitude    = __builtin_bswap16(payload_proc.altitude);

    memcpy(Packet.payload, &payload_proc, sizeof(crsf_gps_t));
    transmit_crsf_packet(&Packet);
}

void CRSF_send_battery_data(uint8_t sync, crsf_battery_t *payload)
{
    crsf_battery_t payload_proc = *payload;
    Packet_t Packet;
    Packet.sync = sync;
    Packet.type = TYPE_BATTERY;
    Packet.payload_length = sizeof(crsf_battery_t);

    payload_proc.voltage = __builtin_bswap16(payload_proc.voltage);
    payload_proc.current = __builtin_bswap16(payload_proc.current);
    payload_proc.capacity = __builtin_bswap16(payload_proc.capacity) << 8;

    memcpy(Packet.payload, &payload_proc, sizeof(crsf_battery_t));
    transmit_crsf_packet(&Packet);
}

void CRSF_send_arbitraty_data_over_channels_EXPERIMENTAL(uint8_t sync, uint8_t *data){
    crsf_channels_t payload = *(crsf_channels_t *)data;
    transmit_crsf_channels_and_receive_telemetry(sync, &payload);
}