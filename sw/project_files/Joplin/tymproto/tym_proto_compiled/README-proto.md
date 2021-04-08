# Protocol Documentation
<a name="top"/>

## Table of Contents
* [ble_eq_tuning.proto](#ble_eq_tuning.proto)
 * [Data](#Proto.BleEqTuning.Data)
* [ble_light.proto](#ble_light.proto)
 * [Data](#Proto.BleLight.Data)
* [bt-mcu-ReqResp.proto](#bt-mcu-ReqResp.proto)
 * [ReqResp](#Proto.BtMcu.ReqResp)
* [bt-mcu.proto](#bt-mcu.proto)
 * [Event](#Proto.BtMcu.Event)
 * [Req](#Proto.BtMcu.Req)
 * [Resp](#Proto.BtMcu.Resp)
 * [Event.Type](#Proto.BtMcu.Event.Type)
* [bt_dev_info.proto](#bt_dev_info.proto)
 * [Data](#Proto.BtDevInfo.Data)
* [bt_state.proto](#bt_state.proto)
 * [BleState](#Proto.BtState.BleState)
 * [ConnState](#Proto.BtState.ConnState)
 * [PlayState](#Proto.BtState.PlayState)
* [mcu-bt-ReqResp.proto](#mcu-bt-ReqResp.proto)
 * [ReqResp](#Proto.McuBt.ReqResp)
* [mcu-bt.proto](#mcu-bt.proto)
 * [Event](#Proto.McuBt.Event)
 * [Req](#Proto.McuBt.Req)
 * [Resp](#Proto.McuBt.Resp)
 * [Event.Type](#Proto.McuBt.Event.Type)
* [tym-common.proto](#tym-common.proto)
 * [GenericResponse](#Proto.Tym.GenericResponse)
 * [GenericResponse.Status](#Proto.Tym.GenericResponse.Status)
* [tym-dsp.proto](#tym-dsp.proto)
 * [AbsoluteVolume](#Proto.TymDsp.AbsoluteVolume)
 * [Mute](#Proto.TymDsp.Mute)
 * [RelativeVolumeChange](#Proto.TymDsp.RelativeVolumeChange)
* [tym.proto](#tym.proto)
 * [BtMcuMessage](#Proto.Tym.BtMcuMessage)
 * [McuBtMessage](#Proto.Tym.McuBtMessage)
 * [PbMessage](#Proto.Tym.PbMessage)
* [Scalar Value Types](#scalar-value-types)

<a name="ble_eq_tuning.proto"/>
<p align="right"><a href="#top">Top</a></p>

## ble_eq_tuning.proto



<a name="Proto.BleEqTuning.Data"/>
### Data
bt device info data

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| eq | [bytes](#bytes) | required |  |






<a name="ble_light.proto"/>
<p align="right"><a href="#top">Top</a></p>

## ble_light.proto



<a name="Proto.BleLight.Data"/>
### Data
bt device info data

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| light | [bytes](#bytes) | required |  |






<a name="bt-mcu-ReqResp.proto"/>
<p align="right"><a href="#top">Top</a></p>

## bt-mcu-ReqResp.proto




<a name="Proto.BtMcu.ReqResp"/>
### ReqResp


| Name | Number | Description |
| ---- | ------ | ----------- |
| MCU_APPLICATION_IS_RUNNING | 0 | checks whether the Application is running right now./ has Generic response/ DONE if the application is running/ ERROR if the application is not running |
| DSP_VOLUME_FADE | 1 | Request volume fade/ Input parameter Dsp.AbsoluteVolume with fade_duration field set/ Response: Generic response. If successful, this response should be delivered upon volume fade is finished |
| BT_AUDIO_CUE_PLAY | 2 | Bluetooth request for playing audio cue |
| MCU_FIRMWARE_UPDATE | 3 |  |




<a name="bt-mcu.proto"/>
<p align="right"><a href="#top">Top</a></p>

## bt-mcu.proto



<a name="Proto.BtMcu.Event"/>
### Event
Event report from BT module to MCU (Local UI).

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [Event.Type](#Proto.BtMcu.Event.Type) | optional |  |
| data | [uint32](#uint32) | optional |  |
| connState | [ConnState](#Proto.BtState.ConnState) | optional |  |
| playState | [PlayState](#Proto.BtState.PlayState) | optional |  |
| bleState | [BleState](#Proto.BtState.BleState) | optional |  |
| absoluteVolume | [uint32](#uint32) | optional |  |
| volumeStep | [uint32](#uint32) | optional |  |
| source | [uint32](#uint32) | optional |  |
| eq | [Data](#Proto.BleEqTuning.Data) | optional |  |
| light | [Data](#Proto.BleLight.Data) | optional |  |


<a name="Proto.BtMcu.Req"/>
### Req


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [ReqResp](#Proto.BtMcu.ReqResp) | optional | this should be copied from the request to the response/ to be able to pair messages |
| data | [uint32](#uint32) | optional |  |


<a name="Proto.BtMcu.Resp"/>
### Resp


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [ReqResp](#Proto.McuBt.ReqResp) | optional | this should be copied from the request to the response/ to be able to pair messages |
| data | [uint32](#uint32) | optional |  |
| swVersion | [uint32](#uint32) | optional |  |
| btInfo | [Data](#Proto.BtDevInfo.Data) | optional |  |



<a name="Proto.BtMcu.Event.Type"/>
### Event.Type


| Name | Number | Description |
| ---- | ------ | ----------- |
| BOOTED | 0 | BT module has completed the boot sequence and is ready to receive messages from MCU |
| SYSTEM_STATUS_STANDBY | 1 | Event send when BT module enter source standby |
| SYSTEM_STATUS_ON | 2 | Event send when exiting standby (a source has started from standby) |
| SYSTEM_STATUS_BT_RESTART | 3 | BT module is going to restart, MCU will gracefully shut down DSP and amplifier and signal reboot on the local UI. |
| FACTORY_RESET_DONE | 4 | Event send when &gt;&gt;reset to factory&lt;&lt; is done/ When starting factory reset procedure, FACTORY_RESET_START is sent |
| SW_UPDATE_STARTED | 7 | Software update of the BT module has started, local UI must show software update in progress. BT module reboot is part/ of the update process, if new firmware is available for the MCU, firmware update starts after BT module reboot. |
| SW_UPDATE_FINISHED | 8 |  |
| SW_UPDATE_FAILED | 9 |  |
| COMFORT_TONE_START | 10 | BT module indicates to MCU when the comfort sound playback starts. MCU has to turn on the DSP in case the DSP is off. |
| COMFORT_TONE_DONE | 11 | BT module indicates to MCU when the comfort sound playback is finished. MCU can turn off the DSP in case it was off/ before the comfort sound playback. |
| VOLUME_CHANGED | 12 | associated data in field of type Dsp.AbsoluteVolume/ field fade_duration should be ignored. Use DSP_VOLUME_FADE request to fade volume |
| MUTE_CHANGED | 13 | associated data in field of type Dsp.Mute |
| FACTORY_RESET_START | 16 | Event send when &gt;&gt;reset to factory&lt;&lt; is initiated/ FACTORY_RESET_DONE is sent when factory reset procedure has finished |
| BT_PAIRING_ENABLED | 17 | Bluetooth pairing is active. BT module is in discoverable state(BT). |
| BT_PAIRING_DISABLED | 18 | Bluetooth pairing is inactive. BT module is NOT in discoverable state(BT). |
| BT_PAIRING_FAILED | 19 | Pair of Bluetooth accessory failed. |
| BT_PAIRING_SUCCEEDED | 20 | A Bluetooth accessory has been successfully paired. |
| BTLE_PAIRING_ENABLED | 21 | Bluetooth Low Energy pairing is active. BT module is in discoverable state(BTLE). |
| BTLE_PAIRING_DISABLED | 22 | Bluetooth Low Energy pairing is inactive. BT module is NOT in discoverable state(BTLE). |
| BTLE_PAIRING_FAILED | 23 | Pair of Bluetooth Low Energy accessory failed. |
| BTLE_PAIRING_SUCCEEDED | 24 | A Bluetooth Low Energy accessory has been successfully paired. |
| SYSTEM_STATUS_TURNING_ON | 29 | Event send when on state is requested |
| BT_CONNECTION_STATE | 30 | Event send when on state is requested |
| BT_A2DP_CONNECTED | 31 | Bluetooth A2DP connected |
| BT_A2DP_DISCONNECTED | 32 | Bluetooth A2DP disconnected |
| BT_AUDIO_CUE_PLAY_START | 33 | Bluetooth audio cue play start |
| BT_AUDIO_CUE_PLAY_STOP | 34 | Bluetooth audio cue play stop |
| SOURCE_CHANGED | 36 |  |
| EQ_SET | 37 |  |
| LIGHT_SET | 38 |  |




<a name="bt_dev_info.proto"/>
<p align="right"><a href="#top">Top</a></p>

## bt_dev_info.proto



<a name="Proto.BtDevInfo.Data"/>
### Data
bt device info data

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| btAddr | [bytes](#bytes) | optional |  |






<a name="bt_state.proto"/>
<p align="right"><a href="#top">Top</a></p>

## bt_state.proto




<a name="Proto.BtState.BleState"/>
### BleState


| Name | Number | Description |
| ---- | ------ | ----------- |
| IDLE | 0 |  |
| ADVERTISING | 1 |  |
| SCANNING | 2 |  |
| PERIPHERALS_CONNECTED | 3 |  |
| CENTRAL_CONNECTED | 4 |  |

<a name="Proto.BtState.ConnState"/>
### ConnState


| Name | Number | Description |
| ---- | ------ | ----------- |
| CONNECTABLE | 0 |  |
| PAIRING | 1 |  |
| RECONNECTING | 2 |  |
| CONNECTED | 3 |  |
| A2DPSTREAMING | 4 |  |

<a name="Proto.BtState.PlayState"/>
### PlayState


| Name | Number | Description |
| ---- | ------ | ----------- |
| AVRCP_STOP | 0 |  |
| AVRCP_PLAYING | 1 |  |
| AVRCP_PAUSED | 2 |  |
| AVRCP_FWD_SEEK | 3 |  |
| AVRCP_REV_SEEK | 4 |  |
| AVRCP_ERROR | 5 |  |




<a name="mcu-bt-ReqResp.proto"/>
<p align="right"><a href="#top">Top</a></p>

## mcu-bt-ReqResp.proto




<a name="Proto.McuBt.ReqResp"/>
### ReqResp


| Name | Number | Description |
| ---- | ------ | ----------- |
| PING | 0 | Generic response |
| PLAY | 1 | Generic response |
| PAUSE | 2 | Generic response |
| STOP | 3 | Generic response |
| NEXT | 4 | Generic response |
| PREV | 5 | Generic response |
| NEXT_SOURCE | 6 | Generic response |
| JOIN | 7 | Generic response |
| PLAY_PAUSE_TOGGLE | 8 | Generic response |
| MUTE | 12 | Generic response |
| UNMUTE | 13 | Generic response |
| BT_PAIRING_ON | 14 | Generic response |
| BT_PAIRING_OFF | 15 | Generic response |
| BTLE_PAIRING_ON | 16 | Generic response |
| BTLE_PAIRING_OFF | 17 | Generic response |
| BT_PAIRING_TOGGLE | 18 | Generic response |
| TWS_PAIRING_TOGGLE | 19 | Generic response |
| OFF | 21 | Generic response. When the request is done, BT module is ready to be shut down/ STORAGE and OFF commands make the product turn off/ When error is set, shutdown is not possible. |
| FACTORY_RESET | 22 | Generic response. When the request is done, factory reset has started |
| STANDBY | 23 | Generic response. When the request is done, ASE is put to standby |
| BT_SW_VERSION | 24 | Get bluetooth software version |
| BT_LOCAL_ADDR | 25 | Get bluetooth local address |
| BT_CONNECT_TO_ADDR | 26 | request bluetooth connect to device with specific address |
| BT_RECONNECT_ON | 27 | reconnect on. |
| BT_RECONNECT_OFF | 28 | reconnect off. |
| FAST_FORWARD_START | 29 | fast forward start |
| FAST_FORWARD_STOP | 30 | fast forward stop |
| FAST_BACKWARD_START | 31 | fast backward start |
| FAST_BACKWARD_STOP | 32 | fast backward stop |
| BT_TEST_CUE | 33 | test 1KHz cue |




<a name="mcu-bt.proto"/>
<p align="right"><a href="#top">Top</a></p>

## mcu-bt.proto



<a name="Proto.McuBt.Event"/>
### Event
Event report from MCU to BT module (Local UI).

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [Event.Type](#Proto.McuBt.Event.Type) | optional |  |
| data | [uint32](#uint32) | optional |  |
| absoluteVolume | [uint32](#uint32) | optional |  |
| volumeStep | [uint32](#uint32) | optional |  |
| source | [uint32](#uint32) | optional |  |
| eq | [Data](#Proto.BleEqTuning.Data) | optional |  |
| light | [Data](#Proto.BleLight.Data) | optional |  |


<a name="Proto.McuBt.Req"/>
### Req


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [ReqResp](#Proto.McuBt.ReqResp) | optional | this should be copied from the request to the response/ to be able to pair messages |
| data | [uint32](#uint32) | optional |  |


<a name="Proto.McuBt.Resp"/>
### Resp


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [ReqResp](#Proto.BtMcu.ReqResp) | optional | this should be copied from the request to the response/ to be able to pair messages |
| data | [uint32](#uint32) | optional |  |
| audioCuePlayReady | [bool](#bool) | optional |  |



<a name="Proto.McuBt.Event.Type"/>
### Event.Type


| Name | Number | Description |
| ---- | ------ | ----------- |
| LINE_IN_SENSE_ACTIVE | 0 |  |
| LINE_IN_SENSE_INACTIVE | 1 |  |
| BOOTLOADER_READY | 2 |  |
| FIRMWARE_UPDATE_FINISHED | 3 |  |
| SYSTEM_BOOTED | 4 |  |
| VOLUME_CHANGED | 5 |  |
| SYSTEM_OFF | 6 |  |
| SYSTEM_STANDBY | 7 |  |
| SOURCE_CHANGED | 8 |  |
| EQ_CHANGED | 9 |  |
| LIGHT_CHANGED | 10 |  |




<a name="tym-common.proto"/>
<p align="right"><a href="#top">Top</a></p>

## tym-common.proto



<a name="Proto.Tym.GenericResponse"/>
### GenericResponse


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| status | [GenericResponse.Status](#Proto.Tym.GenericResponse.Status) | optional |  |



<a name="Proto.Tym.GenericResponse.Status"/>
### GenericResponse.Status


| Name | Number | Description |
| ---- | ------ | ----------- |
| DONE | 0 | This is sent when the request was handled |
| ERROR | 1 | This is sent when the request finished with error |
| NOT_SUPPORTED | 2 | This is sent when the request is not handled/ This is not meant to be used when reporting statuses from the user's handler |
| UNKNOWN_REQUEST | 3 | This is sent when the request cannot be parsed from the message/ This is not meant to be used when reporting statuses from the user's handler |




<a name="tym-dsp.proto"/>
<p align="right"><a href="#top">Top</a></p>

## tym-dsp.proto



<a name="Proto.TymDsp.AbsoluteVolume"/>
### AbsoluteVolume


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| volume | [uint32](#uint32) | optional |  |
| fade_duration | [uint32](#uint32) | optional | In milliseconds the duration of the fade operation |


<a name="Proto.TymDsp.Mute"/>
### Mute


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| mute | [bool](#bool) | optional |  |


<a name="Proto.TymDsp.RelativeVolumeChange"/>
### RelativeVolumeChange


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| volumeChange | [int32](#int32) | optional |  |






<a name="tym.proto"/>
<p align="right"><a href="#top">Top</a></p>

## tym.proto



<a name="Proto.Tym.BtMcuMessage"/>
### BtMcuMessage


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| btMcuEvent | [Event](#Proto.BtMcu.Event) | optional |  |
| btMcuReq | [Req](#Proto.BtMcu.Req) | optional |  |
| btMcuResp | [Resp](#Proto.BtMcu.Resp) | optional |  |


<a name="Proto.Tym.McuBtMessage"/>
### McuBtMessage


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| mcuBtEvent | [Event](#Proto.McuBt.Event) | optional |  |
| mcuBtReq | [Req](#Proto.McuBt.Req) | optional |  |
| mcuBtResp | [Resp](#Proto.McuBt.Resp) | optional |  |


<a name="Proto.Tym.PbMessage"/>
### PbMessage


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| btMcuMsg | [BtMcuMessage](#Proto.Tym.BtMcuMessage) | optional |  |
| mcuBtMsg | [McuBtMessage](#Proto.Tym.McuBtMessage) | optional |  |







<a name="scalar-value-types"/>
## Scalar Value Types

| .proto Type | Notes | C++ Type | Java Type | Python Type |
| ----------- | ----- | -------- | --------- | ----------- |
| <a name="double"/> double |  | double | double | float |
| <a name="float"/> float |  | float | float | float |
| <a name="int32"/> int32 | Uses variable-length encoding. Inefficient for encoding negative numbers – if your field is likely to have negative values, use sint32 instead. | int32 | int | int |
| <a name="int64"/> int64 | Uses variable-length encoding. Inefficient for encoding negative numbers – if your field is likely to have negative values, use sint64 instead. | int64 | long | int/long |
| <a name="uint32"/> uint32 | Uses variable-length encoding. | uint32 | int | int/long |
| <a name="uint64"/> uint64 | Uses variable-length encoding. | uint64 | long | int/long |
| <a name="sint32"/> sint32 | Uses variable-length encoding. Signed int value. These more efficiently encode negative numbers than regular int32s. | int32 | int | int |
| <a name="sint64"/> sint64 | Uses variable-length encoding. Signed int value. These more efficiently encode negative numbers than regular int64s. | int64 | long | int/long |
| <a name="fixed32"/> fixed32 | Always four bytes. More efficient than uint32 if values are often greater than 2^28. | uint32 | int | int |
| <a name="fixed64"/> fixed64 | Always eight bytes. More efficient than uint64 if values are often greater than 2^56. | uint64 | long | int/long |
| <a name="sfixed32"/> sfixed32 | Always four bytes. | int32 | int | int |
| <a name="sfixed64"/> sfixed64 | Always eight bytes. | int64 | long | int/long |
| <a name="bool"/> bool |  | bool | boolean | boolean |
| <a name="string"/> string | A string must always contain UTF-8 encoded or 7-bit ASCII text. | string | String | str/unicode |
| <a name="bytes"/> bytes | May contain any arbitrary sequence of bytes. | string | ByteString | str |
