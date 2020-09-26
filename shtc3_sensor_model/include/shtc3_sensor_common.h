#ifndef SHTC3_SENSOR_COMMON_H__
#define SHTC3_SENSOR_COMMON_H__

#include <stdint.h>
#include "access.h"

/**
 * @defgroup SHTC3_SENSOR_MODEL shtc3 sensor model
 * This model implements the message based interface required to
 * set the 1 bit value on the server.
 *
 * This implementation of a shtc3 sensor model can be used to switch things
 * on or off by manipulating a single on/off state. The intention of this model
 * is to have a simple example model that can be used as a baseline for constructing
 * your own model.
 *
 * Do not confuse the shtc3 sensor model with the Generic OnOff Model specified
 * in the Mesh Model Specification v1.0. The Generic OnOff Model provides additional
 * features such as control over when and for how long the transition between
 * the on/off state should be performed.
 *
 * @note When the server has a publish address set (as in the light switch example),
 * the server will publish its state to its publish address every time its state changes.
 *
 * For more information about creating models, see
 * @ref md_doc_libraries_how_to_models.
 *
 * Model Identification
 * @par
 * Company ID: @ref SHTC3_SENSOR_COMPANY_ID
 * @par
 * shtc3 sensor Client Model ID: @ref SHTC3_SENSOR_CLIENT_MODEL_ID
 * @par
 * shtc3 sensor Server Model ID: @ref SHTC3_SENSOR_SERVER_MODEL_ID
 *
 * List of supported messages:
 * @par
 * @copydoc SHTC3_SENSOR_OPCODE_SET
 * @par
 * @copydoc SHTC3_SENSOR_OPCODE_GET
 * @par
 * @copydoc SHTC3_SENSOR_OPCODE_SET_UNRELIABLE
 * @par
 * @copydoc SHTC3_SENSOR_OPCODE_STATUS
 *
 * @ingroup MESH_API_GROUP_VENDOR_MODELS
 * @{
 * @defgroup SHTC3_SENSOR_COMMON Common shtc3 sensor definitions
 * Types and definitions shared between the two shtc3 sensor models.
 * @{
 */

/*lint -align_max(push) -align_max(1) */

/** Vendor specific company ID for shtc3 sensor model */
#define SHTC3_SENSOR_COMPANY_ID    (0x0060)

/** shtc3 sensor opcodes. */
typedef enum
{
    SHTC3_SENSOR_OPCODE_SET = 0xC1,            /**< shtc3 sensor Acknowledged Set. */
    SHTC3_SENSOR_OPCODE_GET = 0xC2,            /**< shtc3 sensor Get. */
    SHTC3_SENSOR_OPCODE_SET_UNRELIABLE = 0xC3, /**< shtc3 sensor Set Unreliable. */
    SHTC3_SENSOR_OPCODE_STATUS = 0xC4          /**< shtc3 sensor Status. */
} shtc3_sensor_opcode_t;

/** Message format for the shtc3 sensor Set message. */
typedef struct __attribute((packed))
{
    uint32_t temperature; /**< State to set. */
    uint32_t humidity; /**< State to set. */
    uint8_t tid;    /**< Transaction number. */
} shtc3_sensor_msg_set_t;

/** Message format for th shtc3 sensor Set Unreliable message. */
typedef struct __attribute((packed))
{
    uint32_t temperature; /**< State to set. */
    uint32_t humidity; /**< State to set. */
    uint8_t tid;    /**< Transaction number. */
} shtc3_sensor_msg_set_unreliable_t;

/** Message format for the shtc3 sensor Status message. */
typedef struct __attribute((packed))
{
    uint32_t temperature; /**< Current state. */
    uint32_t humidity; /**< current humidity. */
} shtc3_sensor_msg_status_t;

/*lint -align_max(pop) */

/** @} end of SHTC3_SENSOR_COMMON_H__ */
/** @} end of SHTC3_SENSOR_COMMON_H__ */
#endif /* SHTC3_SENSOR_COMMON_H__ */
