#ifndef ASTRONODE_TRANSPORT_H
#define ASTRONODE_TRANSPORT_H


//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
// Standard

// Astrocast
#include "astronode_application.h"


//------------------------------------------------------------------------------
// Type definitions
//------------------------------------------------------------------------------
typedef enum return_status_t
{
    RS_FAILURE,
    RS_SUCCESS
} return_status_t;


//------------------------------------------------------------------------------
// Function declarations
//------------------------------------------------------------------------------
/**
 * @brief Send the message to the Asset Interface and return the response.
 */
return_status_t astronode_transport_send_receive(astronode_app_msg_t *p_request, astronode_app_msg_t *p_answer);


#endif /* ASTRONODE_TRANSPORT_H */
