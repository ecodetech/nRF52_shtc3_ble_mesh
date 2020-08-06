
#ifndef SHTC3_SENSOR_SERVER_H__
#define SHTC3_SENSOR_SERVER_H__

#include <stdint.h>
#include <stdbool.h>
#include "access.h"

/**
 * @defgroup SHTC3_SENSOR_SERVER shtc3 sensor Server
 * @ingroup SHTC3_SENSOR_MODEL
 * This module implements a vendor specific shtc3 sensor Server.
 * @{
 */

/** shtc3 sensor Server model ID. */
#define SHTC3_SENSOR_SERVER_MODEL_ID (0x0000)

/** Forward declaration. */
typedef struct __shtc3_sensor_server shtc3_sensor_server_t;

/**
 * Get callback type.
 * @param[in] p_self Pointer to the shtc3 sensor Server context structure.
 * @returns @c true if the state is On, @c false otherwise.
 */
typedef uint32_t (*shtc3_sensor_get_cb_t)(const shtc3_sensor_server_t * p_self);

/**
 * Set callback type.
 * @param[in] p_self Pointer to the shtc3 sensor Server context structure.
 * @param[in] on_off Desired state
 * @returns @c true if the current state is On, @c false otherwise.
 */
typedef uint32_t (*shtc3_sensor_set_cb_t)(const shtc3_sensor_server_t * p_self, uint32_t val);

/** shtc3 sensor Server state structure. */
struct __shtc3_sensor_server
{
    /** Model handle assigned to the server. */
    access_model_handle_t model_handle;
    /** Get callback. */
    shtc3_sensor_get_cb_t get_cb;
    /** Set callback. */
    shtc3_sensor_set_cb_t set_cb;
};

/**
 * Initializes the shtc3 sensor server.
 *
 * @note This function should only be called _once_.
 * @note The server handles the model allocation and adding.
 *
 * @param[in] p_server      shtc3 sensor Server structure pointer.
 * @param[in] element_index Element index to add the server model.
 *
 * @retval NRF_SUCCESS         Successfully added server.
 * @retval NRF_ERROR_NULL      NULL pointer supplied to function.
 * @retval NRF_ERROR_NO_MEM    No more memory available to allocate model.
 * @retval NRF_ERROR_FORBIDDEN Multiple model instances per element is not allowed.
 * @retval NRF_ERROR_NOT_FOUND Invalid element index.
 */
uint32_t shtc3_sensor_server_init(shtc3_sensor_server_t * p_server, uint16_t element_index);

/**
 * Publishes unsolicited status message.
 *
 * This API can be used to send unsolicited status messages to report updated state value as a result
 * of local action.
 *
 * @param[in]  p_server         shtc3 sensor Server structure pointer
 * @param[in]  value            Current on/off value to be published
 *
 * @retval NRF_SUCCESS              Successfully queued packet for transmission.
 * @retval NRF_ERROR_NULL           NULL pointer supplied to function.
 * @retval NRF_ERROR_NO_MEM         Not enough memory available for message.
 * @retval NRF_ERROR_NOT_FOUND      Invalid model handle or model not bound to element.
 * @retval NRF_ERROR_INVALID_ADDR   The element index is greater than the number of local unicast
 *                                  addresses stored by the @ref DEVICE_STATE_MANAGER.
 * @retval NRF_ERROR_INVALID_PARAM  Model not bound to appkey, publish address not set or wrong
 *                                  opcode format.
 * @retval NRF_ERROR_INVALID_LENGTH Attempted to send message larger than @ref ACCESS_MESSAGE_LENGTH_MAX.
 *
 */
uint32_t shtc3_sensor_server_status_publish(shtc3_sensor_server_t * p_server, uint32_t value);

/** @} end of SHTC3_SENSOR_SERVER */

#endif /* SHTC3_SENSOR_SERVER_H__ */
