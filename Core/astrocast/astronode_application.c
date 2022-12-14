//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
// Standard
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>      // for isprint function

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
#define ASTRONODE_BIT_OFFSET_CMD_AVA_EVT_PIN_MASK   2
#define ASTRONODE_BIT_OFFSET_MSG_TXP_EVT_PIN_MASK   3

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
#define ASTRONODE_BIT_OFFSET_CMD            2
#define ASTRONODE_BIT_OFFEST_MSG_TX         3

#define ASTRONODE_BYTE_OFFSET_SSC_WR_PERIOD_ENUM    0
#define ASTRONODE_BYTE_OFFSET_SSC_WR_ENA_SEARCH     1
#define ASTRONODE_BIT_OFFSET_ENA_SEARCH             0

#define ASTRONODE_UART_DEBUG_BUFFER_LENGTH 80

#define PC_COUNTER_ID_SAT_DET_PHASE_COUNT               0x01
#define PC_COUNTER_ID_SAT_DET_OPERATIONS_COUNT          0x02
#define PC_COUNTER_ID_SIGNALLING_DEMOD_PHASE_COUNT      0x03
#define PC_COUNTER_ID_SIGNALLING_DEMOD_ATTEMPTS_COUNT   0x04
#define PC_COUNTER_ID_SIGNALLING_DEMOD_SUCCESSES_COUNT  0x05
#define PC_COUNTER_ID_ACK_DEMOD_ATTEMPTS_COUNT          0x06
#define PC_COUNTER_ID_ACK_DEMOD_SUCCESS_COUNT           0x07
#define PC_COUNTER_ID_QUEUED_MSG_COUNT                  0x08
#define PC_COUNTER_ID_DEQUEUED_UNACKED_MSG_COUNT        0x09
#define PC_COUNTER_ID_ACKED_MSG_COUNT                   0x0A
#define PC_COUNTER_ID_SENT_FRAG_COUNT                   0x0B
#define PC_COUNTER_ID_ACKED_FRAG_COUNT                  0x0C
#define PC_COUNTER_ID_COMMAND_DEMOD_ATTEMPT_COUNT       0x0D
#define PC_COUNTER_ID_COMMAND_DEMOD_SUCCESS_COUNT       0x0E
#define PC_COUNTER_ID_MSGS_IN_QUEUE                     0x41
#define PC_COUNTER_ID_ACKED_MSGS_IN_QUEUE               0x42
#define PC_COUNTER_ID_LAST_RESET_REASON                 0x43
#define PC_COUNTER_ID_UPTIME_COUNTER                    0x44
#define PC_COUNTER_ID_START_OF_LAST_PASS                0x51
#define PC_COUNTER_ID_END_OF_LAST_PASS                  0x52
#define PC_COUNTER_ID_PEAK_POWER_OF_LAST_PASS           0x53
#define PC_COUNTER_ID_TIME_PEAK_POWER_OF_LAST_PASS      0x54
#define PC_COUNTER_ID_LAST_MAC_RESULT                   0x61
#define PC_COUNTER_ID_LAST_SEARCH_POWER                 0x62
#define PC_COUNTER_ID_LAST_SEARCH_TIME                  0x63


//------------------------------------------------------------------------------
// Global variable definitions
//------------------------------------------------------------------------------
static bool g_is_sak_available = false;
static bool g_is_astronode_reset = false;
static bool g_is_command_available = false;
static bool g_is_tx_msg_pending = false;

//------------------------------------------------------------------------------
// Function declaration
//------------------------------------------------------------------------------
void append_multiple_data_size_to_string(char * const p_str, uint32_t * const p_data, uint8_t size);


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
            if (answer.p_payload[ASTRONODE_BYTE_OFFSET_CFG_RR_EVT_PIN_MASK] & (1 << ASTRONODE_BIT_OFFSET_RST_NTF_EVT_PIN_MASK))
            {
                send_debug_logs("EVT pin shows EVT register Reset Event Notification bit state.");
            }
            else
            {
                send_debug_logs("EVT pin does not show EVT register Reset Event Notification bit state.");
            }
            if (answer.p_payload[ASTRONODE_BYTE_OFFSET_CFG_RR_EVT_PIN_MASK] & (1 << ASTRONODE_BIT_OFFSET_CMD_AVA_EVT_PIN_MASK))
            {
                send_debug_logs("EVT pin shows EVT register Command Available bit state.");
            }
            else
            {
                send_debug_logs("EVT pin does not show EVT register Command Available bit state.");
            }
            if (answer.p_payload[ASTRONODE_BYTE_OFFSET_CFG_RR_EVT_PIN_MASK] & (1 << ASTRONODE_BIT_OFFSET_MSG_TXP_EVT_PIN_MASK))
            {
                send_debug_logs("EVT pin shows EVT register Message Transmission pending bit state.");
            }
            else
            {
                send_debug_logs("EVT pin does not shows EVT register Message Transmission pending bit state.");
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
                            bool reset_notification_event_pin_mask,
							bool command_available_event_pin_mask,
							bool message_tx_event_pin_mask)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_CFG_WR;

    request.p_payload[ASTRONODE_BYTE_OFFSET_CFG_WR_CONFIG] = payload_acknowledgment << ASTRONODE_BIT_OFFSET_PAYLOAD_ACK
        | add_geolocation << ASTRONODE_BIT_OFFSET_ADD_GEO
        | enable_ephemeris << ASTRONODE_BIT_OFFSET_ENABLE_EPH
        | deep_sleep_mode << ASTRONODE_BIT_OFFSET_DEEP_SLEEP_MODE;

    request.p_payload[ASTRONODE_BYTE_OFFSET_CFG_WR_EVT_PIN_MASK] = message_ack_event_pin_mask << ASTRONODE_BIT_OFFSET_MSG_ACK_EVT_PIN_MASK
        | reset_notification_event_pin_mask << ASTRONODE_BIT_OFFSET_RST_NTF_EVT_PIN_MASK
        | command_available_event_pin_mask << ASTRONODE_BIT_OFFSET_CMD_AVA_EVT_PIN_MASK
        | message_tx_event_pin_mask << ASTRONODE_BIT_OFFSET_MSG_TXP_EVT_PIN_MASK;

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

void astronode_send_mgi_rr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_MGI_RR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_MGI_RA)
        {
            char guid[answer.payload_len];
            send_debug_logs("Module GUID is:");
            snprintf(guid, answer.payload_len, "%s", answer.p_payload);
            send_debug_logs(guid);
        }
        else
        {
            send_debug_logs("Failed to read module GUID.");
        }
    }
}

void astronode_send_msn_rr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_MSN_RR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_MSN_RA)
        {
            char serial_number[answer.payload_len];
            send_debug_logs("Module's Serial Number is:");
            snprintf(serial_number, answer.payload_len, "%s", answer.p_payload);
            send_debug_logs(serial_number);
        }
        else
        {
            send_debug_logs("Failed to read module Serial Number.");
        }
    }
}

void astronode_send_nco_rr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_NCO_RR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_NCO_RA)
        {
            uint32_t time_to_next_pass = answer.p_payload[0]
                                        + (answer.p_payload[1] << 8)
                                        + (answer.p_payload[2] << 16)
                                        + (answer.p_payload[3] << 24);
            char str[ASTRONODE_UART_DEBUG_BUFFER_LENGTH];
            sprintf(str, "Next opportunity for communication with the Astrocast Network: %lds.", time_to_next_pass);
            send_debug_logs(str);
        }
        else
        {
            send_debug_logs("Failed to read satellite constellation ephemeris data.");
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
            if ((answer.p_payload[ASTRONODE_BYTE_OFFSET_EVT_RR_EVENT]) & (1 << ASTRONODE_BIT_OFFSET_CMD))
            {
                g_is_command_available = true;
                send_debug_logs("Command available.");
            }
            if ((answer.p_payload[ASTRONODE_BYTE_OFFSET_EVT_RR_EVENT]) & (1 << ASTRONODE_BIT_OFFEST_MSG_TX))
            {
                g_is_tx_msg_pending = true;
                send_debug_logs("TX message pending.");
            }

        }
    }
}

void astronode_send_geo_wr(int32_t latitude, int32_t longitude)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_GEO_WR;

    request.p_payload[request.payload_len++] = (uint8_t) latitude;
    request.p_payload[request.payload_len++] = (uint8_t) (latitude >> 8);
    request.p_payload[request.payload_len++] = (uint8_t) (latitude >> 16);
    request.p_payload[request.payload_len++] = (uint8_t) (latitude >> 24);

    request.p_payload[request.payload_len++] = (uint8_t) longitude;
    request.p_payload[request.payload_len++] = (uint8_t) (longitude >> 8);
    request.p_payload[request.payload_len++] = (uint8_t) (longitude >> 16);
    request.p_payload[request.payload_len++] = (uint8_t) (longitude >> 24);

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_GEO_WA)
        {
            send_debug_logs("Geolocation values were set successfully.");
        }
        else
        {
            send_debug_logs("Failed to set the geolocation information.");
        }
    }
}

void astronode_send_pld_dr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_PLD_DR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_PLD_EA)
        {
            uint16_t payload_id = answer.p_payload[0] + (answer.p_payload[1] << 8);
            char str[ASTRONODE_UART_DEBUG_BUFFER_LENGTH];
            sprintf(str, "Payload %d was successfully dequeued.", payload_id);
            send_debug_logs(str);
        }
        else
        {
            send_debug_logs("Failed to dequeue oldest payload.");
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

void astronode_send_pld_fr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_PLD_FR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_PLD_FA)
        {
            send_debug_logs("Entire payload queue has been cleared.");
        }
        else
        {
            send_debug_logs("Failed to clear the payload queue.");
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

void astronode_send_rtc_rr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_RTC_RR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_RTC_RA)
        {
            uint32_t rtc_time = answer.p_payload[0]
                                        + (answer.p_payload[1] << 8)
                                        + (answer.p_payload[2] << 16)
                                        + (answer.p_payload[3] << 24);
            char str[ASTRONODE_UART_DEBUG_BUFFER_LENGTH];
            sprintf(str, "RTC time since Astrocast Epoch (2018-01-01 00:00:00 UTC): %lds.", rtc_time);
            send_debug_logs(str);
        }
        else
        {
            send_debug_logs("Failed to read rtc time.");
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
            sprintf(str, "Acknowledgment for payload %d is available.", payload_id);
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

void astronode_send_ssc_wr(uint8_t search_period_enum, bool enable_search_without_msg_queued)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_SSC_WR;

    request.p_payload[ASTRONODE_BYTE_OFFSET_SSC_WR_PERIOD_ENUM] = search_period_enum;
    request.p_payload[ASTRONODE_BYTE_OFFSET_SSC_WR_ENA_SEARCH] = enable_search_without_msg_queued << ASTRONODE_BIT_OFFSET_ENA_SEARCH;

    request.payload_len = 2;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_SSC_WA)
        {
            send_debug_logs("Astronode satellite config successfully set.");
        }
        else
        {
            send_debug_logs("Failed to set the Astronode satellite configuration.");
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

void astronode_send_mpn_rr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_MPN_RR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_MPN_RA)
        {
            char product_number[answer.payload_len];
            send_debug_logs("Module's product number is:");
            snprintf(product_number, answer.payload_len, "%s", answer.p_payload);
            send_debug_logs(product_number);
        }
        else
        {
            send_debug_logs("Failed to read module Serial Number.");
        }
    }
}

void append_multiple_data_size_to_string(char * const p_str, uint32_t * const p_data, uint8_t size)
{
    switch (size)
    {
        case 1:
            sprintf(p_str, "%u", (uint8_t) *p_data);
            break;
        case 2:
            sprintf(p_str, "%u", (uint16_t) *p_data);
            break;
        case 4:
            sprintf(p_str, "%lu", *p_data);
            break;
        default:
            sprintf(p_str, "tlv size error %u", size);
    }
}

void astronode_send_per_rr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_PER_RR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_PER_RA)
        {
            uint16_t tlv_index = 0; // size 16bits to fit to payload_len
            char log_text[ASTRONODE_UART_DEBUG_BUFFER_LENGTH];
            uint8_t tlv_size = 0;
            while (tlv_index < answer.payload_len)
            {
                uint32_t *p_data = (uint32_t *) &answer.p_payload[tlv_index + 2];
                tlv_size = answer.p_payload[tlv_index + 1];
                switch (answer.p_payload[tlv_index])
                {
                    case PC_COUNTER_ID_SAT_DET_PHASE_COUNT:
                        send_debug_logs("PC sat det phase count is: ");
                        break;
                    case PC_COUNTER_ID_SAT_DET_OPERATIONS_COUNT:
                        send_debug_logs("PC sat det operation count is: ");
                        break;
                    case PC_COUNTER_ID_SIGNALLING_DEMOD_PHASE_COUNT:
                        send_debug_logs("PC signalling demod phase count is: ");
                        break;
                    case PC_COUNTER_ID_SIGNALLING_DEMOD_ATTEMPTS_COUNT:
                        send_debug_logs("PC signalling demod attemps count is: ");
                        break;
                    case PC_COUNTER_ID_SIGNALLING_DEMOD_SUCCESSES_COUNT:
                        send_debug_logs("PC signalling demod successes count is: ");
                        break;
                    case PC_COUNTER_ID_ACK_DEMOD_ATTEMPTS_COUNT:
                        send_debug_logs("PC ack demod attemps count is: ");
                        break;
                    case PC_COUNTER_ID_ACK_DEMOD_SUCCESS_COUNT:
                        send_debug_logs("PC ack demod success count is: ");
                        break;
                    case PC_COUNTER_ID_QUEUED_MSG_COUNT:
                        send_debug_logs("PC queued message count is: ");
                        break;
                    case PC_COUNTER_ID_DEQUEUED_UNACKED_MSG_COUNT:
                        send_debug_logs("PC dequeued unacked message count is: ");
                        break;
                    case PC_COUNTER_ID_ACKED_MSG_COUNT:
                        send_debug_logs("PC acked message count is: ");
                        break;
                    case PC_COUNTER_ID_SENT_FRAG_COUNT:
                        send_debug_logs("PC sent frag count is: ");
                        break;
                    case PC_COUNTER_ID_ACKED_FRAG_COUNT:
                        send_debug_logs("PC ack frag count is: ");
                        break;
                    case PC_COUNTER_ID_COMMAND_DEMOD_ATTEMPT_COUNT:
                        send_debug_logs("PC unicast demod attempt count is: ");
                        break;
                    case PC_COUNTER_ID_COMMAND_DEMOD_SUCCESS_COUNT:
                        send_debug_logs("PC unicast demod success count is: ");
                        break;
                    default:
                        send_debug_logs("PC error, type unknown");
                        tlv_size = 0;
                }
                append_multiple_data_size_to_string(log_text, p_data, tlv_size);
                send_debug_logs(log_text);
                log_text[0] = '\0';
                tlv_index += tlv_size + 2;
            }
        }
        else
        {
            send_debug_logs("Failed to get performance counters.");
        }
    }
}

void astronode_send_per_cr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_PER_CR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_PER_CA)
        {
            send_debug_logs("The performance counters have been cleared.");
        }
        else
        {
            send_debug_logs("Failed to clear performance counters.");
        }
    }
}

void astronode_send_mst_rr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_MST_RR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_MST_RA)
        {
            uint16_t tlv_index = 0; // size 16bits to fit to payload_len
            char log_text[ASTRONODE_UART_DEBUG_BUFFER_LENGTH];
            uint8_t tlv_size = 0;
            while (tlv_index < answer.payload_len)
            {
                uint32_t *p_data = (uint32_t *) &answer.p_payload[tlv_index + 2];
                tlv_size = answer.p_payload[tlv_index + 1];
                switch (answer.p_payload[tlv_index])
                {
                    case PC_COUNTER_ID_MSGS_IN_QUEUE:
                        send_debug_logs("MS messages in queue is: ");
                        break;
                    case PC_COUNTER_ID_ACKED_MSGS_IN_QUEUE:
                        send_debug_logs("MS acked messages in queue is: ");
                        break;
                    case PC_COUNTER_ID_LAST_RESET_REASON:
                        send_debug_logs("MS last reset reason is: ");
                        break;
                    case PC_COUNTER_ID_UPTIME_COUNTER:
                        send_debug_logs("MS uptime counter is: ");
                        break;
                    default:
                        send_debug_logs("Module state error, type unknown");
                        tlv_size = 0;
                }
                append_multiple_data_size_to_string(log_text, p_data, tlv_size);
                send_debug_logs(log_text);
                log_text[0] = '\0';
                tlv_index += tlv_size + 2;
            }
        }
        else
        {
            send_debug_logs("Failed to get module state.");
        }
    }
}

void astronode_send_lcd_rr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_LCD_RR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_LCD_RA)
        {
            uint16_t tlv_index = 0; // size 16bits to fit to payload_len
            char log_text[ASTRONODE_UART_DEBUG_BUFFER_LENGTH];
            uint8_t tlv_size = 0;
            while (tlv_index < answer.payload_len)
            {
                uint32_t *p_data = (uint32_t *) &answer.p_payload[tlv_index + 2];
                tlv_size = answer.p_payload[tlv_index + 1];
                switch (answer.p_payload[tlv_index])
                {
                    case PC_COUNTER_ID_START_OF_LAST_PASS:
                        send_debug_logs("Time of start of last pass contact is: ");
                        break;
                    case PC_COUNTER_ID_END_OF_LAST_PASS:
                        send_debug_logs("Time of end of last pass contact is: ");
                        break;
                    case PC_COUNTER_ID_PEAK_POWER_OF_LAST_PASS:
                        send_debug_logs("Peak RSSI of last pass is: ");
                        break;
                    case PC_COUNTER_ID_TIME_PEAK_POWER_OF_LAST_PASS:
                        send_debug_logs("Time of peak RSSI in last pass contact is: ");
                        break;
                    default:
                        send_debug_logs("Module state error, type unknown");
                        tlv_size = 0;
                }
                append_multiple_data_size_to_string(log_text, p_data, tlv_size);
                send_debug_logs(log_text);
                log_text[0] = '\0';
                tlv_index += tlv_size + 2;
            }
        }
        else
        {
            send_debug_logs("Failed to get module state.");
        }
    }
}

void astronode_send_end_rr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_END_RR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_END_RA)
        {
            uint16_t tlv_index = 0; // size 16bits to fit to payload_len
            char log_text[ASTRONODE_UART_DEBUG_BUFFER_LENGTH];
            uint8_t tlv_size = 0;
            while (tlv_index < answer.payload_len)
            {
                uint32_t *p_data = (uint32_t *) &answer.p_payload[tlv_index + 2];
                tlv_size = answer.p_payload[tlv_index + 1];
                switch (answer.p_payload[tlv_index])
                {
                    case PC_COUNTER_ID_LAST_MAC_RESULT:
                        send_debug_logs("PC Last MAC Result is: ");
                        break;
                    case PC_COUNTER_ID_LAST_SEARCH_POWER:
                        send_debug_logs("PC Last satellite search peak RSSI is: ");
                        break;
                    case PC_COUNTER_ID_LAST_SEARCH_TIME:
                        send_debug_logs("PC Time since last satellite search is: ");
                        break;
                    default:
                        send_debug_logs("Module state error, type unknown");
                        tlv_size = 0;
                }
                append_multiple_data_size_to_string(log_text, p_data, tlv_size);
                send_debug_logs(log_text);
                log_text[0] = '\0';
                tlv_index += tlv_size + 2;
            }
        }
        else
        {
            send_debug_logs("Failed to get module state.");
        }
    }
}

void astronode_send_cmd_cr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_CMD_CR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_CMD_CA)
        {
            g_is_command_available = false;
            send_debug_logs("The command ack has been cleared.");
        }
        else
        {
            send_debug_logs("No command to clear.");
        }
    }
}

void astronode_send_cmd_rr(void)
{
    astronode_app_msg_t request = {0};
    astronode_app_msg_t answer = {0};

    request.op_code = ASTRONODE_OP_CODE_CMD_RR;

    if (astronode_transport_send_receive(&request, &answer) == RS_SUCCESS)
    {
        if (answer.op_code == ASTRONODE_OP_CODE_CMD_RA)
        {
            send_debug_logs("Received downlink command");
            uint32_t rtc_time = answer.p_payload[0]
                                + (answer.p_payload[1] << 8)
                                + (answer.p_payload[2] << 16)
                                + (answer.p_payload[3] << 24);
            char str[ASTRONODE_UART_DEBUG_BUFFER_LENGTH];
            sprintf(str, "Command created date, Ref is astrocast Epoch (2018-01-01 00:00:00 UTC): %lds.", rtc_time);
            send_debug_logs(str);

            if (((answer.payload_len - 4) != 40) && ((answer.payload_len - 4) != 8))
            {
                send_debug_logs("Command size error");
                return;
            }

            char command_content[answer.payload_len];
            uint16_t command_content_size = snprintf(command_content, (answer.payload_len - 4) + 1, "%s", &answer.p_payload[4]);
            for (uint8_t index = 0; index < command_content_size; index++)
            {
            	if (isprint((unsigned char)command_content[index]) == 0)
                {
                    send_debug_logs("Command contains non printable characters");
                    return;
                }
            }
            send_debug_logs("Command content is: ");
            sprintf(command_content, "%s ", &answer.p_payload[4]);
            send_debug_logs(command_content);
        }
        else
        {
            send_debug_logs("No command available.");
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

bool is_command_available()
{
    return g_is_command_available;
}

bool is_tx_msg_pending()
{
    return g_is_tx_msg_pending;
}