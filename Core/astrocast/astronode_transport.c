//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
// Standard
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Astrocast
#include "astronode_definitions.h"
#include "astronode_transport.h"
#include "drivers.h"


//------------------------------------------------------------------------------
// Definitions
//------------------------------------------------------------------------------
// STX + OPCODE + Parameters + CRC + ETX
#define ASTRONODE_TRANSPORT_MSG_MAX_LEN_BYTES ( 1 + \
                                                2 + \
                                                2 * ASTRONODE_APP_MSG_MAX_LEN_BYTES + \
                                                4 + \
                                                1)


//------------------------------------------------------------------------------
// Global variable definitions
//------------------------------------------------------------------------------
static const uint8_t g_ascii_lookup[16] =
{
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};


//------------------------------------------------------------------------------
// Function declarations
//------------------------------------------------------------------------------
static bool ascii_to_value(const uint8_t ascii, uint8_t *p_value);
static uint16_t astronode_create_request_transport(astronode_app_msg_t *p_source_message, uint8_t *p_destination_buffer);
static return_status_t astronode_decode_answer_transport(uint8_t *p_source_buffer, uint16_t length_buffer, astronode_app_msg_t *p_destination_message);
static uint16_t calculate_crc(const uint8_t *p_data, uint16_t data_len, uint16_t init_value);
static void check_for_error(astronode_app_msg_t *p_answer);
static return_status_t receive_astronode_answer(uint8_t *p_rx_buffer, uint16_t *p_buffer_length);
static void uint8_to_ascii_buffer(const uint8_t value, uint8_t *p_target_buffer);


//------------------------------------------------------------------------------
// Function definitions
//------------------------------------------------------------------------------
static bool ascii_to_value(const uint8_t ascii, uint8_t *p_value)
{
    if (ascii >= '0' && ascii <= '9')
    {
        *p_value = ascii - '0';
        return true;
    }
    else if (ascii >= 'A' && ascii <= 'F')
    {
        *p_value = ascii - 'A' + 10;
        return true;
    }
    else
    {
        return false;
    }
}

static uint16_t astronode_create_request_transport(astronode_app_msg_t *p_source_message, uint8_t *p_destination_buffer)
{
    uint16_t index = 0;

    p_destination_buffer[index++] = ASTRONODE_TRANSPORT_STX;

    uint16_t crc = calculate_crc((const uint8_t *)&p_source_message->op_code, 1, 0xFFFF);
    crc = calculate_crc((const uint8_t *)&p_source_message->p_payload, p_source_message->payload_len, crc);
    crc = ((crc << 8) & 0xff00) | ((crc >> 8) & 0x00ff);

    uint8_to_ascii_buffer(p_source_message->op_code, &p_destination_buffer[index]);
    index += 2;

    for (uint16_t i = 0; i < p_source_message->payload_len; i++)
    {
        uint8_to_ascii_buffer(p_source_message->p_payload[i], &p_destination_buffer[index]);
        index += 2;
    }

    uint8_to_ascii_buffer(crc >> 8, &p_destination_buffer[index]);
    index += 2;
    uint8_to_ascii_buffer(crc & 0xFF, &p_destination_buffer[index]);
    index += 2;

    p_destination_buffer[index++] = ASTRONODE_TRANSPORT_ETX;

    return index;
}

static return_status_t astronode_decode_answer_transport(uint8_t *p_source_buffer, uint16_t length_buffer, astronode_app_msg_t *p_destination_message)
{
    if (p_source_buffer[0] != ASTRONODE_TRANSPORT_STX)
    {
        send_debug_logs("ERROR : Message received from the Astronode does not start with STX character.");
        return RS_FAILURE;
    }

    if (length_buffer % 2 == 1 || length_buffer < 8) // 8: STX, ETX, 2 x opcode, 4 x CRC
    {
        send_debug_logs("ERROR : Message received from the Astronode is missing at least one character.");
        return RS_FAILURE;
    }

    p_destination_message->payload_len = (length_buffer - 8) / 2;

    if (p_source_buffer[length_buffer - 1] != ASTRONODE_TRANSPORT_ETX)
    {
        send_debug_logs("ERROR : Message received from the Astronode does not end with ETX character.");
        return RS_FAILURE;
    }

    uint8_t nibble_high = 0;
    uint8_t nibble_low = 0;

    // Op code
    if (ascii_to_value(p_source_buffer[1], &nibble_high) == false
        || ascii_to_value(p_source_buffer[2], &nibble_low) == false)
    {
        send_debug_logs("ERROR : Message received from the Astronode contains a non-ASCII character.");
        return RS_FAILURE;
    }

    p_destination_message->op_code = (nibble_high << 4) + nibble_low;

    // Payload
    for (uint16_t i = 3, j = 0; i < length_buffer - 5; i += 2)
    {
        if (ascii_to_value(p_source_buffer[i], &nibble_high) == false
            || ascii_to_value(p_source_buffer[i + 1], &nibble_low) == false)
        {
            send_debug_logs("ERROR : Message received from the Astronode contains a non-ASCII character.");
            return RS_FAILURE;
        }

        p_destination_message->p_payload[j++] = (nibble_high << 4) + nibble_low;
    }

    // CRC
    uint16_t crc_calculated = calculate_crc((const uint8_t *)&p_destination_message->op_code, 1, 0xFFFF);
    crc_calculated = calculate_crc((const uint8_t *)&p_destination_message->p_payload, p_destination_message->payload_len, crc_calculated);
    crc_calculated = ((crc_calculated << 8) & 0xff00) | ((crc_calculated >> 8) & 0x00ff);


    if (ascii_to_value(p_source_buffer[length_buffer - 5], &nibble_high) == false
        || ascii_to_value(p_source_buffer[length_buffer - 4], &nibble_low) == false)
    {
        send_debug_logs("ERROR : Message received from the Astronode contains a non-ASCII character.");
        return RS_FAILURE;
    }

    uint16_t crc_received = (nibble_high << 12) + (nibble_low << 8);

    if (ascii_to_value(p_source_buffer[length_buffer - 3], &nibble_high) == false
        || ascii_to_value(p_source_buffer[length_buffer - 2], &nibble_low) == false)
    {
        send_debug_logs("ERROR : Message received from the Astronode contains a non-ASCII character.");
        return RS_FAILURE;
    }

    crc_received += (nibble_high << 4) + nibble_low;

    if (crc_received != crc_calculated)
    {
        send_debug_logs("ERROR : CRC sent by the Astronode does not match the expected CRC");
        return RS_FAILURE;
    }

    if (p_destination_message->op_code == ASTRONODE_OP_CODE_ERROR)
    {
        check_for_error(p_destination_message);
    }

    return RS_SUCCESS;
}

return_status_t astronode_transport_send_receive(astronode_app_msg_t *p_request, astronode_app_msg_t *p_answer)
{
    uint8_t request_transport[ASTRONODE_TRANSPORT_MSG_MAX_LEN_BYTES] = {0};
    uint8_t answer_transport[ASTRONODE_TRANSPORT_MSG_MAX_LEN_BYTES] = {0};
    uint16_t answer_length =  0;

    uint16_t request_length = astronode_create_request_transport(p_request, request_transport);

    send_astronode_request(request_transport, request_length);

    if(receive_astronode_answer(answer_transport, &answer_length) == RS_SUCCESS)
    {
        return astronode_decode_answer_transport(answer_transport, answer_length, p_answer);
    }
    else
    {
        return RS_FAILURE;
    }
}

static uint16_t calculate_crc(const uint8_t *p_data, uint16_t data_len, uint16_t init_value)
{
    uint16_t crc = init_value;

    while (data_len--)
    {
        uint16_t x = crc >> 8 ^ *p_data++;
        x ^= x >> 4;
        crc = (crc << 8) ^ (x << 12) ^ (x << 5) ^ (x);
    }
    return crc;
}

static void check_for_error(astronode_app_msg_t *p_answer)
{
    uint16_t error_code = p_answer->p_payload[0] + (p_answer->p_payload[1] << 8);

    switch (error_code)
    {
        case ASTRONODE_ERR_CODE_CRC_NOT_VALID:
            send_debug_logs("[ERROR] CRC_NOT_VALID : Discrepancy between provided CRC and expected CRC.");
            break;

        case ASTRONODE_ERR_CODE_LENGTH_NOT_VALID:
            send_debug_logs("[ERROR] LENGTH_NOT_VALID : Message exceeds the maximum length allowed by the given operation code.");
            break;

        case ASTRONODE_ERR_CODE_OPCODE_NOT_VALID:
            send_debug_logs("[ERROR] OPCODE_NOT_VALID : Invalid operation code used.");
            break;

        case ASTRONODE_ERR_CODE_FORMAT_NOT_VALID:
            send_debug_logs("[ERROR] FORMAT_NOT_VALID : At least one of the fields (SSID, password, token) is not composed of exclusively printable standard ASCII characters (0x20 to 0x7E).");
            break;

        case ASTRONODE_ERR_CODE_FLASH_WRITING_FAILED:
            send_debug_logs("[ERROR] FLASH_WRITING_FAILED : Failed to write the Wi-Fi settings (SSID, password, token) to the flash.");
            break;

        case ASTRONODE_ERR_CODE_BUFFER_FULL:
            send_debug_logs("[ERROR] BUFFER_FULL : Failed to queue the payload because the sending queue is already full.");
            break;

        case ASTRONODE_ERR_CODE_DUPLICATE_ID:
            send_debug_logs("[ERROR] DUPLICATE_ID : Failed to queue the payload because the Payload ID provided by the asset is already in use in the Astronode queue.");
            break;

        case ASTRONODE_ERR_CODE_BUFFER_EMPTY:
            send_debug_logs("[ERROR] BUFFER_EMPTY : Failed to dequeue a payload from the buffer because the buffer is empty.");
            break;

        case ASTRONODE_ERR_CODE_INVALID_POS:
            send_debug_logs("[ERROR] INVALID_POS : Failed to update the geolocation information. Latitude and longitude fields must in the range [-90,90] degrees and [-180,180] degrees, respectively.");
            break;

        case ASTRONODE_ERR_CODE_NO_ACK:
            send_debug_logs("[ERROR] NO_ACK : No satellite acknowledgement available for any payload.");
            break;

        case ASTRONODE_ERR_CODE_NO_CLEAR:
            send_debug_logs("[ERROR] NO_CLEAR : No payload ack to clear, or it was already cleared.");
            break;

        default:
            send_debug_logs("[ERROR] error_code is not defined.");
            break;
    }
}

static return_status_t receive_astronode_answer(uint8_t *p_rx_buffer, uint16_t *p_buffer_length)
{
    uint8_t rx_char = 0;
    uint16_t length = 0;
    uint32_t timeout_answer_received = get_systick();
    bool is_answer_received = false;

    while (is_answer_received == false)
    {
        if (is_systick_timeout_over(timeout_answer_received, ASTRONODE_ANSWER_TIMEOUT_MS))
        {
            send_debug_logs("ERROR : Received answer timeout..");
            return RS_FAILURE;
        }
        if (is_astronode_character_received(&rx_char))
        {
            if (rx_char == ASTRONODE_TRANSPORT_STX)
            {
                is_answer_received = false;
                length = 0;
            }

            p_rx_buffer[length] = rx_char;
            length++;

            if (length > ASTRONODE_MAX_LENGTH_RESPONSE)
            {
                send_debug_logs("ERROR : Message received from the Astronode exceed maximum length allowed.");
                return RS_FAILURE;
            }

            if (rx_char == ASTRONODE_TRANSPORT_ETX)
            {
                if (length > 1)
                {
                    *p_buffer_length = length;
                    is_answer_received = true;
                }
            }
        }
    }

    send_debug_logs("Message received from the Astronode <-- ");
    send_debug_logs((char *) p_rx_buffer);

    return RS_SUCCESS;
}

static void uint8_to_ascii_buffer(const uint8_t value, uint8_t *p_target_buffer)
{
    p_target_buffer[0] = g_ascii_lookup[value >> 4];
    p_target_buffer[1] = g_ascii_lookup[value & 0x0F];
}
