
#ifndef ASTRONODE_APPLICATION_H
#define ASTRONODE_APPLICATION_H


//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
// Standard
#include <stdint.h>
#include <stdbool.h>

// Astrocast
#include "astronode_definitions.h"


//------------------------------------------------------------------------------
// Definitions
//------------------------------------------------------------------------------
#define ASTRONODE_APP_MSG_MAX_LEN_BYTES     194 // Wi-Fi write is 194 bytes
#define ASTRONODE_APP_PAYLOAD_MAX_LEN_BYTES 160 // 152 if geolocation is used


//------------------------------------------------------------------------------
// Type definitions
//------------------------------------------------------------------------------
typedef struct astronode_app_msg_t
{
    astronode_op_code   op_code;
    char                p_payload[ASTRONODE_APP_MSG_MAX_LEN_BYTES];
    uint16_t            payload_len;
} astronode_app_msg_t;


//------------------------------------------------------------------------------
// Function declarations
//------------------------------------------------------------------------------
void astronode_send_cfg_fr(void);

void astronode_send_cfg_rr(void);

void astronode_send_cfg_sr(void);

void astronode_send_cfg_wr( bool payload_acknowledgment,
                            bool add_geolocation,
                            bool enable_ephemeris,
                            bool deep_sleep_mode,
                            bool message_ack_event_pin_mask,
                            bool reset_notification_event_pin_mask,
                            bool command_available_event_pin_mask,
                            bool message_tx_event_pin_mask);

void astronode_send_ctx_sr(void);                            

void astronode_send_mgi_rr(void);

void astronode_send_msn_rr(void);

void astronode_send_nco_rr(void);

void astronode_send_evt_rr(void);

void astronode_send_geo_wr(int32_t latitude, int32_t longitude);

void astronode_send_pld_dr(void);

void astronode_send_pld_er(uint16_t payload_id, char *p_payload, uint16_t payload_length);

void astronode_send_pld_fr(void);

void astronode_send_res_cr(void);

void astronode_send_rtc_rr(void);

void astronode_send_sak_cr(void);

void astronode_send_sak_rr(void);

void astronode_send_ssc_wr(uint8_t search_period_enum, bool enable_search_without_msg_queued);

void astronode_send_wif_wr(char *p_wlan_ssid, char *p_wlan_key, char *p_auth_token);

void astronode_send_mpn_rr(void);

void astronode_send_per_cr(void);

void astronode_send_per_rr(void);

void astronode_send_mst_rr(void);

void astronode_send_lcd_rr(void);

void astronode_send_end_rr(void);

void astronode_send_cmd_cr(void);

void astronode_send_cmd_rr(void);

/**
 * @brief Check is an acknowledgment has been read.
 */
bool is_sak_available(void);

/**
 * @brief Check is the Astronode reset has been read.
 */
bool is_astronode_reset(void);

/**
 * @brief Check is a unicast command available.
 */
bool is_command_available(void);

/**
 * @brief Check is TX message pending.
 */
bool is_tx_msg_pending(void);


#endif /* ASTRONODE_APPLICATION_H */
