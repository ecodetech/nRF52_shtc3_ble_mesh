#include <stdint.h>
#include <string.h>

/* HAL */
#include "app_timer.h"
#include "boards.h"
#include "simple_hal.h"

/* Core */
#include "access_config.h"
#include "device_state_manager.h"
#include "mesh_stack.h"
#include "nrf_mesh.h"
#include "nrf_mesh_config_core.h"
#include "nrf_mesh_configure.h"
#include "nrf_mesh_gatt.h"
#include "proxy.h"

/* Provisioning and configuration */
#include "mesh_app_utils.h"
#include "mesh_provisionee.h"

/* Models */
#include "generic_onoff_server.h"
#include "shtc3_sensor_server.h"

/* Logging and RTT */
#include "log.h"
#include "rtt_input.h"

/* Example specific includes */
#include "app_config.h"
#include "app_onoff.h"
#include "ble_softdevice_support.h"
#include "example_common.h"
#include "light_switch_example_common.h"
#include "nrf_mesh_config_examples.h"

/* shtc3 driver includes and init*/
#include "app_timer.h"
#include "shtc3_drv.h"

#define ONOFF_SERVER_0_LED (BSP_LED_0)
#define APP_ONOFF_ELEMENT_INDEX (0)

/* TWI instance ID. */
#define TWI_INSTANCE_ID 0
APP_TIMER_DEF(m_temp_timer_id); /**< Handler for repeated timer used to measure temp. */
/* Indicates if operation on TWI has ended. */
static volatile bool m_xfer_done = false;
/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const *p_event, void *p_context) {
  switch (p_event->type) {
  case NRF_DRV_TWI_EVT_DONE:
    if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX) {
      //data_handler(SHTC3_RawTemp());
    }
    m_xfer_done = true;
    break;
  default:
    break;
  }
}
/**
 * @brief twi initialization.
 */
void twi_init(void) {
  ret_code_t err_code;

  const nrf_drv_twi_config_t twi_shtc3_config = {
      .scl = ARDUINO_SCL_PIN,
      .sda = ARDUINO_SDA_PIN,
      .frequency = NRF_DRV_TWI_FREQ_100K,
      .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
      .clear_bus_init = false};

  err_code = nrf_drv_twi_init(&m_twi, &twi_shtc3_config, twi_handler, NULL);
  APP_ERROR_CHECK(err_code);

  nrf_drv_twi_enable(&m_twi);
}
/* state machine for temperature read
*/
static void temp_sensor_process(void) {
  static enum {
    SHTC3_STATE_IDLE,
    SHTC3_STATE_WAKE,
    SHTC3_STATE_MODE,
    SHTC3_STATE_START_READ,
    SHTC3_STATE_PROCESS_READ
  } Shtc3_Read_State = SHTC3_STATE_IDLE;
  float temp_val;
  switch (Shtc3_Read_State) {
  case SHTC3_STATE_IDLE:
    m_xfer_done = false;
    Shtc3_Read_State = SHTC3_STATE_WAKE;
    break;
  case SHTC3_STATE_WAKE:
    SHTC3_Wake(); // wake up sensor
    Shtc3_Read_State = SHTC3_STATE_MODE;
    break;
  case SHTC3_STATE_MODE:
    if (m_xfer_done == false) {
      return;
    }
    m_xfer_done = false;
    SHTC3_setMode(SHTC3_CS_RTF_NP); // enable clock stretching, read temp first in normal power mode
    Shtc3_Read_State = SHTC3_STATE_START_READ;
    break;
  case SHTC3_STATE_START_READ:
    if (m_xfer_done == false) {
      return;
    }
    m_xfer_done = false;
    SHTC3_StartReadPRocess(); // start reading temp and humidity
    Shtc3_Read_State = SHTC3_STATE_PROCESS_READ;
    break;
  case SHTC3_STATE_PROCESS_READ:
    if (m_xfer_done == false) {
      return;
    }
    m_xfer_done = false;
    temp_val = SHTC3_TempDegC();
    //temp_characteristic_update(&m_thservice, &temp_val); //update value in service temp service
    Shtc3_Read_State = SHTC3_STATE_IDLE;
    break;
  default:
    Shtc3_Read_State = SHTC3_STATE_IDLE;
    break;
  }
}

static void temp_read_timeout_handler() {
  // start read process again
  temp_sensor_process();
}
/**@brief Function for starting timers.
 */
static void application_timers_start(void) {
  /* YOUR_JOB: Start your timers. below is an example of how to start a timer.*/
  ret_code_t err_code;
  err_code = app_timer_start(m_temp_timer_id, APP_TIMER_TICKS(200), NULL);
  APP_ERROR_CHECK(err_code);
}
/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void) {
  // Initialize timer module, making it use the scheduler
  ret_code_t err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);

  // Create timers
  err_code = app_timer_create(&m_temp_timer_id,
      APP_TIMER_MODE_REPEATED,
      temp_read_timeout_handler);
  APP_ERROR_CHECK(err_code);
}

static void temp_sensor_init() {
  SHTC3_init(&m_twi); //setup instance
}

static bool m_device_provisioned;

/*************************************************************************************************/
static void app_onoff_server_set_cb(const app_onoff_server_t *p_server, bool onoff);
static void app_onoff_server_get_cb(const app_onoff_server_t *p_server, bool *p_present_onoff);

/* Generic OnOff server structure definition and initialization */
APP_ONOFF_SERVER_DEF(m_onoff_server_0,
    APP_CONFIG_FORCE_SEGMENTATION,
    APP_CONFIG_MIC_SIZE,
    app_onoff_server_set_cb,
    app_onoff_server_get_cb)
/* shtc3 model vars*/
static shtc3_sensor_server_t m_shtc3_server;

static shtc3_sensor_msg_status_t shtc3_server_get_cb(const shtc3_sensor_server_t *p_server) {
  shtc3_sensor_msg_status_t status_val;
  status_val.temperature =  (uint32_t) SHTC3_TempDegC();
  status_val.humidity = (uint32_t)SHTC3_RelHumidPercent();
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Getting temp of sensor: 0x%08x\n", status_val.temperature);
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Getting humidity of sensor: %f\n", status_val.humidity);
  // add code here
  
  return status_val;
}

static shtc3_sensor_msg_status_t shtc3_server_set_cb(const shtc3_sensor_server_t *p_server, shtc3_sensor_msg_status_t value) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Setting value of sensor:0x%04x\n", value.temperature);
  return value;
}

/* Callback for updating the hardware state */
static void app_onoff_server_set_cb(const app_onoff_server_t *p_server, bool onoff) {
  /* Resolve the server instance here if required, this example uses only 1 instance. */

  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Setting GPIO value: %d\n", onoff);

  hal_led_pin_set(ONOFF_SERVER_0_LED, onoff);
}

/* Callback for reading the hardware state */
static void app_onoff_server_get_cb(const app_onoff_server_t *p_server, bool *p_present_onoff) {
  /* Resolve the server instance here if required, this example uses only 1 instance. */

  *p_present_onoff = hal_led_pin_get(ONOFF_SERVER_0_LED);
}

static void app_model_init(void) {
  /* Instantiate onoff server on element index APP_ONOFF_ELEMENT_INDEX */
  ERROR_CHECK(app_onoff_init(&m_onoff_server_0, APP_ONOFF_ELEMENT_INDEX));
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "App OnOff Model Handle: %d\n", m_onoff_server_0.server.model_handle);

  m_shtc3_server.get_cb = shtc3_server_get_cb;
  m_shtc3_server.set_cb = shtc3_server_set_cb;

  ERROR_CHECK(shtc3_sensor_server_init(&m_shtc3_server, 1));
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "SHTC3 Model Handle: %d\n", m_shtc3_server.model_handle);
  ERROR_CHECK(access_model_subscription_list_alloc(m_shtc3_server.model_handle));

}

/*************************************************************************************************/

static void node_reset(void) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- Node reset  -----\n");
  hal_led_blink_ms(LEDS_MASK, LED_BLINK_INTERVAL_MS, LED_BLINK_CNT_RESET);
  /* This function may return if there are ongoing flash operations. */
  mesh_stack_device_reset();
}

static void config_server_evt_cb(const config_server_evt_t *p_evt) {
  if (p_evt->type == CONFIG_SERVER_EVT_NODE_RESET) {
    node_reset();
  }
}

static void button_event_handler(uint32_t button_number) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Button %u pressed\n", button_number);

  switch (button_number) {
  /* Pressing SW1 on the Development Kit will result in LED state to toggle and trigger
        the STATUS message to inform client about the state change. This is a demonstration of
        state change publication due to local event. */
  case 0: {
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "User action \n");
    hal_led_pin_set(ONOFF_SERVER_0_LED, !hal_led_pin_get(ONOFF_SERVER_0_LED));
    app_onoff_status_publish(&m_onoff_server_0);
    break;
  }

  /* Initiate node reset */
  case 3: {
    /* Clear all the states to reset the node. */
    if (mesh_stack_is_device_provisioned()) {
#if MESH_FEATURE_GATT_PROXY_ENABLED
      (void)proxy_stop();
#endif
      mesh_stack_config_clear();
      node_reset();
    } else {
      __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "The device is unprovisioned. Resetting has no effect.\n");
    }
    break;
  }

  default:
    break;
  }
}

static void app_rtt_input_handler(int key) {
  if (key >= '0' && key <= '4') {
    uint32_t button_number = key - '0';
    button_event_handler(button_number);
  }
}

static void device_identification_start_cb(uint8_t attention_duration_s) {
  hal_led_mask_set(LEDS_MASK, false);
  hal_led_blink_ms(BSP_LED_2_MASK | BSP_LED_3_MASK,
      LED_BLINK_ATTENTION_INTERVAL_MS,
      LED_BLINK_ATTENTION_COUNT(attention_duration_s));
}

static void provisioning_aborted_cb(void) {
  hal_led_blink_stop();
}

static void provisioning_complete_cb(void) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Successfully provisioned\n");

#if MESH_FEATURE_GATT_ENABLED
  /* Restores the application parameters after switching from the Provisioning
     * service to the Proxy  */
  gap_params_init();
  conn_params_init();
#endif

  dsm_local_unicast_address_t node_address;
  dsm_local_unicast_addresses_get(&node_address);
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Node Address: 0x%04x \n", node_address.address_start);

  hal_led_blink_stop();
  hal_led_mask_set(LEDS_MASK, LED_MASK_STATE_OFF);
  hal_led_blink_ms(LEDS_MASK, LED_BLINK_INTERVAL_MS, LED_BLINK_CNT_PROV);
}

static void models_init_cb(void) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initializing and adding models\n");
  app_model_init();
}

static void mesh_init(void) {
  mesh_stack_init_params_t init_params =
      {
          .core.irq_priority = NRF_MESH_IRQ_PRIORITY_LOWEST,
          .core.lfclksrc = DEV_BOARD_LF_CLK_CFG,
          .core.p_uuid = NULL,
          .models.models_init_cb = models_init_cb,
          .models.config_server_cb = config_server_evt_cb};

  uint32_t status = mesh_stack_init(&init_params, &m_device_provisioned);
  switch (status) {
  case NRF_ERROR_INVALID_DATA:
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Data in the persistent memory was corrupted. Device starts as unprovisioned.\n");
    break;
  case NRF_SUCCESS:
    break;
  default:
    ERROR_CHECK(status);
  }
}

static void initialize(void) {
  __LOG_INIT(LOG_SRC_APP | LOG_SRC_FRIEND, LOG_LEVEL_DBG1, LOG_CALLBACK_DEFAULT);
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- BLE Mesh Light Switch Server Demo -----\n");

  ERROR_CHECK(app_timer_init());
  hal_leds_init();

#if BUTTON_BOARD
  ERROR_CHECK(hal_buttons_init(button_event_handler));
#endif

  ble_stack_init();

#if MESH_FEATURE_GATT_ENABLED
  gap_params_init();
  conn_params_init();
#endif

  mesh_init();
}

static void start(void) {
  rtt_input_enable(app_rtt_input_handler, RTT_INPUT_POLL_PERIOD_MS);

  if (!m_device_provisioned) {
    static const uint8_t static_auth_data[NRF_MESH_KEY_SIZE] = STATIC_AUTH_DATA;
    mesh_provisionee_start_params_t prov_start_params =
        {
            .p_static_data = static_auth_data,
            .prov_complete_cb = provisioning_complete_cb,
            .prov_device_identification_start_cb = device_identification_start_cb,
            .prov_device_identification_stop_cb = NULL,
            .prov_abort_cb = provisioning_aborted_cb,
            .p_device_uri = EX_URI_LS_SERVER};
    ERROR_CHECK(mesh_provisionee_prov_start(&prov_start_params));
  }

  mesh_app_uuid_print(nrf_mesh_configure_device_uuid_get());

  ERROR_CHECK(mesh_stack_start());

  hal_led_mask_set(LEDS_MASK, LED_MASK_STATE_OFF);
  hal_led_blink_ms(LEDS_MASK, LED_BLINK_INTERVAL_MS, LED_BLINK_CNT_START);
}

int main(void) {
  twi_init(); //START I2C interface
  temp_sensor_init();
  timers_init();
  initialize();
  start();
  application_timers_start();
  for (;;) {
    (void)sd_app_evt_wait();
  }
}