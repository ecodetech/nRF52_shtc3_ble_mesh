#ifndef BLE_TH_SVC_H
#define BLE_TH_SVC_H

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

/**@brief   Macro for defining a ble_simple_service instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_TH_SVC_DEF(_name)                                                               \
static ble_th_svc_t _name;                                                                  \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     2,                                                     \
                     ble_th_svc_on_ble_evt, &_name)

// Simple service:                     E54B0001-67F5-479E-8711-B3B99198CE6C
//   Button 1 press characteristic:    E54B0002-67F5-479E-8711-B3B99198CE6C

// The bytes are stored in little-endian format, meaning the
// Least Significant Byte is stored first
// (reversed from the order they're displayed as)

// Base UUID: E54B0000-67F5-479E-8711-B3B99198CE6C
#define BLE_UUID_TH_SVC_BASE_UUID  {0x6C, 0xCE, 0x98, 0x91, 0xB9, 0xB3, 0x11, 0x87, 0x9E, 0x47, 0xF5, 0x67, 0x00, 0x00, 0x4B, 0xE5}

// Service & characteristics UUIDs
#define BLE_UUID_TH_SVC_UUID        0x0001
#define BLE_UUID_TEMP_INFO_CHAR_UUID        0x0002

typedef struct ble_th_svc_s ble_th_svc_t;

typedef enum
{
    BLE_TEMP_EVT_NOTIFICATION_ENABLED,
    BLE_TEMP_EVT_NOTIFICATION_DISABLED,
} ble_th_evt_type_t;

typedef struct
{
    ble_th_evt_type_t evt_type;
} ble_th_evt_t;

typedef void (*ble_th_evt_handler_t) (ble_th_svc_t * p_simple_service, ble_th_evt_t * p_evt);

typedef struct ble_th_svc_s
{
    uint16_t                         conn_handle;
    uint16_t                         service_handle;
    uint8_t                          uuid_type;
    ble_th_evt_handler_t         evt_handler;
    ble_gatts_char_handles_t         temp_char_handles;

} ble_th_svc_t;

uint32_t ble_th_svc_init(ble_th_svc_t * p_simple_service, ble_th_evt_handler_t app_evt_handler);

void ble_th_svc_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

void temp_characteristic_update(ble_th_svc_t * p_simple_service, float *button_action);

#endif /* BLE_TH_SVC_H */