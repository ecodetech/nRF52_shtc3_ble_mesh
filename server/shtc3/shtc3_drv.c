
#include <stdbool.h>
#include <stdint.h>

#include "nrf_drv_twi.h"
#include "shtc3_drv.h"

/*lint ++flb "Enter library region" */

static uint8_t m_device_address = 0x70; // !< Device address in bits [7:1]
static nrf_drv_twi_t *m_twi;
static uint8_t regRead_Data[6] = {0, 0, 0, 0, 0, 0};

bool SHTC3_init(nrf_drv_twi_t *twi_drv_instance) {
  m_twi = twi_drv_instance;
//  uint8_t reg = 0x06;
//  ret_code_t err_code = nrf_drv_twi_tx(m_twi, 0x00, &reg, sizeof(reg), false);
//  APP_ERROR_CHECK(err_code);
}

void SHTC3_Command(uint16_t cmd) {
  uint8_t reg[2];
  reg[0] = (cmd >> 8);
  reg[1] = cmd & 0xff;
  ret_code_t err_code = nrf_drv_twi_tx(m_twi, m_device_address, reg, sizeof(reg), false);
  APP_ERROR_CHECK(err_code);
}

void SHTC3_setMode(SHTC3_MeasModes_TypeDef cmd) {
  SHTC3_Command((uint16_t)cmd);
}

bool SHTC3_verify_id(void) {
  uint16_t id_data = ((uint16_t)regRead_Data[0] << 8) | ((uint16_t)regRead_Data[1]);
  if (!SHTC3_verify_CRC(id_data, regRead_Data[2])) {
    return false;
  }

  if ((id_data & 0b0000100000111111) != 0b0000100000000111) { // Bits 11 and 5-0 must match
    return false;
  }
  return true;
}

void SHTC3_Sleep(void) {
  SHTC3_Command((uint16_t)SHTC3_COMMAND_SLEEP);
}

void SHTC3_Wake(void) {
  SHTC3_Command((uint16_t)SHTC3_COMMAND_WAKE);
}

void SHTC3_SoftReset(void) {
  SHTC3_Command((uint16_t)SHTC3_COMMAND_SOFT_RST);
}

bool SHTC3_verify_CRC(uint16_t data, uint8_t cmp) {
  uint8_t upper = data >> 8;
  uint8_t lower = data & 0xff;
  uint8_t dats_var[2] = {upper, lower};
  uint8_t crc = 0xFF;
  uint8_t poly = 0x31;

  for (uint8_t indi = 0; indi < 2; indi++) {
    crc ^= dats_var[indi];

    for (uint8_t indj = 0; indj < 8; indj++) {
      if (crc & 0x80) {
        crc = (uint8_t)((crc << 1) ^ poly);
      } else {
        crc <<= 1;
      }
    }
  }

  if (cmp ^ crc) {
    return false;
  }
  return true;
}

uint16_t SHTC3_RawTemp() {
  return ((uint16_t)regRead_Data[0] << 8) | regRead_Data[1];
}

uint16_t SHTC3_RawHumid() {
  return ((uint16_t)regRead_Data[3] << 8) | regRead_Data[4];
}

float SHTC3_TempDegC(void) {
  return (-45 + 175 * ((float)(SHTC3_RawTemp()) / 65535));
}

float SHTC3_TempDegF(void) {
  return (SHTC3_TempDegC() * (9.0 / 5) + 32.0);
}

float SHTC3_RelHumidPercent(void) {
return 100 * ((float)( SHTC3_RawHumid()) / 65535);
}

bool SHTC3_StartReadPRocess(void) {
  ret_code_t err_code = nrf_drv_twi_rx(m_twi, m_device_address, regRead_Data, 6);
  APP_ERROR_CHECK(err_code);
}