#ifndef SHTC3_SENSOR_CLIENT_H__
#define SHTC3_SENSOR_CLIENT_H__

#include <stdint.h>
#include "access.h"
#include "shtc3_sensor_common.h"

/**
 * @defgroup SHTC3_SENSOR_CLIENT Simple OnOff Client
 * @ingroup SHTC3_SENSOR_MODEL
 * This module implements a vendor specific Simple OnOff Client.
 *
 * @{
 */

/** Acknowledged message transaction timeout */
#ifndef SHTC3_SENSOR_CLIENT_ACKED_TRANSACTION_TIMEOUT
#define SHTC3_SENSOR_CLIENT_ACKED_TRANSACTION_TIMEOUT  (SEC_TO_US(60))
#endif

/** Simple OnOff Client model ID. */
#define SHTC3_SENSOR_CLIENT_MODEL_ID (0x0001)

/** Simple OnOff status codes. */
typedef struct
{
    uint32_t temperature;
} shtc3_sensor_status_t;

/** Forward declaration. */
typedef struct __shtc3_sensor_client shtc3_sensor_client_t;

/**
 * Simple OnOff status callback type.
 *
 * @param[in] p_self Pointer to the Simple OnOff client structure that received the status.
 * @param[in] status The received status of the remote server.
 * @param[in] src    Element address of the remote server.
 */
typedef void (*shtc3_sensor_status_cb_t)(const shtc3_sensor_client_t * p_self, uint32_t status, uint16_t src);

/**
 * Simple OnOff timeout callback type.
 *
 * @param[in] handle Model handle
 * @param[in] p_self Pointer to the Simple OnOff client structure that received the status.
 */
typedef void (*shtc3_sensor_timeout_cb_t)(access_model_handle_t handle, void * p_self);

/** Simple OnOff Client state structure. */
struct __shtc3_sensor_client
{
    /** Model handle assigned to the client. */
    access_model_handle_t model_handle;
    /** Status callback called after status received from server. */
    shtc3_sensor_status_cb_t status_cb;
    /** Periodic timer timeout callback used for periodic publication. */
    shtc3_sensor_timeout_cb_t timeout_cb;
    /** Internal client state. */
    struct
    {
        bool reliable_transfer_active; /**< Variable used to determine if a transfer is currently active. */
        shtc3_sensor_msg_set_t data;  /**< Variable reflecting the data stored in the server. */
    } state;
};

/**
 * Initializes the Simple OnOff client.
 *
 * @note This function should only be called _once_.
 * @note The client handles the model allocation and adding.
 *
 * @param[in,out] p_client      Simple OnOff Client structure pointer.
 * @param[in]     element_index Element index to add the server model.
 *
 * @retval NRF_SUCCESS         Successfully added client.
 * @retval NRF_ERROR_NULL      NULL pointer supplied to function.
 * @retval NRF_ERROR_NO_MEM    No more memory available to allocate model.
 * @retval NRF_ERROR_FORBIDDEN Multiple model instances per element is not allowed.
 * @retval NRF_ERROR_NOT_FOUND Invalid element index.
 */
uint32_t shtc3_sensor_client_init(shtc3_sensor_client_t * p_client, uint16_t element_index);

/**
 * Sets the state of the Simple OnOff server.
 *
 * @param[in,out] p_client Simple OnOff Client structure pointer.
 * @param[in]     on_off   Value to set the Simple OnOff Server state to.
 *
 * @retval NRF_SUCCESS              Successfully sent message.
 * @retval NRF_ERROR_NULL           NULL pointer in function arguments
 * @retval NRF_ERROR_NO_MEM         Not enough memory available for message.
 * @retval NRF_ERROR_NOT_FOUND      Invalid model handle or model not bound to element.
 * @retval NRF_ERROR_INVALID_ADDR   The element index is greater than the number of local unicast
 *                                  addresses stored by the @ref DEVICE_STATE_MANAGER.
 * @retval NRF_ERROR_INVALID_STATE  Message already scheduled for a reliable transfer.
 * @retval NRF_ERROR_INVALID_PARAM  Model not bound to appkey, publish address not set or wrong
 *                                  opcode format.
 */
uint32_t shtc3_sensor_client_set(shtc3_sensor_client_t * p_client, uint32_t on_off);

/**
 * Sets the state of the Simple OnOff Server unreliably (without acknowledgment).
 *
 * @param[in,out] p_client Simple OnOff Client structure pointer.
 * @param[in]     on_off   Value to set the Simple OnOff Server state to.
 * @param[in]     repeats  Number of messages to send in a single burst. Increasing the number may
 *                     increase probability of successful delivery.
 *
 * @retval NRF_SUCCESS              Successfully sent message.
 * @retval NRF_ERROR_NULL           NULL pointer in function arguments
 * @retval NRF_ERROR_NO_MEM         Not enough memory available for message.
 * @retval NRF_ERROR_NOT_FOUND      Invalid model handle or model not bound to element.
 * @retval NRF_ERROR_INVALID_ADDR   The element index is greater than the number of local unicast
 *                                  addresses stored by the @ref DEVICE_STATE_MANAGER.
 * @retval NRF_ERROR_INVALID_PARAM  Model not bound to appkey, publish address not set or wrong
 *                                  opcode format.
 */
uint32_t shtc3_sensor_client_set_unreliable(shtc3_sensor_client_t * p_client, uint32_t on_off, uint8_t repeats);

/**
 * Gets the state of the Simple OnOff server.
 *
 * @note The state of the server will be given in the @ref shtc3_sensor_status_cb_t callback.
 *
 * @param[in,out] p_client Simple OnOff Client structure pointer.
 *
 * @retval NRF_SUCCESS              Successfully sent message.
 * @retval NRF_ERROR_NULL           NULL pointer in function arguments
 * @retval NRF_ERROR_NO_MEM         Not enough memory available for message.
 * @retval NRF_ERROR_NOT_FOUND      Invalid model handle or model not bound to element.
 * @retval NRF_ERROR_INVALID_ADDR   The element index is greater than the number of local unicast
 *                                  addresses stored by the @ref DEVICE_STATE_MANAGER.
 * @retval NRF_ERROR_INVALID_STATE  Message already scheduled for a reliable transfer.
 * @retval NRF_ERROR_INVALID_PARAM  Model not bound to appkey, publish address not set or wrong
 *                                  opcode format.
 */
uint32_t shtc3_sensor_client_get(shtc3_sensor_client_t * p_client);

/**
 * Cancel any ongoing reliable message transfer.
 *
 * @param[in,out] p_client Pointer to the client instance structure.
 */
void shtc3_sensor_client_pending_msg_cancel(shtc3_sensor_client_t * p_client);



/** @} end of SHTC3_SENSOR_CLIENT */

#endif /* SHTC3_SENSOR_CLIENT_H__ */
