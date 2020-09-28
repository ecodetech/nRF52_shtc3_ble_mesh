# nRF52_shtc3_ble_mesh
 NRF5 mesh app for shtc3 reading

### Hardware used: Two nRF52840 development kits ###
### Mesh SDK: 4.0 ###
### nRF5 SDK: 16.0 ###

Using simple onOff vendor model a model is created for SHTC3 sensor.

SHTC3 sensor is connected to Server device flashed with Server firmware
Another is flashed with Client firmware.

On server side shtc3 value is read per second

Client reads server status periodically and receives values


### Setup Segger Studio Environment
1. Open Segger Embedded Studio we installed.
2. Go to Tab **Tools->Building**, under **Build** look for **Global Macros**, insert the nRF5 SDK address to assign a variable to it. e.g.(*SDK_ROOT=C:/nRF5_SDK_16.0*)
3. Also add global macro for nRF5 Mesh SDK. e.g.(*NRF52_MESH=C:/nRF5_Mesh_4.0*)
4. Now we can use this variable globally in our projects.
