#include "shtc3_sensor_server.h"
#include "shtc3_sensor_common.h"

#include <stdint.h>
#include <stddef.h>

#include "access.h"
#include "nrf_mesh_assert.h"
#include "log.h"

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void reply_status(const shtc3_sensor_server_t * p_server,
                         const access_message_rx_t * p_message,
                         const shtc3_sensor_msg_status_t status_val )
{
    access_message_tx_t reply;
    reply.opcode.opcode = SHTC3_SENSOR_OPCODE_STATUS;
    reply.opcode.company_id = SHTC3_SENSOR_COMPANY_ID;
    reply.p_buffer = (const uint8_t *) &status_val;
    reply.length = sizeof(status_val);
    reply.force_segmented = false;
    reply.transmic_size = NRF_MESH_TRANSMIC_SIZE_DEFAULT;
    reply.access_token = nrf_mesh_unique_token_get();

    (void) access_model_reply(p_server->model_handle, p_message, &reply);
}

/*****************************************************************************
 * Opcode handler callbacks
 *****************************************************************************/

static void handle_set_cb(access_model_handle_t handle, const access_message_rx_t * p_message, void * p_args)
{
    shtc3_sensor_server_t * p_server = p_args;
    NRF_MESH_ASSERT(p_server->set_cb != NULL);

    shtc3_sensor_msg_status_t status_val;
    status_val.temperature = (((shtc3_sensor_msg_set_t*) p_message->p_data)->temperature);
    status_val.humidity = (((shtc3_sensor_msg_set_t*) p_message->p_data)->humidity);
    status_val = p_server->set_cb(p_server, status_val);
    reply_status(p_server, p_message, status_val);
    (void) shtc3_sensor_server_status_publish(p_server, status_val); /* We don't care about status */
}

static void handle_get_cb(access_model_handle_t handle, const access_message_rx_t * p_message, void * p_args)
{
    shtc3_sensor_server_t * p_server = p_args;
    NRF_MESH_ASSERT(p_server->get_cb != NULL);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "GET Request received RSSI: %ddB\n", p_message->meta_data.p_core_metadata->params.scanner.rssi);
    reply_status(p_server, p_message, p_server->get_cb(p_server));
}

static void handle_set_unreliable_cb(access_model_handle_t handle, const access_message_rx_t * p_message, void * p_args)
{
    shtc3_sensor_server_t * p_server = p_args;
    NRF_MESH_ASSERT(p_server->set_cb != NULL);
    shtc3_sensor_msg_status_t status_val;
     status_val.temperature = (((shtc3_sensor_msg_set_unreliable_t*) p_message->p_data)->temperature);
     status_val.humidity = (((shtc3_sensor_msg_set_unreliable_t*) p_message->p_data)->humidity);
    status_val = p_server->set_cb(p_server, status_val);
    (void) shtc3_sensor_server_status_publish(p_server, status_val);
}

static const access_opcode_handler_t m_opcode_handlers[] =
{
    {ACCESS_OPCODE_VENDOR(SHTC3_SENSOR_OPCODE_SET,            SHTC3_SENSOR_COMPANY_ID), handle_set_cb},
    {ACCESS_OPCODE_VENDOR(SHTC3_SENSOR_OPCODE_GET,            SHTC3_SENSOR_COMPANY_ID), handle_get_cb},
    {ACCESS_OPCODE_VENDOR(SHTC3_SENSOR_OPCODE_SET_UNRELIABLE, SHTC3_SENSOR_COMPANY_ID), handle_set_unreliable_cb}
};

static void handle_publish_timeout(access_model_handle_t handle, void * p_args)
{
    shtc3_sensor_server_t * p_server = p_args;

    (void) shtc3_sensor_server_status_publish(p_server, p_server->get_cb(p_server));
}

/*****************************************************************************
 * Public API
 *****************************************************************************/

uint32_t shtc3_sensor_server_init(shtc3_sensor_server_t * p_server, uint16_t element_index)
{
    if (p_server == NULL ||
        p_server->get_cb == NULL ||
        p_server->set_cb == NULL)
    {
        return NRF_ERROR_NULL;
    }

    access_model_add_params_t init_params;
    init_params.element_index =  element_index;
    init_params.model_id.model_id = SHTC3_SENSOR_SERVER_MODEL_ID;
    init_params.model_id.company_id = SHTC3_SENSOR_COMPANY_ID;
    init_params.p_opcode_handlers = &m_opcode_handlers[0];
    init_params.opcode_count = sizeof(m_opcode_handlers) / sizeof(m_opcode_handlers[0]);
    init_params.p_args = p_server;
    init_params.publish_timeout_cb = handle_publish_timeout;
    return access_model_add(&init_params, &p_server->model_handle);
}

uint32_t shtc3_sensor_server_status_publish(shtc3_sensor_server_t * p_server, shtc3_sensor_msg_status_t status_val)
{
   
    access_message_tx_t msg;
    msg.opcode.opcode = SHTC3_SENSOR_OPCODE_STATUS;
    msg.opcode.company_id = SHTC3_SENSOR_COMPANY_ID;
    msg.p_buffer = (const uint8_t *) &status_val;
    msg.length = sizeof(status_val);
    msg.force_segmented = false;
    msg.transmic_size = NRF_MESH_TRANSMIC_SIZE_DEFAULT;
    msg.access_token = nrf_mesh_unique_token_get();
    return access_model_publish(p_server->model_handle, &msg);
}
