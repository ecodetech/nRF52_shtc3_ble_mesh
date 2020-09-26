#include "shtc3_sensor_client.h"
#include "shtc3_sensor_common.h"

#include <stdint.h>
#include <stddef.h>

#include "access.h"
#include "access_config.h"
#include "access_reliable.h"
#include "device_state_manager.h"
#include "nrf_mesh.h"
#include "nrf_mesh_assert.h"
#include "log.h"

/*****************************************************************************
 * Static variables
 *****************************************************************************/

/** Keeps a single global TID for all transfers. */
static uint8_t m_tid;

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void reliable_status_cb(access_model_handle_t model_handle,
                               void * p_args,
                               access_reliable_status_t status)
{
    shtc3_sensor_client_t * p_client = p_args;
    NRF_MESH_ASSERT(p_client->status_cb != NULL);
shtc3_sensor_status_t stat;
    p_client->state.reliable_transfer_active = false;

    switch (status)
    {
        case ACCESS_RELIABLE_TRANSFER_SUCCESS:
            /* Ignore */
            break;
        case ACCESS_RELIABLE_TRANSFER_TIMEOUT:
            p_client->status_cb(p_client, stat, NRF_MESH_ADDR_UNASSIGNED);
            break;
        case ACCESS_RELIABLE_TRANSFER_CANCELLED:
            p_client->status_cb(p_client, stat, NRF_MESH_ADDR_UNASSIGNED);
            break;
        default:
            /* Should not be possible. */
            NRF_MESH_ASSERT(false);
            break;
    }
}

static uint32_t send_reliable_message(const shtc3_sensor_client_t * p_client,
                                      shtc3_sensor_opcode_t opcode,
                                      const uint8_t * p_data,
                                      uint16_t length)
{
    access_reliable_t reliable;
    reliable.model_handle = p_client->model_handle;
    reliable.message.p_buffer = p_data;
    reliable.message.length = length;
    reliable.message.opcode.opcode = opcode;
    reliable.message.opcode.company_id = SHTC3_SENSOR_COMPANY_ID;
    reliable.message.force_segmented = false;
    reliable.message.transmic_size = NRF_MESH_TRANSMIC_SIZE_DEFAULT;
    reliable.message.access_token = nrf_mesh_unique_token_get();
    reliable.reply_opcode.opcode = SHTC3_SENSOR_OPCODE_STATUS;
    reliable.reply_opcode.company_id = SHTC3_SENSOR_COMPANY_ID;
    reliable.timeout = SHTC3_SENSOR_CLIENT_ACKED_TRANSACTION_TIMEOUT;
    reliable.status_cb = reliable_status_cb;

    return access_model_reliable_publish(&reliable);
}

/*****************************************************************************
 * Opcode handler callback(s)
 *****************************************************************************/

static void handle_status_cb(access_model_handle_t handle, const access_message_rx_t * p_message, void * p_args)
{
    shtc3_sensor_client_t * p_client = p_args;
    NRF_MESH_ASSERT(p_client->status_cb != NULL);
    shtc3_sensor_msg_status_t * p_status =
        (shtc3_sensor_msg_status_t *) p_message->p_data;
    
    shtc3_sensor_status_t stat;
    stat.temperature = p_status->temperature;
    stat.humidity = p_status->humidity;
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "GET completed RSSI: %ddB\n", p_message->meta_data.p_core_metadata->params.scanner.rssi);
    p_client->status_cb(p_client, stat, p_message->meta_data.src.value);
}

static const access_opcode_handler_t m_opcode_handlers[] =
{
    {ACCESS_OPCODE_VENDOR(SHTC3_SENSOR_OPCODE_STATUS, SHTC3_SENSOR_COMPANY_ID), handle_status_cb}
};

static void handle_publish_timeout(access_model_handle_t handle, void * p_args)
{
    shtc3_sensor_client_t * p_client = p_args;

    if (p_client->timeout_cb != NULL)
    {
        p_client->timeout_cb(handle, p_args);
    }
}
/*****************************************************************************
 * Public API
 *****************************************************************************/

uint32_t shtc3_sensor_client_init(shtc3_sensor_client_t * p_client, uint16_t element_index)
{
    if (p_client == NULL ||
        p_client->status_cb == NULL)
    {
        return NRF_ERROR_NULL;
    }

    access_model_add_params_t init_params;
    init_params.model_id.model_id = SHTC3_SENSOR_CLIENT_MODEL_ID;
    init_params.model_id.company_id = SHTC3_SENSOR_COMPANY_ID;
    init_params.element_index = element_index;
    init_params.p_opcode_handlers = &m_opcode_handlers[0];
    init_params.opcode_count = sizeof(m_opcode_handlers) / sizeof(m_opcode_handlers[0]);
    init_params.p_args = p_client;
    init_params.publish_timeout_cb = handle_publish_timeout;
    uint32_t status =  access_model_add(&init_params, &p_client->model_handle);
    return  status;

}

uint32_t shtc3_sensor_client_set(shtc3_sensor_client_t * p_client, shtc3_sensor_status_t status_val)
{
    if (p_client == NULL || p_client->status_cb == NULL)
    {
        return NRF_ERROR_NULL;
    }
    else if (p_client->state.reliable_transfer_active)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    p_client->state.data.temperature = status_val.temperature ;
    p_client->state.data.humidity = status_val.humidity ;
    p_client->state.data.tid = m_tid++;

    uint32_t status = send_reliable_message(p_client,
                                            SHTC3_SENSOR_OPCODE_SET,
                                            (const uint8_t *)&p_client->state.data,
                                            sizeof(shtc3_sensor_msg_set_t));
    if (status == NRF_SUCCESS)
    {
        p_client->state.reliable_transfer_active = true;
    }
    return status;
}

uint32_t shtc3_sensor_client_set_unreliable(shtc3_sensor_client_t * p_client, shtc3_sensor_status_t status_val, uint8_t repeats)
{
    shtc3_sensor_msg_set_unreliable_t set_unreliable;
    set_unreliable.temperature = status_val.temperature;
    set_unreliable.humidity = status_val.humidity;
    set_unreliable.tid = m_tid++;

    access_message_tx_t message;
    message.opcode.opcode = SHTC3_SENSOR_OPCODE_SET_UNRELIABLE;
    message.opcode.company_id = SHTC3_SENSOR_COMPANY_ID;
    message.p_buffer = (const uint8_t*) &set_unreliable;
    message.length = sizeof(set_unreliable);
    message.force_segmented = false;
    message.transmic_size = NRF_MESH_TRANSMIC_SIZE_DEFAULT;

    uint32_t status = NRF_SUCCESS;
    for (uint8_t i = 0; i < repeats; ++i)
    {
        message.access_token = nrf_mesh_unique_token_get();
        status = access_model_publish(p_client->model_handle, &message);
        if (status != NRF_SUCCESS)
        {
            break;
        }
    }
    return status;
}

uint32_t shtc3_sensor_client_get(shtc3_sensor_client_t * p_client)
{
    if (p_client == NULL || p_client->status_cb == NULL)
    {
        return NRF_ERROR_NULL;
    }
    else if (p_client->state.reliable_transfer_active)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    uint32_t status = send_reliable_message(p_client,
                                            SHTC3_SENSOR_OPCODE_GET,
                                            NULL,
                                            0);
    if (status == NRF_SUCCESS)
    {
        p_client->state.reliable_transfer_active = true;
    }
    return status;
}

/**
 * Cancel any ongoing reliable message transfer.
 *
 * @param[in] p_client Pointer to the client instance structure.
 */
void shtc3_sensor_client_pending_msg_cancel(shtc3_sensor_client_t * p_client)
{
    (void)access_model_reliable_cancel(p_client->model_handle);
}
