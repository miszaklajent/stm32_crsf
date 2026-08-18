/**
  ******************************************************************************
  * @file           : rx_telemetry_example.c
  * @brief          : Example demonstrating CRSF receiver & telemetry on STM32
  ******************************************************************************
  */

#include "main.h"
#include "crsf.h"
#include <stdlib.h>

extern UART_HandleTypeDef huart2;

/* Global variables for viewing in debugger Live Expressions */
volatile Packet_t g_crsf_packet = {0};
volatile crsf_channels_t g_received_channels = {0};
volatile uint16_t g_crsf_channels[16] = {0};
volatile uint32_t g_packets_received_count = 0;

/* Telemetry payload */
crsf_gps_t g_gps_telemetry = {0};

void example_crsf_setup(void)
{
    /* Initialize CRSF reception over DMA with IDLE line detection */
    crsf_init(&huart2);
}

void example_crsf_send_periodic_gps(void)
{
    /* Update GPS telemetry data */
    g_gps_telemetry.latitude    = 522297000 + ((rand() % 2000) - 1000);  // 52.2297000 deg
    g_gps_telemetry.longitude   = 210122000 + ((rand() % 2000) - 1000);  // 21.0122000 deg
    g_gps_telemetry.groundspeed = 150 + (rand() % 50);                   // ~15 km/h
    g_gps_telemetry.heading     = (rand() % 360) * 100;                  // 0..359.99 deg
    g_gps_telemetry.altitude    = 1000 + 150 + (rand() % 20);            // 150m ASL (+1000m offset)
    g_gps_telemetry.satellites  = 12;

    /* Transmit GPS packet over UART DMA (0xC8 sync = Flight Controller source) */
    CRSF_send_gps_data(0xC8, &g_gps_telemetry);
}

/* Callbacks required in your application (e.g. main.c or stm32g0xx_it.c) */

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        /* Re-enable receiver DMA after half-duplex transmit finishes */
        crsf_re_init();
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART2)
    {
        if (huart->RxEventType == HAL_UART_RXEVENT_IDLE || huart->RxEventType == HAL_UART_RXEVENT_TC)
        {
            process_crsf_packet((Packet_t *)&g_crsf_packet);

            if (g_crsf_packet.type == TYPE_CHANNELS)
            {
                crsf_channels_t *ch = (crsf_channels_t *)g_crsf_packet.payload;
                g_received_channels = *ch;

                /* Unpack into 16-channel array for easy indexing (values: 172..1811) */
                g_crsf_channels[0]  = ch->ch1;
                g_crsf_channels[1]  = ch->ch2;
                g_crsf_channels[2]  = ch->ch3;
                g_crsf_channels[3]  = ch->ch4;
                g_crsf_channels[4]  = ch->ch5;
                g_crsf_channels[5]  = ch->ch6;
                g_crsf_channels[6]  = ch->ch7;
                g_crsf_channels[7]  = ch->ch8;
                g_crsf_channels[8]  = ch->ch9;
                g_crsf_channels[9]  = ch->ch10;
                g_crsf_channels[10] = ch->ch11;
                g_crsf_channels[11] = ch->ch12;
                g_crsf_channels[12] = ch->ch13;
                g_crsf_channels[13] = ch->ch14;
                g_crsf_channels[14] = ch->ch15;
                g_crsf_channels[15] = ch->ch16;

                g_packets_received_count++;
            }
        }
    }
}
