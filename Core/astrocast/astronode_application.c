//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
// Standard
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

// Astrocast
#include "astronode_definitions.h"
#include "astronode_application.h"
#include "astronode_transport.h"
#include "drivers.h"


//------------------------------------------------------------------------------
// Definitions
//------------------------------------------------------------------------------
#define ASTRONODE_BYTE_OFFSET_CFG_WR_CONFIG     0
#define ASTRONODE_BIT_OFFSET_PAYLOAD_ACK        0
#define ASTRONODE_BIT_OFFSET_ADD_GEO            1
#define ASTRONODE_BIT_OFFSET_ENABLE_EPH         2
#define ASTRONODE_BIT_OFFSET_DEEP_SLEEP_MODE    3

#define ASTRONODE_BYTE_OFFSET_CFG_WR_EVT_PIN_MASK   2
#define ASTRONODE_BIT_OFFSET_MSG_ACK_EVT_PIN_MASK   0
#define ASTRONODE_BIT_OFFSET_RST_NTF_EVT_PIN_MASK   1

#define ASTRONODE_BYTE_OFFSET_CFG_RR_DEV_TYPE_ID    0
#define ASTRONODE_BYTE_OFFSET_CFG_RR_HW_REV         1
#define ASTRONODE_BYTE_OFFSET_CFG_RR_FW_MAJOR_VER   2
#define ASTRONODE_BYTE_OFFSET_CFG_RR_FW_MINOR_VER   3
#define ASTRONODE_BYTE_OFFSET_CFG_RR_FW_REV         4
#define ASTRONODE_BYTE_OFFSET_CFG_RR_CONFIG         5
#define ASTRONODE_BYTE_OFFSET_CFG_RR_EVT_PIN_MASK   7

#define ASTRONODE_BYTE_OFFSET_EVT_RR_EVENT  0
#define ASTRONODE_BIT_OFFSET_ACK            0
#define ASTRONODE_BIT_OFFSET_RST            1


//------------------------------------------------------------------------------
// Global variable definitions
//------------------------------------------------------------------------------
static bool g_is_sak_available = false;
static bool g_is_astronode_reset = false;


//------------------------------------------------------------------------------
// Function definitions
//------------------------------------------------------------------------------
void astronode_send_cfg_fr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_CFG_FR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_CFG_FA)
        {
            send_debug_logs("Astronode settings reverted to default values.");
        }
        else
        {
            send_debug_logs("Failed to process the factory reset.");
        }
    }
}

void astronode_send_cfg_rr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_CFG_RR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_CFG_RA)
        {
            char str[ASTRONODE_UART_DEBUG_BUFFER_LENGTH];

            send_debug_logs("Astronode settings read successfully:");

            switch (answer.p_payload[ASTRONODE_BYTE_OFFSET_CFG_RR_DEV_TYPE_ID])
            {
                case 3:
                    send_debug_logs("Device Type ID : Commercial Satellite Astronode.");
                    break;

                case 4:
                    send_debug_logs("Device Type ID : Commercial Wi-Fi Dev Kit.");
                    break;

                default:
                    send_debug_logs("Error reading the Device Type ID.");
                    break;
            }

            sprintf(str, "Hardware revision : %d, Firmware version : %d.%d.%d",
                    answer.p_payload[ASTRONODE_BYTE_OFFSET_CFG_RR_HW_REV],
                    answer.p_payload[ASTRONODE_BYTE_OFFSET_CFG_RR_FW_MAJOR_VER],
                    answer.p_payload[ASTRONODE_BYTE_OFFSET_CFG_RR_FW_MINOR_VER],
                    answer.p_payload[ASTRONODE_BYTE_OFFSET_CFG_RR_FW_REV]);

            send_debug_logs(str);

            if (answer.p_payload[ASTRONODE_BYTE_OFFSET_CFG_RR_CONFIG] & (1 << ASTRONODE_BIT_OFFSET_PAYLOAD_ACK))
            {
                send_debug_logs("Asset is informed of payload acks.");
            }
            else
            {
                send_debug_logs("Asset is not informed of payload acks.");
            }
            if (answer.p_payload[ASTRONODE_BYTE_OFFSET_CFG_RR_CONFIG] & (1 << ASTRONODE_BIT_OFFSET_ADD_GEO))
            {
                send_debug_logs("Geolocation is on.");
            }
            else
            {
                send_debug_logs("No geolocation.");
            }
            if (answer.p_payload[ASTRONODE_BYTE_OFFSET_CFG_RR_CONFIG] & (1 << ASTRONODE_BIT_OFFSET_ENABLE_EPH))
            {
                send_debug_logs("Ephemeris is enabled.");
            }
            else
            {
                send_debug_logs("Ephemeris is disabled.");
            }
            if (answer.p_payload[ASTRONODE_BYTE_OFFSET_CFG_RR_CONFIG] & (1 << ASTRONODE_BIT_OFFSET_DEEP_SLEEP_MODE))
            {
                send_debug_logs("Deep sleep mode is used.");
            }
            else
            {
                send_debug_logs("Deep sleep mode not used.");
            }

            if (answer.p_payload[ASTRONODE_BYTE_OFFSET_CFG_RR_EVT_PIN_MASK] & (1 << ASTRONODE_BIT_OFFSET_MSG_ACK_EVT_PIN_MASK))
            {
                send_debug_logs("EVT pin shows EVT register Message Ack bit state.");
            }
            else
            {
                send_debug_logs("EVT pin does not show EVT register Message Ack bit state.");
            }
            if (answer.p_payload[ASTRONODE_BYTE_OFFSET_CFG_RR_EVT_PIN_MASK] & (1 << ASTRONODE_BIT_OFFSET_MSG_ACK_EVT_PIN_MASK))
            {
                send_debug_logs("EVT pin shows EVT register Message Ack bit state.");
            }
            else
            {
                send_debug_logs("EVT pin does not show EVT register Message Ack bit state.");
            }
            if (answer.p_payload[ASTRONODE_BYTE_OFFSET_CFG_RR_EVT_PIN_MASK] & (1 << ASTRONODE_BIT_OFFSET_RST_NTF_EVT_PIN_MASK))
            {
                send_debug_logs("EVT pin does not show EVT register Reset Event Notification bit state.");
            }
            else
            {
                send_debug_logs("EVT pin shows EVT register Reset Event Notification bit state.");
            }
        }
        else
        {
            send_debug_logs("Failed to process the factory reset.");
        }
    }
}

void astronode_send_cfg_sr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_CFG_SR;

    astronode_transport_send_receive(&request, &answer);

    if (answer.op_code == ASTRONODE_OP_CODE_CFG_SA)
    {
        send_debug_logs("Astronode configuration successfully saved in NVM.");
    }
    else
    {
        send_debug_logs("Failed to save the Astronode configuration in NVM.");
    }
}

void astronode_send_cfg_wr(bool payload_acknowledgment,
                            bool add_geolocation,
                            bool enable_ephemeris,
                            bool deep_sleep_mode,
                            bool message_ack_event_pin_mask,
                            bool reset_notification_event_pin_mask)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_CFG_WR;

    request.p_payload[ASTRONODE_BYTE_OFFSET_CFG_WR_CONFIG] = payload_acknowledgment << ASTRONODE_BIT_OFFSET_PAYLOAD_ACK
        | add_geolocation << ASTRONODE_BIT_OFFSET_ADD_GEO
        | enable_ephemeris << ASTRONODE_BIT_OFFSET_ENABLE_EPH
        | deep_sleep_mode << ASTRONODE_BIT_OFFSET_DEEP_SLEEP_MODE;

    request.p_payload[ASTRONODE_BYTE_OFFSET_CFG_WR_EVT_PIN_MASK] = message_ack_event_pin_mask << ASTRONODE_BIT_OFFSET_MSG_ACK_EVT_PIN_MASK
        | reset_notification_event_pin_mask << ASTRONODE_BIT_OFFSET_RST_NTF_EVT_PIN_MASK;

    request.payload_len = 3;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_CFG_WA)
        {
            send_debug_logs("Astronode configuration successfully set.");
        }
        else
        {
            send_debug_logs("Failed to set the Astronode configuration.");
        }
    }
}

void astronode_send_evt_rr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_EVT_RR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_EVT_RA)
        {
            if ((answer.p_payload[ASTRONODE_BYTE_OFFSET_EVT_RR_EVENT]) & (1 << ASTRONODE_BIT_OFFSET_ACK))
            {
                g_is_sak_available = true;
                send_debug_logs("Message acknowledgment available.");
            }
            if ((answer.p_payload[ASTRONODE_BYTE_OFFSET_EVT_RR_EVENT]) & (1 << ASTRONODE_BIT_OFFSET_RST))
            {
                g_is_astronode_reset = true;
                send_debug_logs("Astronode has reset.");
            }
        }
    }
}

void astronode_send_pld_er(uint16_t payload_id, char *p_payload, uint16_t payload_length)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_PLD_ER;

    request.p_payload[request.payload_len++] = (uint8_t) payload_id;
    request.p_payload[request.payload_len++] = (uint8_t) (payload_id >> 8);

    memcpy(&request.p_payload[request.payload_len], p_payload, payload_length);
    request.payload_len = 2 + payload_length;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_PLD_EA)
        {
            send_debug_logs("Payload was successfully queued.");
        }
        else
        {
            send_debug_logs("Payload failed to be queued.");
        }
    }
}

void astronode_send_res_cr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_RES_CR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_RES_CA)
        {
            g_is_astronode_reset = false;
            send_debug_logs("The reset has been cleared.");
        }
        else
        {
            send_debug_logs("No reset to clear.");
        }
    }
}

void astronode_send_sak_rr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_SAK_RR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_SAK_RA)
        {
            uint16_t payload_id = answer.p_payload[0] + (answer.p_payload[1] << 8);
            char str[ASTRONODE_UART_DEBUG_BUFFER_LENGTH];
            sprintf(str, "Acknowledgment for message %d is available.", payload_id);
            send_debug_logs(str);
        }
        else
        {
            send_debug_logs("No acknowledgment available.");
        }
    }
}

void astronode_send_sak_cr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_SAK_CR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_SAK_CA)
        {
            g_is_sak_available = false;
            send_debug_logs("The acknowledgment has been cleared.");
        }
        else
        {
            send_debug_logs("No acknowledgment available.");
        }
    }
}

void astronode_send_wif_wr(char *p_wlan_ssid, char *p_wlan_key, char *p_auth_token)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_WIF_WR;

    memcpy(&request.p_payload[request.payload_len], p_wlan_ssid, ASTRONODE_WLAN_SSID_MAX_LENGTH);
    request.payload_len += ASTRONODE_WLAN_SSID_MAX_LENGTH;

    memcpy(&request.p_payload[request.payload_len], p_wlan_key, ASTRONODE_WLAN_KEY_MAX_LENGTH);
    request.payload_len += ASTRONODE_WLAN_KEY_MAX_LENGTH;

    memcpy(&request.p_payload[request.payload_len], p_auth_token, ASTRONODE_AUTH_TOKEN_MAX_LENGTH);
    request.payload_len += ASTRONODE_AUTH_TOKEN_MAX_LENGTH;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_WIF_WA)
        {
            send_debug_logs("WiFi settings successfully set.");
        }
        else
        {
            send_debug_logs("WiFi settings failed to be set.");
        }
    }
}


bool is_sak_available()
{
    return g_is_sak_available;
}

bool is_astronode_reset()
{
    return g_is_astronode_reset;
}
