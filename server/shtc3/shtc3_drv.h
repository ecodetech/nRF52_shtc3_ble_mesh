
#ifndef SHTC3_DRV_H
#define SHTC3_DRV_H

/*lint ++flb "Enter library region" */

#include "nrf_drv_twi.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  SHTC3_COMMAND_WAKE = 0x3517,
  SHTC3_COMMAND_SLEEP = 0xB098,

  SHTC3_COMMAND_SOFT_RST = 0x805D,

  SHTC3_COMMAND_READ_ID = 0xEFC8,
} SHTC3_Cmd_TypeDef;

typedef enum {
  SHTC3_CS_RRHF_NM = 0x5C24, // Clock stretching Enabled; RelHumid first; Normal power mode
  SHTC3_CS_RRHF_LP = 0x44DE, // Clock stretching Enabled; RelHumid first; Low power mode
  SHTC3_CS_RTF_NP = 0x7CA2,  // Clock stretching Enabled; Temp first; Normal power mode
  SHTC3_CS_RTF_LP = 0x6458,  // Clock stretching Enabled; Temp first; Low power mode

  SHTC3_CSD_RRHF_NP = 0x58E0, // DISBALED; RelHumid first; Normal power mode
  SHTC3_CSD_RRHF_LP = 0x401A, // DISBALED; RelHumid first; Low power mode
  SHTC3_CSD_RTF_NP = 0x7866,  // DISBALED; Temp first; Normal power mode
  SHTC3_CSD_RTF_LP = 0x609C   // DISBALED; Temp first; Low power mode
} SHTC3_MeasModes_TypeDef;

bool SHTC3_init();

void SHTC3_Command(SHTC3_Cmd_TypeDef cmd);                            // send normal commands

void SHTC3_setMode(SHTC3_MeasModes_TypeDef cmd); // send mesaurement setup cmds

bool SHTC3_verify_id(void); // verify if shtc3 present

void SHTC3_Sleep(void); // put shtc3 to sleep

void SHTC3_Wake(void); // wake up shtc3

void SHTC3_SoftReset(void); // soft reset shtc3 wait for 240 us after this

bool SHTC3_verify_CRC(uint16_t data, uint8_t cmp); // verify received data packet

uint16_t SHTC3_RawTemp();

uint16_t SHTC3_RawHumid();

float SHTC3_TempDegC(void);        // temperature in Degree Celsius

float SHTC3_TempDegF(void);        // temperature in Degree Fahrenheit

float SHTC3_RelHumidPercent(void); // humidy in percent

bool SHTC3_StartReadPRocess(void); //wait after execution of this Normal mode = 12.1ms in Low power mode = 0.8ms

/**
 *@}
 **/

/*lint --flb "Leave library region" */

#ifdef __cplusplus
}
#endif

#endif /* SHTC3_DRV_H */