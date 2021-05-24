
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
                            bool reset_notification_event_pin_mask);

void astronode_send_dgi_rr(void);

void astronode_send_dsn_rr(void);

void astronode_send_eph_rr(void);

void astronode_send_evt_rr(void);

void astronode_send_geo_wr(int32_t latitude, int32_t longitude);

void astronode_send_pld_dr(uint16_t payload_id);

void astronode_send_pld_er(uint16_t payload_id, char *p_payload, uint16_t payload_length);

void astronode_send_pld_fr(void);

void astronode_send_res_cr(void);

void astronode_send_rtc_rr(void);

void astronode_send_sak_cr(void);

void astronode_send_sak_rr(void);

void astronode_send_wif_wr(char *p_wlan_ssid, char *p_wlan_key, char *p_auth_token);


/**
 * @brief Check is an acknowledgment has been read.
 */
bool is_sak_available(void);

/**
 * @brief Check is the Astronode reset has been read.
 */
bool is_astronode_reset(void);


#endif /* ASTRONODE_APPLICATION_H */
