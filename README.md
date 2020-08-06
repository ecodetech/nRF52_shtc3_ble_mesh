# nRF52_shtc3_ble_mesh
 NRF5 mesh app for shtc3 reading

### Hardware used: Two nRF52840 development kits ###
### Mesh SDK: 4.0 ###
###nRF5 SDK: 16.0 ###

Using simple onOff vendor model a model is created for SHTC3 sensor.

SHTC3 sensor is connected to Server device flashed with Server firmware
Another is flashed with Client firmware.

On server side shtc3 value is read per second

Client reads server status periodically and receives values
