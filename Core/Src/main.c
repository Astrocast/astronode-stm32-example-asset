//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
// Standard
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Astrocast
#include "main.h"
#include "astronode_application.h"
#include "astronode_definitions.h"
#include "drivers.h"


//------------------------------------------------------------------------------
// Global variable definitions
//------------------------------------------------------------------------------
uint16_t g_payload_id_counter = 0;


//------------------------------------------------------------------------------
// Function definitions
//------------------------------------------------------------------------------
int main(void)
{
    init_drivers();

    send_debug_logs("\nStart the application...");

    reset_astronode();

    // WIFI DEV KIT ONLY: comment this section to use the astronode.
    // Send WiFi configuration write to access Astrocast backend.
    char ssid[ASTRONODE_WLAN_SSID_MAX_LENGTH] = "my_wifi_ssid";
    char wlan_key[ASTRONODE_WLAN_KEY_MAX_LENGTH] = "my_wifi_password";
    char api_token[ASTRONODE_AUTH_TOKEN_MAX_LENGTH] = "6nxGR4eWYb4R8fEsXx2h1hGoR6nvku2TvGvTuFzxiGYPpICAAroZKttHnzXTQSLEilvCTT7r7E7urZ7iEW42fdibmXG4ROQz";
    astronode_send_wif_wr(ssid, wlan_key, api_token);
    // WIFI DEV KIT: end of section.

    uint32_t print_housekeeping_timer = get_systick();

    // Send config write with:
    // EVT pin shows sat ack
    // No geolocation
    // Ephemeris Enable
    // Deep Sleep not used
    // EVT pin shows Message Ack
    // EVT pin shows Reset
    // EVT pin shows downlink command available
    // EVT pin did not show tx message pending (keep it to false in this example)
    astronode_send_cfg_wr(true, false, true, false, true, true, true, false);

    // Store current configuration in NVM.
    astronode_send_cfg_sr();

    while (1)
    {
        if (is_evt_pin_high())
        {
            send_debug_logs("Evt pin is high.");
            astronode_send_evt_rr();
            if (is_sak_available())
            {
                astronode_send_sak_rr();
                astronode_send_sak_cr();
                send_debug_logs("Message has been acknowledged.");
                astronode_send_per_rr();
            }
            if (is_astronode_reset())
            {
                send_debug_logs("Terminal has been reset.");
                astronode_send_res_cr();
            }
            if (is_command_available())
            {
                send_debug_logs("Unicast command is available");
                astronode_send_cmd_rr();
                astronode_send_cmd_cr();
            }
        }
        else if (is_message_available())
        {
            send_debug_logs("The button is pressed.");

            g_payload_id_counter++;
            char payload[ASTRONODE_APP_PAYLOAD_MAX_LEN_BYTES] = {0};

            sprintf(payload, "Test message %d", g_payload_id_counter);

            astronode_send_pld_er(g_payload_id_counter, payload, strlen(payload));
        }

        if (get_systick() - print_housekeeping_timer > 60000)
        {
            astronode_send_per_rr();
            print_housekeeping_timer = get_systick();
        }
    }
}