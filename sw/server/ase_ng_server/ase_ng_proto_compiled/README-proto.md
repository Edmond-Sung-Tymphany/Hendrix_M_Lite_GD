# Protocol Documentation
<a name="top"/>

## Table of Contents
* [ase-fep-ReqResp.proto](#ase-fep-ReqResp.proto)
 * [ReqResp](#Proto.AseFep.ReqResp)
* [ase-fep.proto](#ase-fep.proto)
 * [Event](#Proto.AseFep.Event)
 * [Req](#Proto.AseFep.Req)
 * [Resp](#Proto.AseFep.Resp)
 * [Event.Type](#Proto.AseFep.Event.Type)
* [common.proto](#common.proto)
 * [GenericResponse](#Proto.Core.GenericResponse)
 * [GenericResponse.Status](#Proto.Core.GenericResponse.Status)
* [core.proto](#core.proto)
 * [AseFepMessage](#Proto.Core.AseFepMessage)
 * [FepAseMessage](#Proto.Core.FepAseMessage)
* [dsp.proto](#dsp.proto)
 * [AbsoluteVolume](#Proto.Dsp.AbsoluteVolume)
 * [InternalAmplifierCommand](#Proto.Dsp.InternalAmplifierCommand)
 * [InternalSpeaker](#Proto.Dsp.InternalSpeaker)
 * [LineInSensitivity](#Proto.Dsp.LineInSensitivity)
 * [Mute](#Proto.Dsp.Mute)
 * [NTCDataEvent](#Proto.Dsp.NTCDataEvent)
 * [NTCDataEvent.NTCValue](#Proto.Dsp.NTCDataEvent.NTCValue)
 * [Parameter](#Proto.Dsp.Parameter)
 * [PositionSoundMode](#Proto.Dsp.PositionSoundMode)
 * [RelativeVolumeChange](#Proto.Dsp.RelativeVolumeChange)
 * [RequestAudioInput](#Proto.Dsp.RequestAudioInput)
 * [RequestInternalSpeakerCompensation](#Proto.Dsp.RequestInternalSpeakerCompensation)
 * [RequestPositionSoundMode](#Proto.Dsp.RequestPositionSoundMode)
 * [ResponseInternalSpeakerCompensation](#Proto.Dsp.ResponseInternalSpeakerCompensation)
 * [ResponsePositionSoundMode](#Proto.Dsp.ResponsePositionSoundMode)
 * [SpeakerEnableCommand](#Proto.Dsp.SpeakerEnableCommand)
 * [SpeakerEnableCommand.Speaker](#Proto.Dsp.SpeakerEnableCommand.Speaker)
 * [ToneTouch](#Proto.Dsp.ToneTouch)
 * [ToslinkOutSampleRate](#Proto.Dsp.ToslinkOutSampleRate)
 * [InternalAmplifierCommand.State](#Proto.Dsp.InternalAmplifierCommand.State)
 * [InternalSpeaker.Position](#Proto.Dsp.InternalSpeaker.Position)
 * [InternalSpeaker.Type](#Proto.Dsp.InternalSpeaker.Type)
 * [LineInSensitivity.Sensitivity](#Proto.Dsp.LineInSensitivity.Sensitivity)
 * [NTCDataEvent.NTCValue.NTC](#Proto.Dsp.NTCDataEvent.NTCValue.NTC)
 * [Parameter.Type](#Proto.Dsp.Parameter.Type)
 * [PositionSoundMode.Orientation](#Proto.Dsp.PositionSoundMode.Orientation)
 * [PositionSoundMode.Position](#Proto.Dsp.PositionSoundMode.Position)
 * [RequestAudioInput.AudioInput](#Proto.Dsp.RequestAudioInput.AudioInput)
 * [ResponseInternalSpeakerCompensation.Error](#Proto.Dsp.ResponseInternalSpeakerCompensation.Error)
 * [ResponsePositionSoundMode.Error](#Proto.Dsp.ResponsePositionSoundMode.Error)
 * [SpeakerEnableCommand.Speaker.Id](#Proto.Dsp.SpeakerEnableCommand.Speaker.Id)
* [eeb.proto](#eeb.proto)
 * [EebTelegram](#Proto.Eeb.EebTelegram)
 * [EebTelegram.PacketType](#Proto.Eeb.EebTelegram.PacketType)
* [fep-ase-ReqResp.proto](#fep-ase-ReqResp.proto)
 * [ReqResp](#Proto.FepAse.ReqResp)
* [fep-ase.proto](#fep-ase.proto)
 * [Event](#Proto.FepAse.Event)
 * [Req](#Proto.FepAse.Req)
 * [Resp](#Proto.FepAse.Resp)
 * [Event.Type](#Proto.FepAse.Event.Type)
* [firmware-update.proto](#firmware-update.proto)
 * [Chunk](#Proto.FirmwareUpdate.Chunk)
 * [ChunkResponse](#Proto.FirmwareUpdate.ChunkResponse)
 * [VersionInfo](#Proto.FirmwareUpdate.VersionInfo)
 * [VersionInfo.Module](#Proto.FirmwareUpdate.VersionInfo.Module)
 * [WplOption](#Proto.FirmwareUpdate.WplOption)
 * [Chunk.ModuleType](#Proto.FirmwareUpdate.Chunk.ModuleType)
 * [ChunkResponse.Status](#Proto.FirmwareUpdate.ChunkResponse.Status)
 * [WplOption.ID](#Proto.FirmwareUpdate.WplOption.ID)
 * [WplOption.ModuleID](#Proto.FirmwareUpdate.WplOption.ModuleID)
 * [WplOption.PartID](#Proto.FirmwareUpdate.WplOption.PartID)
* [hdmi.proto](#hdmi.proto)
 * [Arc](#Proto.Hdmi.Arc)
 * [AudioFormat](#Proto.Hdmi.AudioFormat)
 * [AudioModeSelect](#Proto.Hdmi.AudioModeSelect)
 * [HdmiInput](#Proto.Hdmi.HdmiInput)
 * [InputSelected](#Proto.Hdmi.InputSelected)
 * [InputSense](#Proto.Hdmi.InputSense)
 * [InputsSense](#Proto.Hdmi.InputsSense)
 * [InputsSense.SenseState](#Proto.Hdmi.InputsSense.SenseState)
 * [StandbyCmd](#Proto.Hdmi.StandbyCmd)
 * [UhdDeepColour](#Proto.Hdmi.UhdDeepColour)
 * [UhdDeepColour.UhdDCState](#Proto.Hdmi.UhdDeepColour.UhdDCState)
 * [Arc.Status](#Proto.Hdmi.Arc.Status)
 * [AudioFormat.AudioMode](#Proto.Hdmi.AudioFormat.AudioMode)
 * [AudioModeSelect.Type](#Proto.Hdmi.AudioModeSelect.Type)
 * [InputSelected.AudioSelected](#Proto.Hdmi.InputSelected.AudioSelected)
 * [StandbyCmd.Type](#Proto.Hdmi.StandbyCmd.Type)
* [light-sensor.proto](#light-sensor.proto)
 * [Command](#Proto.LightSensor.Command)
 * [LightLevel](#Proto.LightSensor.LightLevel)
 * [ReplyData](#Proto.LightSensor.ReplyData)
 * [Command.Type](#Proto.LightSensor.Command.Type)
* [player.proto](#player.proto)
 * [Data](#Proto.Player.Data)
 * [Data.Source](#Proto.Player.Data.Source)
 * [Data.State](#Proto.Player.Data.State)
* [power-link.proto](#power-link.proto)
 * [AllSensesStatus](#Proto.PowerLink.AllSensesStatus)
 * [AllSensesStatus.PortState](#Proto.PowerLink.AllSensesStatus.PortState)
 * [Data](#Proto.PowerLink.Data)
 * [SenseEvent](#Proto.PowerLink.SenseEvent)
 * [SetMute](#Proto.PowerLink.SetMute)
 * [SetON](#Proto.PowerLink.SetON)
* [production.proto](#production.proto)
 * [ButtonState](#Proto.Production.ButtonState)
 * [GetButtonState](#Proto.Production.GetButtonState)
 * [LedModeSet](#Proto.Production.LedModeSet)
 * [Tunnel](#Proto.Production.Tunnel)
 * [GetButtonState.ButtonId](#Proto.Production.GetButtonState.ButtonId)
 * [LedModeSet.StatusLed](#Proto.Production.LedModeSet.StatusLed)
* [puc.proto](#puc.proto)
 * [PucCommand](#Proto.Puc.PucCommand)
 * [PucCommand.CommandFormat](#Proto.Puc.PucCommand.CommandFormat)
 * [PucCommand.ModulationMode](#Proto.Puc.PucCommand.ModulationMode)
 * [PucCommand.SendMode](#Proto.Puc.PucCommand.SendMode)
* [soundwall.proto](#soundwall.proto)
 * [A2Bmode](#Proto.SoundWall.A2Bmode)
 * [BassAndRoomEQ](#Proto.SoundWall.BassAndRoomEQ)
 * [DriverGain](#Proto.SoundWall.DriverGain)
 * [DspEqParam](#Proto.SoundWall.DspEqParam)
 * [GainAndDelay](#Proto.SoundWall.GainAndDelay)
 * [MuteMode](#Proto.SoundWall.MuteMode)
 * [NodeIndex](#Proto.SoundWall.NodeIndex)
 * [PowerMode](#Proto.SoundWall.PowerMode)
 * [ReqTestTone](#Proto.SoundWall.ReqTestTone)
 * [RespGetTotalNodes](#Proto.SoundWall.RespGetTotalNodes)
 * [A2Bmode.Mode](#Proto.SoundWall.A2Bmode.Mode)
 * [MuteMode.Mode](#Proto.SoundWall.MuteMode.Mode)
 * [PowerMode.Mode](#Proto.SoundWall.PowerMode.Mode)
* [spdif.proto](#spdif.proto)
 * [AudioFormatChanged](#Proto.Spdif.AudioFormatChanged)
 * [AudioFormatChanged.AudioMode](#Proto.Spdif.AudioFormatChanged.AudioMode)
 * [AudioFormatChanged.SampleFrequency](#Proto.Spdif.AudioFormatChanged.SampleFrequency)
* [system.proto](#system.proto)
 * [AudioCue](#Proto.System.AudioCue)
 * [Log](#Proto.System.Log)
 * [NetworkInfo](#Proto.System.NetworkInfo)
 * [NetworkInfo.NetworkInterface](#Proto.System.NetworkInfo.NetworkInterface)
 * [NetworkInfo.NetworkInterface.WiFi](#Proto.System.NetworkInfo.NetworkInterface.WiFi)
 * [PowerRequest](#Proto.System.PowerRequest)
 * [PowerStatus](#Proto.System.PowerStatus)
 * [PowerSupplyVoltage](#Proto.System.PowerSupplyVoltage)
 * [RespGetBoardVersion](#Proto.System.RespGetBoardVersion)
 * [Voltage](#Proto.System.Voltage)
 * [NetworkInfo.NetworkInterface.State](#Proto.System.NetworkInfo.NetworkInterface.State)
 * [NetworkInfo.NetworkInterface.Type](#Proto.System.NetworkInfo.NetworkInterface.Type)
 * [NetworkInfo.NetworkInterface.WiFi.Quality](#Proto.System.NetworkInfo.NetworkInterface.WiFi.Quality)
 * [PowerRequest.Type](#Proto.System.PowerRequest.Type)
 * [PowerStatus.ACLineStatus](#Proto.System.PowerStatus.ACLineStatus)
 * [PowerStatus.BatteryHealthStatus](#Proto.System.PowerStatus.BatteryHealthStatus)
 * [PowerStatus.BatteryStatus](#Proto.System.PowerStatus.BatteryStatus)
 * [Voltage.Type](#Proto.System.Voltage.Type)
* [wpl.proto](#wpl.proto)
 * [Event](#Proto.Wpl.Event)
 * [Request](#Proto.Wpl.Request)
 * [Response](#Proto.Wpl.Response)
 * [Command](#Proto.Wpl.Command)
 * [Status](#Proto.Wpl.Status)
* [Scalar Value Types](#scalar-value-types)

<a name="ase-fep-ReqResp.proto"/>
<p align="right"><a href="#top">Top</a></p>

## ase-fep-ReqResp.proto




<a name="Proto.AseFep.ReqResp"/>
### ReqResp


| Name | Number | Description |
| ---- | ------ | ----------- |
| FIRMWARE_UPDATE_VERSION_INFO | 0 | has Response data of type FirmwareUpdate.VersionInfo |
| FIRMWARE_UPDATE_CHUNK | 1 | has Response data of type FirmwareUpdate.ChunkResponse |
| FIRMWARE_UPDATE_SWITCH_TO_BOOTLOADER | 2 | Generic response |
| HDMI_ARC_STATUS | 3 | Has response Hdmi.Arc.Status |
| HDMI_ARC_START | 4 | Generic response |
| HDMI_ARC_END | 5 | Generic response |
| HDMI_INPUT_SELECT | 6 | Generic response |
| POWER_STATUS | 7 | has Response data of type System.PowerStatus |
| LINE_IN_SENSITIVITY | 8 | has Request and Response data of type Dsp.LineInSensitivity/ Request may have missing Dsp.LineInSensitivity data field. It means query/ of the current linein sensitivity/ Response should always have Dsp.LineInSensitivity data field populated |
| AUDIO_INPUT | 9 | Generic response/ Has request data of type Dsp.AudioInput |
| POSITION_SOUND_MODE | 10 | has request data of type Dsp.RequestPositionSoundMode/ has response data of type Dsp.ResponsePositionSoundMode |
| INTERNAL_SPEAKER_COMPENSATION | 11 | has request data of type Dsp.RequestInternalSpeakerCompensation/ has response data of type Dsp.ResponseInternalSpeakerCompensation |
| POWER_LINK_ALL_SENSES_STATUS | 12 | has Generic request/ has response data of type PowerLink.AllSensesStatus |
| POWER_LINK_SET_ON | 13 | has request data of type PowerLink.SetON/ has Generic response |
| POWER_LINK_SET_MUTE | 14 | has request data of type PowerLink.SetMute/ has Generic response |
| INTERNAL_AMPLIFIER_COMMAND | 15 | has request data of type Dsp.InternalAmplifierCommand/ has Generic response |
| FEP_APPLICATION_IS_RUNNING | 16 | checks whether the Application is running right now./ has Generic response/ DONE if the application is running/ ERROR if the application is not running |
| EEB_TELEGRAM_TRANSMIT | 17 | has request data of type Eeb.EebTelegram/ has generic response/ DONE if the telegram was transmitted/ ERROR if the transmission failed |
| PRODUCTION_LED_MODE_SET | 18 | has request data of type Production.LedModeSet/ has Generic response response/ DONE if the led is valid/ ERROR if the led is invalid for the product |
| PRODUCTION_GET_BUTTON_STATE | 19 | has request data of type Production.GetButtonState/ has Production.ButtonState response for valid buttons/ has generic ERROR response for invalid buttons on the product |
| HDMI_CEC_SEND_STANDBY | 20 | Generic response/ Send CEC command Standby |
| HDMI_CEC_SEND_POWER_UP_TV | 21 | Generic response/ Send CEC command to power up TV |
| WPL_COMMAND | 22 | has request data of type Wpl.Request/ has Wpl.Response response |
| PUC_COMMAND_SEND | 23 | has request data of type Puc.PucCommand/ has generic response/ DONE if the command was send/ ERROR if the command failed/ NOT_SUPPORTED if the command contains any unsupported types |
| POWER_REQUEST | 24 | has request data of type System.PowerRequest/ has generic response/ DONE if the power request has been accepted/ ERROR if the command failed |
| HDMI_UHD_DEEP_COLOUR_ON | 25 | Generic response/ Add UHD deep colour setting to EDID if present in TV/ HdmiInput parameter |
| HDMI_UHD_DEEP_COLOUR_OFF | 26 | Generic response/ Remove UHD deep colour setting from EDID if present/ HdmiInput parameter |
| POWERSUPPLY_VOLTAGE | 27 | has response data of type System.PowerSupplyVoltage |
| SEND_POWER_LINK_DATA | 28 | has request data of type PowerLink.Data/ has generic response |
| TOSLINK_OUT_ADJUST_SAMPLE_RATE | 29 | has request data of type Dsp.ToslinkOutSampleRate/ has generic response |
| TOSLINK_OUT_VOLUME_REGULATION_ON | 30 | request without data/ has generic response |
| TOSLINK_OUT_VOLUME_REGULATION_OFF | 31 | request without data/ has generic response |
| GET_HDMI_UHD_DEEP_COLOUR_STATUS | 32 | Get HDMI UHD Deep colour setting values for all HDMI inputs/ Response has Hdmi.UhdDeepColour |
| GET_HDMI_AUDIO_FORMAT | 33 | Get HDMI AudioFormat, it is also sent as an event, so it should only be asked for after application boot./ Response has Hdmi.AudioFormat |
| GET_HDMI_INPUT_SELECTED | 34 | Get HDMI Input selected (input number or ARC), it is also sent as an event, so it should only be asked for after application boot./ Response has Hdmi.InputSelected |
| GET_HDMI_SENSE_STATUS | 35 | HDMI sense ON/OFF for all inputs/ Response has Hdmi.InputsSense |
| SPEAKER_ENABLE_COMMAND | 36 | Request has data of type Dsp.SpeakerEnableCommand/ Generic response |
| HDMI_AUDIO_MODE_SELECT | 37 | Request HDMI input audio to be automatically detected from inputs or always ARC (Audio guidance)/ Input parameter Hdmi.AudioModeSelect/ Generic response |
| LIGHT_SENSOR_TELEGRAM | 38 | has request data of type LightSensor.Command/ has response data of type LightSensor.ReplyData |
| SOUNDWALL_GET_A2B_MODE | 39 | Get the A2B mode from soundwall/ Response has SoundWall.A2Bmode |
| SOUNDWALL_SET_A2B_MODE | 40 | Set the A2B mode to soundwall/ Input parameter SoundWall.A2Bmode/ Generic response |
| SOUNDWALL_SET_GAIN_AND_DELAY | 41 | set the &quot;GAIN_AND_DELAY&quot; parameter to soundwall/ Input parameter SoundWall.GainAndDelay/ Generic response |
| SOUNDWALL_SET_DRIVER_GAIN | 42 | set the &quot;DRIVER_GAIN&quot; parameter to soundwall/ Input parameter SoundWall.DriverGain/ Generic response |
| SOUNDWALL_SET_BASS_AND_ROOMEQ | 43 | set the &quot;BASS_AND_ROOMEQ&quot; parameter to soundwall/ Input parameter SoundWall.BassAndRoomEQ/ Generic response |
| SOUNDWALL_GET_TOTAL_NODES | 44 | Get total active soundwalls nodes./ Response has SoundWall.RespGetTotalNodes |
| DSP_PARAMETER | 45 | Request DSP paramter change/ Request has value of type Dsp.Parameter/ Possible ERROR is set inside generic response |
| SOUNDWALL_GET_POWER_MODE | 46 | Get power mode of soundwalls, standby/working/ Response has SoundWall.PowerMode |
| SOUNDWALL_SET_POWER_MODE | 47 | Set power mode to soundwalls, standby/working/ Input parameter : SoundWall.PowerMode |
| SOUNDWALL_WRITE_DSP_PARAM | 48 | store the DSP parameter to MCU flash/ input parameter : SoundWall.NodeIndex/ Generic response |
| SOUNDWALL_SET_MUTE_MODE | 49 | Mute/Unmute control/ Input parameter : SoundWall.MuteMode |
| SOUNDWALL_GET_NTC_INFO | 50 | Get version info from specify soundwall/ input parameter : SoundWall.NodeIndex, N/A to get MasterTile version/ Response has Dsp.NTCDataEvent if nodeIndex is correct./ Response genericResponse:Error when input wrong nodeIndex |
| SOUNDWALL_SYSTEM_RESTART | 51 | Restart the soundwall without power cycle the AC cable./ Input parameter : N/A/ Generic response |
| SOUNDWALL_GET_GAIN_AND_DELAY | 52 | Description: get the &quot;GAIN_AND_DELAY&quot; parameter from soundwall/ Parameters: nodeIndex/ Response: Soundwall.GainAndDelay |
| SOUNDWALL_GET_DRIVER_GAIN | 53 | Description: get the &quot;DRIVER_GAIN&quot; parameter from soundwall/ Parameters: nodeIndex/ Response: Soundwall.DriverGain |
| SOUNDWALL_GET_BASS_AND_ROOMEQ | 54 | Description: get the &quot;BASS_AND_ROOMEQ&quot; parameter from soundwall/ Parameters: nodeIndex/ Response: Soundwall.BassAndRoomEQ |
| SOUNDWALL_SET_TEST_TONE | 55 | Description: generator the white-noise the specify speaker tile no./ Parameters: ReqTestTone/ Response: GenericResponse |
| DSP_VOLUME_FADE | 56 | Request volume fade/ Input parameter Dsp.AbsoluteVolume with fade_duration field set/ Response: Generic response. If successful, this response should be delivered upon volume fade is finished |
| SYSTEM_GET_BOARD_VERSION | 57 | Description: get the boardversion from FEP./ Parameters: N/A/ Response: System.RespGetBoardVersion |
| SOUNDWALL_FACTORY_RESET | 58 | Description: Factory Reset command to soundwall/ Parameters: nodeIndex/ Generic response |




<a name="ase-fep.proto"/>
<p align="right"><a href="#top">Top</a></p>

## ase-fep.proto



<a name="Proto.AseFep.Event"/>
### Event
Event report from ASE to FEP (Local UI).

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [Event.Type](#Proto.AseFep.Event.Type) | optional |  |
| soundwallParam | [uint32](#uint32) | optional |  |
| productionTunnel | [Tunnel](#Proto.Production.Tunnel) | optional |  |
| volume | [AbsoluteVolume](#Proto.Dsp.AbsoluteVolume) | optional |  |
| mute | [Mute](#Proto.Dsp.Mute) | optional |  |
| networkInfo | [NetworkInfo](#Proto.System.NetworkInfo) | optional |  |
| playerData | [Data](#Proto.Player.Data) | optional |  |
| dspToneTouch | [ToneTouch](#Proto.Dsp.ToneTouch) | optional |  |


<a name="Proto.AseFep.Req"/>
### Req


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [ReqResp](#Proto.AseFep.ReqResp) | optional | this should be copied from the request to the response/ to be able to pair messages |
| id | [uint32](#uint32) | optional | this should be copied from the request to the response/ to be able to pair messages |
| firmwareUpdateChunk | [Chunk](#Proto.FirmwareUpdate.Chunk) | optional |  |
| hdmiInput | [HdmiInput](#Proto.Hdmi.HdmiInput) | optional |  |
| lineInSensitivity | [LineInSensitivity](#Proto.Dsp.LineInSensitivity) | optional |  |
| audioInput | [RequestAudioInput](#Proto.Dsp.RequestAudioInput) | optional |  |
| positionSoundMode | [RequestPositionSoundMode](#Proto.Dsp.RequestPositionSoundMode) | optional |  |
| internalSpeakerCompensation | [RequestInternalSpeakerCompensation](#Proto.Dsp.RequestInternalSpeakerCompensation) | optional |  |
| powerLinkSetOn | [SetON](#Proto.PowerLink.SetON) | optional |  |
| powerLinkSetMute | [SetMute](#Proto.PowerLink.SetMute) | optional |  |
| internalAmplifierCommand | [InternalAmplifierCommand](#Proto.Dsp.InternalAmplifierCommand) | optional |  |
| eebTelegramTransmit | [EebTelegram](#Proto.Eeb.EebTelegram) | optional |  |
| ledMode | [LedModeSet](#Proto.Production.LedModeSet) | optional |  |
| getButtonState | [GetButtonState](#Proto.Production.GetButtonState) | optional |  |
| standbyType | [StandbyCmd](#Proto.Hdmi.StandbyCmd) | optional |  |
| wplRequest | [Request](#Proto.Wpl.Request) | optional |  |
| sendPucCommand | [PucCommand](#Proto.Puc.PucCommand) | optional |  |
| powerRequest | [PowerRequest](#Proto.System.PowerRequest) | optional |  |
| powerLinkData | [Data](#Proto.PowerLink.Data) | optional |  |
| toslinkOutSampleRate | [ToslinkOutSampleRate](#Proto.Dsp.ToslinkOutSampleRate) | optional |  |
| speakerEnableCommand | [SpeakerEnableCommand](#Proto.Dsp.SpeakerEnableCommand) | optional |  |
| audioMode | [AudioModeSelect](#Proto.Hdmi.AudioModeSelect) | optional |  |
| lightSensorCommand | [Command](#Proto.LightSensor.Command) | optional |  |
| reqA2Bmode | [A2Bmode](#Proto.SoundWall.A2Bmode) | optional |  |
| reqGainAndDelay | [GainAndDelay](#Proto.SoundWall.GainAndDelay) | optional |  |
| reqDriverGain | [DriverGain](#Proto.SoundWall.DriverGain) | optional |  |
| reqBassAndRoomEQ | [BassAndRoomEQ](#Proto.SoundWall.BassAndRoomEQ) | optional |  |
| dspParameter | [Parameter](#Proto.Dsp.Parameter) | optional |  |
| reqPowerMode | [PowerMode](#Proto.SoundWall.PowerMode) | optional |  |
| nodeIndex | [NodeIndex](#Proto.SoundWall.NodeIndex) | optional |  |
| muteMode | [MuteMode](#Proto.SoundWall.MuteMode) | optional |  |
| reqTestTone | [ReqTestTone](#Proto.SoundWall.ReqTestTone) | optional |  |
| dspAbsoluteVolume | [AbsoluteVolume](#Proto.Dsp.AbsoluteVolume) | optional |  |


<a name="Proto.AseFep.Resp"/>
### Resp


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [ReqResp](#Proto.FepAse.ReqResp) | optional | this should be copied from the request to the response/ to be able to pair messages |
| id | [uint32](#uint32) | optional | this should be copied from the request to the response/ to be able to pair messages |
| genericResponse | [GenericResponse](#Proto.Core.GenericResponse) | optional |  |
| networkInfo | [NetworkInfo](#Proto.System.NetworkInfo) | optional |  |
| volume | [AbsoluteVolume](#Proto.Dsp.AbsoluteVolume) | optional |  |



<a name="Proto.AseFep.Event.Type"/>
### Event.Type


| Name | Number | Description |
| ---- | ------ | ----------- |
| BOOTED | 0 | ASE has completed the boot sequence and is ready to receive messages from FEP |
| SYSTEM_STATUS_STANDBY | 1 | Event send when ASE enter source standby |
| SYSTEM_STATUS_ON | 2 | Event send when exiting standby (a source has started from standby) |
| SYSTEM_STATUS_ASE_RESTART | 3 | ASE is going to restart, FEP will gracefully shut down DSP and amplifier and signal reboot on the local UI. |
| FACTORY_RESET_DONE | 4 | Event send when &gt;&gt;reset to factory&lt;&lt; is done/ When starting factory reset procedure, FACTORY_RESET_START is sent |
| SYSTEM_STATUS_ON_NO_OPERATION | 5 | Event send when the product is ON, but no user interaction is registered for a given time period, default is/ 5 minutes. How the FEP will react to the event is product specific, dimming the LEDs is one option. |
| TUNNEL | 6 | associated data in productionTunnel |
| SW_UPDATE_STARTED | 7 | Software update of the ASE has started, local UI must show software update in progress. ASE reboot is part/ of the update process, if new firmware is available for the FEP, firmware update starts after ASE reboot. |
| SW_UPDATE_FINISHED | 8 |  |
| SW_UPDATE_FAILED | 9 |  |
| COMFORT_TONE_START | 10 | ASE indicates to FEP when the comfort sound playback starts. FEP has to turn on the DSP in case the DSP is off. |
| COMFORT_TONE_DONE | 11 | ASE indicates to FEP when the comfort sound playback is finished. FEP can turn off the DSP in case it was off/ before the comfort sound playback. |
| VOLUME_CHANGED | 12 | associated data in field of type Dsp.AbsoluteVolume/ field fade_duration should be ignored. Use DSP_VOLUME_FADE request to fade volume |
| MUTE_CHANGED | 13 | associated data in field of type Dsp.Mute |
| NETWORK_INFO | 14 | associated data in field of type System.NetworkInfo |
| PLAYER_DATA | 15 | associated data in field of type Player.Data |
| FACTORY_RESET_START | 16 | Event send when &gt;&gt;reset to factory&lt;&lt; is initiated/ FACTORY_RESET_DONE is sent when factory reset procedure has finished |
| BT_PAIRING_ENABLED | 17 | Bluetooth pairing is active. ASE is in discoverable state(BT). |
| BT_PAIRING_DISABLED | 18 | Bluetooth pairing is inactive. ASE is NOT in discoverable state(BT). |
| BT_PAIRING_FAILED | 19 | Pair of Bluetooth accessory failed. |
| BT_PAIRING_SUCCEEDED | 20 | A Bluetooth accessory has been successfully paired. |
| BTLE_PAIRING_ENABLED | 21 | Bluetooth Low Energy pairing is active. ASE is in discoverable state(BTLE). |
| BTLE_PAIRING_DISABLED | 22 | Bluetooth Low Energy pairing is inactive. ASE is NOT in discoverable state(BTLE). |
| BTLE_PAIRING_FAILED | 23 | Pair of Bluetooth Low Energy accessory failed. |
| BTLE_PAIRING_SUCCEEDED | 24 | A Bluetooth Low Energy accessory has been successfully paired. |
| LOG_MESSAGE_AVAILABLE | 25 | Informs the peer that System.Log message is ready to be received |
| LOG_MESSAGE_UNAVAILABLE | 26 | This is a symmetric event to LOG_MESSAGE_AVAILABLE |
| DSP_TONE_TOUCH | 27 | Event that is reporting currently set ToneTouch parameters./ Fep should apply parameters from data if possible/ associated data is of type Dsp.ToneTouch |
| SYSTEM_STATUS_STANDBY_MULTIROOM | 28 | Event send when ASE enter source multiroom standby |
| SYSTEM_STATUS_TURNING_ON | 29 | Event send when on state is requested |
| SYSTEM_STATUS_AUDIO_ONLY | 30 | Event send when audio only state is requested |
| BT_PLAYER_CONNECTED | 31 |  |
| BT_PLAYER_DISCONNECTED | 32 |  |




<a name="common.proto"/>
<p align="right"><a href="#top">Top</a></p>

## common.proto



<a name="Proto.Core.GenericResponse"/>
### GenericResponse


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| status | [GenericResponse.Status](#Proto.Core.GenericResponse.Status) | optional |  |



<a name="Proto.Core.GenericResponse.Status"/>
### GenericResponse.Status


| Name | Number | Description |
| ---- | ------ | ----------- |
| DONE | 0 | This is sent when the request was handled |
| ERROR | 1 | This is sent when the request finished with error |
| NOT_SUPPORTED | 2 | This is sent when the request is not handled/ This is not meant to be used when reporting statuses from the user's handler |
| UNKNOWN_REQUEST | 3 | This is sent when the request cannot be parsed from the message/ This is not meant to be used when reporting statuses from the user's handler |




<a name="core.proto"/>
<p align="right"><a href="#top">Top</a></p>

## core.proto



<a name="Proto.Core.AseFepMessage"/>
### AseFepMessage


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| aseFepEvent | [Event](#Proto.AseFep.Event) | optional |  |
| aseFepReq | [Req](#Proto.AseFep.Req) | optional |  |
| aseFepResp | [Resp](#Proto.AseFep.Resp) | optional |  |


<a name="Proto.Core.FepAseMessage"/>
### FepAseMessage


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| fepAseEvent | [Event](#Proto.FepAse.Event) | optional |  |
| fepAseReq | [Req](#Proto.FepAse.Req) | optional |  |
| fepAseResp | [Resp](#Proto.FepAse.Resp) | optional |  |






<a name="dsp.proto"/>
<p align="right"><a href="#top">Top</a></p>

## dsp.proto



<a name="Proto.Dsp.AbsoluteVolume"/>
### AbsoluteVolume


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| volume | [uint32](#uint32) | optional |  |
| fade_duration | [uint32](#uint32) | optional | In milliseconds the duration of the fade operation |


<a name="Proto.Dsp.InternalAmplifierCommand"/>
### InternalAmplifierCommand


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| state | [InternalAmplifierCommand.State](#Proto.Dsp.InternalAmplifierCommand.State) | optional | Desired amplifier state |


<a name="Proto.Dsp.InternalSpeaker"/>
### InternalSpeaker


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| position | [InternalSpeaker.Position](#Proto.Dsp.InternalSpeaker.Position) | optional | Valid combinations for FS1:/ CENTRE - WOOFER/ CENTRE - FULLRANGE// Valid combinations for FS2:/ CENTRE - WOOFER/ LEFT - MIDRANGE/ RIGHT - MIDRANGE/ CENTRE - TWEETER |
| type | [InternalSpeaker.Type](#Proto.Dsp.InternalSpeaker.Type) | optional |  |
| compensation | [double](#double) | optional | The amount of gain compensation to apply to the speaker unit in dB |


<a name="Proto.Dsp.LineInSensitivity"/>
### LineInSensitivity
Set the line-in sensitivity level.

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| sensitivity | [LineInSensitivity.Sensitivity](#Proto.Dsp.LineInSensitivity.Sensitivity) | optional |  |


<a name="Proto.Dsp.Mute"/>
### Mute


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| mute | [bool](#bool) | optional |  |


<a name="Proto.Dsp.NTCDataEvent"/>
### NTCDataEvent


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| ntcValues | [NTCDataEvent.NTCValue](#Proto.Dsp.NTCDataEvent.NTCValue) | repeated |  |


<a name="Proto.Dsp.NTCDataEvent.NTCValue"/>
### NTCDataEvent.NTCValue


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| id | [NTCDataEvent.NTCValue.NTC](#Proto.Dsp.NTCDataEvent.NTCValue.NTC) | optional |  |
| value | [int32](#int32) | required |  |


<a name="Proto.Dsp.Parameter"/>
### Parameter


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [Parameter.Type](#Proto.Dsp.Parameter.Type) | optional | Mandatory field - specifies which dsp parameter should be altered |
| value | [int32](#int32) | optional | Mandatory field - parameter value |


<a name="Proto.Dsp.PositionSoundMode"/>
### PositionSoundMode
Position sound mode setting

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| position | [PositionSoundMode.Position](#Proto.Dsp.PositionSoundMode.Position) | optional |  |
| orientation | [PositionSoundMode.Orientation](#Proto.Dsp.PositionSoundMode.Orientation) | optional |  |


<a name="Proto.Dsp.RelativeVolumeChange"/>
### RelativeVolumeChange


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| volumeChange | [int32](#int32) | optional |  |


<a name="Proto.Dsp.RequestAudioInput"/>
### RequestAudioInput
Set audio input request

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| input | [RequestAudioInput.AudioInput](#Proto.Dsp.RequestAudioInput.AudioInput) | optional | Configure the audio input path. |
| local | [bool](#bool) | optional | Configure whether to use local playback or not when ever the input/output path allows it. |


<a name="Proto.Dsp.RequestInternalSpeakerCompensation"/>
### RequestInternalSpeakerCompensation
Set the internal speaker compensation gain for one or more speaker units.
/ If a speaker unit's performance deviates from the specifications, a gain value can be applied to compensated for the
/ deviation. The FEP or one of its sub-components persist the compensation gain in non-volatile memory. FEP replies
/ with RequestInternalSpeakerCompensation.

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| internalSpeaker | [InternalSpeaker](#Proto.Dsp.InternalSpeaker) | repeated |  |


<a name="Proto.Dsp.RequestPositionSoundMode"/>
### RequestPositionSoundMode


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| positionSoundMode | [PositionSoundMode](#Proto.Dsp.PositionSoundMode) | optional |  |


<a name="Proto.Dsp.ResponseInternalSpeakerCompensation"/>
### ResponseInternalSpeakerCompensation
Response when setting or getting internal speaker compensation

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| error | [ResponseInternalSpeakerCompensation.Error](#Proto.Dsp.ResponseInternalSpeakerCompensation.Error) | optional |  |
| internalSpeaker | [InternalSpeaker](#Proto.Dsp.InternalSpeaker) | repeated | Current internal speaker compensation |


<a name="Proto.Dsp.ResponsePositionSoundMode"/>
### ResponsePositionSoundMode


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| error | [ResponsePositionSoundMode.Error](#Proto.Dsp.ResponsePositionSoundMode.Error) | optional |  |
| mode | [PositionSoundMode](#Proto.Dsp.PositionSoundMode) | optional | Current position sound mode |


<a name="Proto.Dsp.SpeakerEnableCommand"/>
### SpeakerEnableCommand
Speaker enable command - provides api to enable or disable speakers
/ Only speakers that are listed should be modified. Other speakers should be left intact.

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| speaker | [SpeakerEnableCommand.Speaker](#Proto.Dsp.SpeakerEnableCommand.Speaker) | repeated |  |


<a name="Proto.Dsp.SpeakerEnableCommand.Speaker"/>
### SpeakerEnableCommand.Speaker


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| id | [SpeakerEnableCommand.Speaker.Id](#Proto.Dsp.SpeakerEnableCommand.Speaker.Id) | optional |  |
| enabled | [bool](#bool) | optional |  |


<a name="Proto.Dsp.ToneTouch"/>
### ToneTouch


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Gx1 | [double](#double) | optional |  |
| Gy1 | [double](#double) | optional |  |
| Gx2 | [double](#double) | optional |  |
| Gy2 | [double](#double) | optional |  |
| Gz | [double](#double) | optional |  |
| k5 | [double](#double) | optional |  |
| k6 | [double](#double) | optional |  |
| enabled | [bool](#bool) | optional |  |


<a name="Proto.Dsp.ToslinkOutSampleRate"/>
### ToslinkOutSampleRate


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| sampleRate | [uint32](#uint32) | optional |  |



<a name="Proto.Dsp.InternalAmplifierCommand.State"/>
### InternalAmplifierCommand.State


| Name | Number | Description |
| ---- | ------ | ----------- |
| OFF | 0 |  |
| ON | 1 |  |

<a name="Proto.Dsp.InternalSpeaker.Position"/>
### InternalSpeaker.Position


| Name | Number | Description |
| ---- | ------ | ----------- |
| LEFT | 0 |  |
| RIGHT | 1 |  |
| CENTRE | 2 |  |

<a name="Proto.Dsp.InternalSpeaker.Type"/>
### InternalSpeaker.Type


| Name | Number | Description |
| ---- | ------ | ----------- |
| TWEETER | 0 |  |
| MIDRANGE | 1 |  |
| WOOFER | 2 |  |
| FULLRANGE | 3 |  |

<a name="Proto.Dsp.LineInSensitivity.Sensitivity"/>
### LineInSensitivity.Sensitivity


| Name | Number | Description |
| ---- | ------ | ----------- |
| HIGH | 0 | Line-in is triggered by a weak input signal. Suitable for most MP3 players. |
| MEDIUM | 1 | Line-in is triggered by a medium input signal. Use with standard audio equipment and computers. |
| LOW | 2 | Line-in is triggered by a strong input signal. Suitable for DVD/BD players. |
| DISABLED | 3 | Line-in sense is disabled |

<a name="Proto.Dsp.NTCDataEvent.NTCValue.NTC"/>
### NTCDataEvent.NTCValue.NTC


| Name | Number | Description |
| ---- | ------ | ----------- |
| EXT_NTC1 | 0 |  |
| EXT_NTC2 | 1 |  |
| EXT_NTC3 | 2 |  |
| EXT_NTC4 | 3 |  |
| EXT_NTC5 | 4 |  |
| EXT_NTC6 | 5 |  |
| EXT_NTC7 | 6 |  |
| EXT_NTC8 | 7 |  |
| AMP_NTC_CH1 | 8 |  |
| AMP_NTC_CH2_3 | 9 |  |
| AMP_NTC_CH4 | 10 |  |
| AMP_NTC_CH5 | 11 |  |
| AMP_NTC_CH6_7 | 12 |  |
| AMP_NTC_CH8 | 13 |  |
| NTC_PSU | 14 |  |
| NTC_DSP | 15 |  |
| NTC_AMP | 16 |  |

<a name="Proto.Dsp.Parameter.Type"/>
### Parameter.Type


| Name | Number | Description |
| ---- | ------ | ----------- |
| LOUDNESS | 0 |  |
| BASS | 1 |  |
| TREBLE | 2 |  |

<a name="Proto.Dsp.PositionSoundMode.Orientation"/>
### PositionSoundMode.Orientation


| Name | Number | Description |
| ---- | ------ | ----------- |
| NONE | 0 |  |
| HORIZONTAL | 1 |  |
| VERTICAL | 2 |  |

<a name="Proto.Dsp.PositionSoundMode.Position"/>
### PositionSoundMode.Position


| Name | Number | Description |
| ---- | ------ | ----------- |
| UNDEFINED | 0 |  |
| FREE | 1 |  |
| WALL | 2 |  |
| CORNER | 3 |  |
| TABLE | 4 |  |

<a name="Proto.Dsp.RequestAudioInput.AudioInput"/>
### RequestAudioInput.AudioInput


| Name | Number | Description |
| ---- | ------ | ----------- |
| ASE | 0 |  |
| LINE | 1 |  |
| TOS_LINK | 2 |  |
| POWER_LINK | 3 |  |
| HDMI | 4 |  |
| WIRELESS_MULTICHANNEL | 5 |  |
| HDMI_ARC | 6 |  |
| MICROPHONE_1 | 7 |  |
| MICROPHONE_2 | 8 |  |

<a name="Proto.Dsp.ResponseInternalSpeakerCompensation.Error"/>
### ResponseInternalSpeakerCompensation.Error


| Name | Number | Description |
| ---- | ------ | ----------- |
| NO_ERROR | 1 |  |
| POSITION_ERROR | 2 |  |
| TYPE_ERROR | 3 |  |
| POSITION_TYPE_COMBINATION_ERROR | 4 |  |
| GAIN_ERROR | 5 |  |

<a name="Proto.Dsp.ResponsePositionSoundMode.Error"/>
### ResponsePositionSoundMode.Error


| Name | Number | Description |
| ---- | ------ | ----------- |
| NO_ERROR | 1 |  |
| POSITION_ERROR | 2 |  |
| ORIENTATION_ERROR | 3 |  |
| COMBINATION_ERROR | 4 |  |

<a name="Proto.Dsp.SpeakerEnableCommand.Speaker.Id"/>
### SpeakerEnableCommand.Speaker.Id


| Name | Number | Description |
| ---- | ------ | ----------- |
| FRONT | 0 |  |
| REAR | 1 |  |




<a name="eeb.proto"/>
<p align="right"><a href="#top">Top</a></p>

## eeb.proto



<a name="Proto.Eeb.EebTelegram"/>
### EebTelegram
Input parameter for to AseFep.ReqResp(EEB_TELEGRAM_TRANSMIT) and FepAse.Event(EEB_TELEGRAM_RECEIVE)

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| packetType | [EebTelegram.PacketType](#Proto.Eeb.EebTelegram.PacketType) | optional |  |
| groupId | [bool](#bool) | required |  |
| id | [uint32](#uint32) | required |  |
| telegramType | [uint32](#uint32) | required |  |
| telegramCommand | [uint32](#uint32) | required |  |
| data | [bytes](#bytes) | optional |  |



<a name="Proto.Eeb.EebTelegram.PacketType"/>
### EebTelegram.PacketType


| Name | Number | Description |
| ---- | ------ | ----------- |
| CONFIGURATION_PACKET | 0 |  |
| COMMAND_PACKET | 1 |  |




<a name="fep-ase-ReqResp.proto"/>
<p align="right"><a href="#top">Top</a></p>

## fep-ase-ReqResp.proto




<a name="Proto.FepAse.ReqResp"/>
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
| SOUND_SILENCE_TOGGLE | 9 | Generic response |
| SOUND | 10 | Generic response |
| SILENCE | 11 | Generic response |
| MUTE | 12 | Generic response |
| UNMUTE | 13 | Generic response |
| BT_PAIRING_ON | 14 | Generic response |
| BT_PAIRING_OFF | 15 | Generic response |
| BTLE_PAIRING_ON | 16 | Generic response |
| BTLE_PAIRING_OFF | 17 | Generic response |
| BT_PAIRING_TOGGLE | 18 | Generic response |
| OFF | 19 | Generic response. When the request is done, ASE is ready to be shut down/ STORAGE and OFF commands make the product turn off/ When error is set, shutdown is not possible. |
| STORAGE | 20 |  |
| FACTORY_RESET | 21 | Generic response. When the request is done, factory reset has started |
| NETWORK_SETUP | 22 | Generic response. When the request is done, network setup is initiated |
| STANDBY | 23 | Generic response. When the request is done, ASE is put to standby |
| ALL_STANDBY | 24 | Generic response. When the request is done, ALL STANDBY request is/ sent over the network and ASE is put to standby. |
| NETWORK_INFO | 25 | has Response data of type System.NetworkInfo |
| VOLUME_CHANGE | 26 | Request has data  of type Dsp.AbsoluteVolume or Dsp.RelativeVolumeChange/ Request may have missing data field. It means query/ of the current volume level. In this scenario response has data of type Dsp.AbsoluteVolume. |
| WPL_COMMAND | 27 | Response Wpl.Response.Status is set to Done if command was successfuly/ executed, or to Error if any error ocured. |
| PLAY_AUDIO_CUE | 28 |  |




<a name="fep-ase.proto"/>
<p align="right"><a href="#top">Top</a></p>

## fep-ase.proto



<a name="Proto.FepAse.Event"/>
### Event
Event report from FEP to ASE (Local UI).

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [Event.Type](#Proto.FepAse.Event.Type) | optional |  |
| productionTunnel | [Tunnel](#Proto.Production.Tunnel) | optional |  |
| powerLinkSense | [SenseEvent](#Proto.PowerLink.SenseEvent) | optional |  |
| eebTelegramReceiveEvent | [EebTelegram](#Proto.Eeb.EebTelegram) | optional |  |
| wplEvent | [Event](#Proto.Wpl.Event) | optional |  |
| ntcData | [NTCDataEvent](#Proto.Dsp.NTCDataEvent) | optional |  |
| audioFormat | [AudioFormat](#Proto.Hdmi.AudioFormat) | optional |  |
| logMessage | [Log](#Proto.System.Log) | optional |  |
| inputSelected | [InputSelected](#Proto.Hdmi.InputSelected) | optional |  |
| inputSense | [InputSense](#Proto.Hdmi.InputSense) | optional |  |
| lightSensorLightLevel | [LightLevel](#Proto.LightSensor.LightLevel) | optional |  |
| spdifAudioFormat | [AudioFormatChanged](#Proto.Spdif.AudioFormatChanged) | optional |  |


<a name="Proto.FepAse.Req"/>
### Req


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [ReqResp](#Proto.FepAse.ReqResp) | optional | this should be copied from the request to the response/ to be able to pair messages |
| id | [uint32](#uint32) | optional | this should be copied from the request to the response/ to be able to pair messages |
| absoluteVolume | [AbsoluteVolume](#Proto.Dsp.AbsoluteVolume) | optional |  |
| relativeVolumeChange | [RelativeVolumeChange](#Proto.Dsp.RelativeVolumeChange) | optional |  |
| audioCue | [AudioCue](#Proto.System.AudioCue) | optional |  |


<a name="Proto.FepAse.Resp"/>
### Resp


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [ReqResp](#Proto.AseFep.ReqResp) | optional | this should be copied from the request to the response/ to be able to pair messages |
| id | [uint32](#uint32) | optional | this should be copied from the request to the response/ to be able to pair messages |
| genericResponse | [GenericResponse](#Proto.Core.GenericResponse) | optional |  |
| firmwareUpdateChunk | [ChunkResponse](#Proto.FirmwareUpdate.ChunkResponse) | optional |  |
| firmwareUpdateVersionInfo | [VersionInfo](#Proto.FirmwareUpdate.VersionInfo) | optional |  |
| hdmiArc | [Arc](#Proto.Hdmi.Arc) | optional |  |
| powerStatus | [PowerStatus](#Proto.System.PowerStatus) | optional |  |
| lineInSensitivity | [LineInSensitivity](#Proto.Dsp.LineInSensitivity) | optional |  |
| positionSoundMode | [ResponsePositionSoundMode](#Proto.Dsp.ResponsePositionSoundMode) | optional |  |
| internalSpeakerCompensation | [ResponseInternalSpeakerCompensation](#Proto.Dsp.ResponseInternalSpeakerCompensation) | optional |  |
| powerLinkSensesStatus | [AllSensesStatus](#Proto.PowerLink.AllSensesStatus) | optional |  |
| buttonState | [ButtonState](#Proto.Production.ButtonState) | optional |  |
| wplResponse | [Response](#Proto.Wpl.Response) | optional |  |
| powerSupplyVoltage | [PowerSupplyVoltage](#Proto.System.PowerSupplyVoltage) | optional |  |
| uhdDeepColour | [UhdDeepColour](#Proto.Hdmi.UhdDeepColour) | optional |  |
| audioFormat | [AudioFormat](#Proto.Hdmi.AudioFormat) | optional |  |
| inputSelected | [InputSelected](#Proto.Hdmi.InputSelected) | optional |  |
| inputsSense | [InputsSense](#Proto.Hdmi.InputsSense) | optional |  |
| lightSensorReplyData | [ReplyData](#Proto.LightSensor.ReplyData) | optional |  |
| respA2Bmode | [A2Bmode](#Proto.SoundWall.A2Bmode) | optional |  |
| respTotalNodes | [RespGetTotalNodes](#Proto.SoundWall.RespGetTotalNodes) | optional |  |
| respPowerMode | [PowerMode](#Proto.SoundWall.PowerMode) | optional |  |
| respDriverGain | [DriverGain](#Proto.SoundWall.DriverGain) | optional |  |
| respBassAndRoomEQ | [BassAndRoomEQ](#Proto.SoundWall.BassAndRoomEQ) | optional |  |
| respGainAndDelay | [GainAndDelay](#Proto.SoundWall.GainAndDelay) | optional |  |
| respGetBoardVersion | [RespGetBoardVersion](#Proto.System.RespGetBoardVersion) | optional |  |



<a name="Proto.FepAse.Event.Type"/>
### Event.Type


| Name | Number | Description |
| ---- | ------ | ----------- |
| LINE_IN_SENSE_ACTIVE | 0 |  |
| LINE_IN_SENSE_INACTIVE | 1 |  |
| TOSLINK_SENSE_ACTIVE | 2 |  |
| TOSLINK_SENSE_INACTIVE | 3 |  |
| HDMI_ARC_START_REQUEST | 4 |  |
| HDMI_ARC_STARTED | 5 |  |
| HDMI_ARC_END_REQUEST | 6 |  |
| HDMI_ARC_ENDED | 7 |  |
| HDMI_ARC_AUDIO_MODE_ON | 8 |  |
| HDMI_ARC_AUDIO_MODE_OFF | 9 |  |
| TUNNEL | 10 | associated data in productionTunnel |
| BOOTLOADER_READY | 11 |  |
| FIRMWARE_UPDATE_FINISHED | 12 |  |
| POWERLINK_SENSE_EVENT | 13 |  |
| EEB_TELEGRAM_RECEIVE | 14 |  |
| WPL_EVENT | 15 |  |
| DSP_NTC_DATA_EVENT | 16 |  |
| HDMI_AUDIO_FORMAT | 17 |  |
| LOG_MESSAGE | 18 | associated data in field of type System.Log |
| HDMI_CEC_VOLUME_UP | 19 | Volume UP via HDMI CEC User control message |
| HDMI_CEC_VOLUME_DOWN | 20 | Volume DOWN via HDMI CEC User control message |
| HDMI_CEC_VOLUME_MUTE | 21 | Volume MUTE via HDMI CEC User control message |
| HDMI_INPUT_SELECTED | 22 | HDMI Input selected (input number or ARC) |
| POWERFAIL | 23 | indicates power failure |
| HDMI_INPUT_SENSE_CHANGED | 24 | One input sense change ON/OFF Hdmi.InputSense |
| LIGHT_SENSOR_LIGHT_LEVEL | 25 |  |
| SPDIF_AUDIO_FORMAT_CHANGED | 26 |  |




<a name="firmware-update.proto"/>
<p align="right"><a href="#top">Top</a></p>

## firmware-update.proto



<a name="Proto.FirmwareUpdate.Chunk"/>
### Chunk
Message that contains one chunk of firmware update file
/ into request data

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| totalSize | [uint32](#uint32) | required |  |
| offset | [uint32](#uint32) | required |  |
| data | [bytes](#bytes) | required | size of data is encoded as a part of data field - avoid duplicit information |
| crc | [uint32](#uint32) | optional |  |
| moduleType | [Chunk.ModuleType](#Proto.FirmwareUpdate.Chunk.ModuleType) | optional | type of a module being updated |
| wplOption | [WplOption](#Proto.FirmwareUpdate.WplOption) | optional |  |


<a name="Proto.FirmwareUpdate.ChunkResponse"/>
### ChunkResponse
Response to Chunk

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| status | [ChunkResponse.Status](#Proto.FirmwareUpdate.ChunkResponse.Status) | optional |  |
| offset | [uint32](#uint32) | required | offset for which this status applies |


<a name="Proto.FirmwareUpdate.VersionInfo"/>
### VersionInfo
Response from AseFepReq(VERSION_INFO)

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| module | [VersionInfo.Module](#Proto.FirmwareUpdate.VersionInfo.Module) | repeated | Contains version information about the connected hardware, it may consist of multiple modules. |


<a name="Proto.FirmwareUpdate.VersionInfo.Module"/>
### VersionInfo.Module


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| name | [string](#string) | required |  |
| version | [string](#string) | required |  |
| metadata | [string](#string) | optional |  |


<a name="Proto.FirmwareUpdate.WplOption"/>
### WplOption


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| id | [WplOption.ID](#Proto.FirmwareUpdate.WplOption.ID) | optional |  |
| moduleID | [WplOption.ModuleID](#Proto.FirmwareUpdate.WplOption.ModuleID) | optional |  |
| partID | [WplOption.PartID](#Proto.FirmwareUpdate.WplOption.PartID) | optional |  |



<a name="Proto.FirmwareUpdate.Chunk.ModuleType"/>
### Chunk.ModuleType


| Name | Number | Description |
| ---- | ------ | ----------- |
| FEP | 0 |  |
| WPL | 1 | in case of WPL maximum 128 bytes should be sent |
| PLD | 2 |  |

<a name="Proto.FirmwareUpdate.ChunkResponse.Status"/>
### ChunkResponse.Status


| Name | Number | Description |
| ---- | ------ | ----------- |
| OK | 0 |  |
| ERROR_CRC | 1 | when CRC supplied is invalid for provided chunk |
| ERROR_WRITE | 2 | when an error occurs during the write of the image to the flash |

<a name="Proto.FirmwareUpdate.WplOption.ID"/>
### WplOption.ID
ID of speaker that is being updated.
/ In case of master wpl module update
/ MASTER should be set

| Name | Number | Description |
| ---- | ------ | ----------- |
| SPEAKER_0 | 0 |  |
| SPEAKER_1 | 1 |  |
| SPEAKER_2 | 2 |  |
| SPEAKER_3 | 3 |  |
| SPEAKER_4 | 4 |  |
| SPEAKER_5 | 5 |  |
| SPEAKER_6 | 6 |  |
| SPEAKER_7 | 7 |  |
| SPEAKER_8 | 8 |  |
| SPEAKER_9 | 9 |  |
| SPEAKER_10 | 10 |  |
| SPEAKER_11 | 11 |  |
| MASTER | 254 |  |

<a name="Proto.FirmwareUpdate.WplOption.ModuleID"/>
### WplOption.ModuleID
Module that is being updated

| Name | Number | Description |
| ---- | ------ | ----------- |
| FEP | 0 |  |
| Summit | 1 |  |
| DSP | 2 |  |

<a name="Proto.FirmwareUpdate.WplOption.PartID"/>
### WplOption.PartID
ID of firmware part sent

| Name | Number | Description |
| ---- | ------ | ----------- |
| APP | 0 |  |
| MFG | 1 |  |
| DFS | 2 |  |
| SYSTEM_DATA | 3 |  |




<a name="hdmi.proto"/>
<p align="right"><a href="#top">Top</a></p>

## hdmi.proto



<a name="Proto.Hdmi.Arc"/>
### Arc
Response from AseFep.Req(HDMI_ARC_STATUS)

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| status | [Arc.Status](#Proto.Hdmi.Arc.Status) | optional |  |


<a name="Proto.Hdmi.AudioFormat"/>
### AudioFormat


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| audioInfoFrameData | [bytes](#bytes) | optional |  |
| channelStatusData | [bytes](#bytes) | optional |  |
| audioMode | [AudioFormat.AudioMode](#Proto.Hdmi.AudioFormat.AudioMode) | optional |  |


<a name="Proto.Hdmi.AudioModeSelect"/>
### AudioModeSelect
Input parameter for to AseFep.ReqResp(HDMI_AUDIO_MODE_SELECT)

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [AudioModeSelect.Type](#Proto.Hdmi.AudioModeSelect.Type) | optional |  |


<a name="Proto.Hdmi.HdmiInput"/>
### HdmiInput
Input parameter for to AseFep.ReqResp(HDMI_INPUT_SELECT)

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| number | [uint32](#uint32) | required |  |


<a name="Proto.Hdmi.InputSelected"/>
### InputSelected
Sent as an event or could be asked as a request AseFep.Req(GET_HDMI_INPUT_SELECTED)
/Main reason for having this is HDMI-CEC 'One touch play' feature

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| audio | [InputSelected.AudioSelected](#Proto.Hdmi.InputSelected.AudioSelected) | required |  |
| inputNumber | [uint32](#uint32) | optional | If audio from HDMI then HDMI input number |


<a name="Proto.Hdmi.InputSense"/>
### InputSense
Event from Fep (HDMI_INPUT_SENSE_CHANGED)
/ HDMI sense ON/OFF for individual input

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| senseOn | [bool](#bool) | required |  |
| hdmiPort | [uint32](#uint32) | required | 1-4 |


<a name="Proto.Hdmi.InputsSense"/>
### InputsSense
Response from AseFep.Req(GET_HDMI_SENSE_STATUS)
/ HDMI sense ON/OFF for all inputs

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| senseStatus | [InputsSense.SenseState](#Proto.Hdmi.InputsSense.SenseState) | repeated |  |


<a name="Proto.Hdmi.InputsSense.SenseState"/>
### InputsSense.SenseState


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| senseOn | [bool](#bool) | required |  |
| hdmiPort | [uint32](#uint32) | required | 1-4 |


<a name="Proto.Hdmi.StandbyCmd"/>
### StandbyCmd
Input parameter for to AseFep.ReqResp(HDMI_CEC_SEND_STANDBY)

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [StandbyCmd.Type](#Proto.Hdmi.StandbyCmd.Type) | optional |  |


<a name="Proto.Hdmi.UhdDeepColour"/>
### UhdDeepColour
Response from AseFep.Req(GET_HDMI_UHD_DEEP_COLOUR_STATUS)
/ UHD Deep colour ON/OFF for all inputs

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| uhddcStatus | [UhdDeepColour.UhdDCState](#Proto.Hdmi.UhdDeepColour.UhdDCState) | repeated |  |


<a name="Proto.Hdmi.UhdDeepColour.UhdDCState"/>
### UhdDeepColour.UhdDCState


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| uhddcEnabled | [bool](#bool) | required |  |
| hdmiPort | [uint32](#uint32) | required | 1-4 |



<a name="Proto.Hdmi.Arc.Status"/>
### Arc.Status


| Name | Number | Description |
| ---- | ------ | ----------- |
| HDMI_ARC_NOT_STARTED | 0 |  |
| HDMI_ARC_STARTED | 1 | HDMI-ARC is started, next step is wait audio mode ON from TV |
| HDMI_ARC_START_BY_TV | 2 | HDMI-ARC start is initiated by TV, not started yet |
| HDMI_ARC_START_BY_US | 3 | HDMI-ARC start is initiated by ASE, not started yet |
| HDMI_ARC_END_BY_TV | 4 | HDMI-ARC end is initiated by TV, not ended yet |
| HDMI_ARC_END_BY_US | 5 | HDMI-ARC end is initiated by ASE, not ended yet |
| HDMI_ARC_ENDED | 6 |  |
| HDMI_ARC_AUDIO_MODE_ON | 7 | HDMI-ARC is started and audio mode is set to ON (sound from TV) |
| HDMI_ARC_AUDIO_MODE_OFF | 8 |  |

<a name="Proto.Hdmi.AudioFormat.AudioMode"/>
### AudioFormat.AudioMode


| Name | Number | Description |
| ---- | ------ | ----------- |
| CHANNELS_2 | 0 |  |
| CHANNELS_8 | 1 |  |
| HBRA | 2 |  |
| DSD | 3 |  |

<a name="Proto.Hdmi.AudioModeSelect.Type"/>
### AudioModeSelect.Type


| Name | Number | Description |
| ---- | ------ | ----------- |
| AUTOMATIC | 0 | Default case. HDMI input audio is taken directly from inputs and audio from TV via ARC/ Needs to be set only when ARC_ONLY has been set previously |
| ARC_ONLY | 1 | Request HDMI input audio to be directed to TV and played back using ARC. Used when 'Audio guidance' is enabled |

<a name="Proto.Hdmi.InputSelected.AudioSelected"/>
### InputSelected.AudioSelected


| Name | Number | Description |
| ---- | ------ | ----------- |
| ARC | 0 |  |
| HDMI | 1 |  |

<a name="Proto.Hdmi.StandbyCmd.Type"/>
### StandbyCmd.Type


| Name | Number | Description |
| ---- | ------ | ----------- |
| HDMI_CEC_STANDBY_TV | 0 | Send CEC standby command to connected TV |
| HDMI_CEC_STANDBY_ALL | 1 | Send CEC standby command to all connected HDMI devices |




<a name="light-sensor.proto"/>
<p align="right"><a href="#top">Top</a></p>

## light-sensor.proto



<a name="Proto.LightSensor.Command"/>
### Command
Input parameter for to AseFep.Req (LIGHT_SENSOR_TELEGRAM)

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [Command.Type](#Proto.LightSensor.Command.Type) | required |  |
| calibrationLightLevel | [uint32](#uint32) | optional |  |


<a name="Proto.LightSensor.LightLevel"/>
### LightLevel
Event from Fep (LIGHT_SENSOR_LIGHT_LEVEL)

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| lux | [uint32](#uint32) | required |  |


<a name="Proto.LightSensor.ReplyData"/>
### ReplyData
Response from AseFep.Resp (LIGHT_SENSOR_TELEGRAM)

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| successful | [bool](#bool) | required |  |
| rawCombinedValue | [uint32](#uint32) | optional |  |
| rawIrValue | [uint32](#uint32) | optional |  |
| calibrationConst | [uint32](#uint32) | optional |  |
| lux | [uint32](#uint32) | optional |  |



<a name="Proto.LightSensor.Command.Type"/>
### Command.Type


| Name | Number | Description |
| ---- | ------ | ----------- |
| INITIALIZE | 0 |  |
| READ_RAW | 1 |  |
| CALIBRATE | 2 |  |
| READ_CALIBRATION | 3 |  |
| READ_LIGHT_LEVEL | 4 |  |




<a name="player.proto"/>
<p align="right"><a href="#top">Top</a></p>

## player.proto



<a name="Proto.Player.Data"/>
### Data


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| state | [Data.State](#Proto.Player.Data.State) | optional |  |
| source | [Data.Source](#Proto.Player.Data.Source) | optional |  |



<a name="Proto.Player.Data.Source"/>
### Data.Source


| Name | Number | Description |
| ---- | ------ | ----------- |
| UNKNOWN | 0 |  |
| BLUETOOTH | 1 |  |

<a name="Proto.Player.Data.State"/>
### Data.State


| Name | Number | Description |
| ---- | ------ | ----------- |
| PLAYING | 0 |  |
| PAUSED | 1 |  |
| STOPPED | 2 |  |




<a name="power-link.proto"/>
<p align="right"><a href="#top">Top</a></p>

## power-link.proto



<a name="Proto.PowerLink.AllSensesStatus"/>
### AllSensesStatus
Response from AseFep.Req(POWER_LINK_ALL_SENSES_STATUS)

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| senses | [AllSensesStatus.PortState](#Proto.PowerLink.AllSensesStatus.PortState) | repeated |  |


<a name="Proto.PowerLink.AllSensesStatus.PortState"/>
### AllSensesStatus.PortState


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| connected | [bool](#bool) | required |  |
| port | [uint32](#uint32) | required | Zero based index |


<a name="Proto.PowerLink.Data"/>
### Data
Request from AseFep.Req(SEND_POWER_LINK_DATA) to send data throught PL DATA pin

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| telegram | [bytes](#bytes) | optional |  |
| wired | [bool](#bool) | optional | Send to wired speakers |
| wireless | [bool](#bool) | optional | Send to wpl speakers |


<a name="Proto.PowerLink.SenseEvent"/>
### SenseEvent
Event from FepAse about the the change on PowerLink port

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| connected | [bool](#bool) | required |  |
| port | [uint32](#uint32) | required | Zero based index |


<a name="Proto.PowerLink.SetMute"/>
### SetMute
Request from AseFep.Req(POWER_LINK_SET_MUTE) to control MUTE pin

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| enable | [bool](#bool) | required |  |


<a name="Proto.PowerLink.SetON"/>
### SetON
Request from AseFep.Req(POWER_LINK_SET_ON) to control ON pins

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| port | [uint32](#uint32) | required | Zero based index |
| enable | [bool](#bool) | required |  |






<a name="production.proto"/>
<p align="right"><a href="#top">Top</a></p>

## production.proto



<a name="Proto.Production.ButtonState"/>
### ButtonState
Response for AseFep.Req(PRODUCTION_GET_BUTTON_STATE)

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| pressed | [bool](#bool) | required |  |


<a name="Proto.Production.GetButtonState"/>
### GetButtonState
Request from AseFep.Req(PRODUCTION_GET_BUTTON_STATE) for TestdASEProductionRpcGen.GetButtonState

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| buttonId | [GetButtonState.ButtonId](#Proto.Production.GetButtonState.ButtonId) | optional |  |


<a name="Proto.Production.LedModeSet"/>
### LedModeSet
Request from AseFep.Req(PRODUCTION_LED_MODE_SET) for TestdASEProductionRpcGen.LedModeSet

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| led | [LedModeSet.StatusLed](#Proto.Production.LedModeSet.StatusLed) | optional |  |
| onTimeMs | [int32](#int32) | required |  |
| offTimeMs | [int32](#int32) | required |  |


<a name="Proto.Production.Tunnel"/>
### Tunnel


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| data | [bytes](#bytes) | optional |  |



<a name="Proto.Production.GetButtonState.ButtonId"/>
### GetButtonState.ButtonId
this has NOTHING with TestdASEProduction.ButtonIdentifier

| Name | Number | Description |
| ---- | ------ | ----------- |
| BLE_Pairing | 0 |  |
| SoftAP | 1 |  |

<a name="Proto.Production.LedModeSet.StatusLed"/>
### LedModeSet.StatusLed
synchonized with TestdASEProduction.StatusLed

| Name | Number | Description |
| ---- | ------ | ----------- |
| NetBlue | 0 |  |
| NetOrange | 1 |  |
| NetRed | 2 |  |
| NetWhite | 3 |  |
| ProdOrange | 4 |  |
| ProdRed | 5 |  |
| ProdWhite | 6 |  |
| ProdGreen | 7 |  |
| PairBlue | 8 |  |
| PairRed | 9 |  |




<a name="puc.proto"/>
<p align="right"><a href="#top">Top</a></p>

## puc.proto



<a name="Proto.Puc.PucCommand"/>
### PucCommand


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| sendMode | [PucCommand.SendMode](#Proto.Puc.PucCommand.SendMode) | optional |  |
| modulationMode | [PucCommand.ModulationMode](#Proto.Puc.PucCommand.ModulationMode) | optional |  |
| commandFormat | [PucCommand.CommandFormat](#Proto.Puc.PucCommand.CommandFormat) | optional |  |
| pucOutput | [uint32](#uint32) | required |  |
| code | [uint32](#uint32) | repeated |  |
| main | [bytes](#bytes) | optional |  |
| alt | [bytes](#bytes) | optional |  |
| repeat_signal_main | [bytes](#bytes) | optional |  |
| repeat_signal_alt | [bytes](#bytes) | optional |  |
| flanks | [bytes](#bytes) | optional |  |
| altFlanks | [bytes](#bytes) | optional |  |



<a name="Proto.Puc.PucCommand.CommandFormat"/>
### PucCommand.CommandFormat


| Name | Number | Description |
| ---- | ------ | ----------- |
| IR_PROTOCOL_NEC_FORMAT | 0 |  |
| IR_PROTOCOL_RC5_FORMAT | 1 |  |
| IR_PROTOCOL_BITSTREAM_FORMAT | 2 |  |
| IR_PROTOCOL_RAW_FORMAT | 3 |  |

<a name="Proto.Puc.PucCommand.ModulationMode"/>
### PucCommand.ModulationMode


| Name | Number | Description |
| ---- | ------ | ----------- |
| IR_MODULATION_OFF | 0 |  |
| IR_MODULATION_ON | 1 |  |

<a name="Proto.Puc.PucCommand.SendMode"/>
### PucCommand.SendMode


| Name | Number | Description |
| ---- | ------ | ----------- |
| IR_SEND_COMMAND_ONCE | 0 |  |
| IR_SEND_COMMAND_CONTINUOUS | 1 |  |
| IR_SEND_COMMAND_CONTINUOUS_END | 2 |  |
| IR_SET_PUC_OUTPUT_LOW | 3 |  |
| IR_SET_PUC_OUTPUT_HIGH | 4 |  |




<a name="soundwall.proto"/>
<p align="right"><a href="#top">Top</a></p>

## soundwall.proto



<a name="Proto.SoundWall.A2Bmode"/>
### A2Bmode
*********************************************************************

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| mode | [A2Bmode.Mode](#Proto.SoundWall.A2Bmode.Mode) | optional |  |


<a name="Proto.SoundWall.BassAndRoomEQ"/>
### BassAndRoomEQ
in the &quot;Soundwall MCU BO DSP Tuning Parameter Interface v1.5.docx&quot;
/ the RoomEQ need 16*5*32bits

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| nodeIndex | [uint32](#uint32) | required | which node you want to update? |
| bass_gain | [uint32](#uint32) | required | BASS_AND_ROOMEQ setting |
| eqParam | [DspEqParam](#Proto.SoundWall.DspEqParam) | repeated | EQ array... |


<a name="Proto.SoundWall.DriverGain"/>
### DriverGain


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| nodeIndex | [uint32](#uint32) | required | which node you want to update? |
| Tile_1_B_Cal_Gain | [uint32](#uint32) | required | DRIVER_GAIN setting : [8:24] format |
| Tile_1_MT_Cal_Gain | [uint32](#uint32) | required |  |
| Tile_2_B_Cal_Gain | [uint32](#uint32) | required |  |
| Tile_2_MT_Cal_Gain | [uint32](#uint32) | required |  |
| Tile_3_B_Cal_Gain | [uint32](#uint32) | required |  |
| Tile_3_MT_Cal_Gain | [uint32](#uint32) | required |  |
| Tile_4_B_Cal_Gain | [uint32](#uint32) | required |  |
| Tile_4_MT_Cal_Gain | [uint32](#uint32) | required |  |
| Tile_1_TW_Cal_Gain | [uint32](#uint32) | required |  |
| Tile_2_TW_Cal_Gain | [uint32](#uint32) | required |  |
| Tile_3_TW_Cal_Gain | [uint32](#uint32) | required |  |
| Tile_4_TW_Cal_Gain | [uint32](#uint32) | required |  |


<a name="Proto.SoundWall.DspEqParam"/>
### DspEqParam
define the EQ parameter for 1 'point'

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| param1 | [uint32](#uint32) | required | eq parameter : [8:24] format |
| param2 | [uint32](#uint32) | required |  |
| param3 | [uint32](#uint32) | required |  |
| param4 | [uint32](#uint32) | required |  |
| param5 | [uint32](#uint32) | required |  |


<a name="Proto.SoundWall.GainAndDelay"/>
### GainAndDelay
---------------------------------------------------------------------------
/ for the parameter definition and data format,
/ refer to &quot;Soundwall MCU BO DSP Tuning Parameter Interface v1.5.docx&quot;

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| nodeIndex | [uint32](#uint32) | required | which node you want to update? |
| MT_TW_gain_L_1 | [uint32](#uint32) | required | gain setting : [8.24] format |
| MT_TW_gain_R_1 | [uint32](#uint32) | required |  |
| MT_TW_gain_L_2 | [uint32](#uint32) | required |  |
| MT_TW_gain_R_2 | [uint32](#uint32) | required |  |
| MT_TW_gain_L_3 | [uint32](#uint32) | required |  |
| MT_TW_gain_R_3 | [uint32](#uint32) | required |  |
| MT_TW_gain_L_4 | [uint32](#uint32) | required |  |
| MT_TW_gain_R_4 | [uint32](#uint32) | required |  |
| MT_TW_delay_L_1 | [uint32](#uint32) | required | delay setting : [32.0] format |
| MT_TW_delay_R_1 | [uint32](#uint32) | required |  |
| MT_TW_delay_L_2 | [uint32](#uint32) | required |  |
| MT_TW_delay_R_2 | [uint32](#uint32) | required |  |
| MT_TW_delay_L_3 | [uint32](#uint32) | required |  |
| MT_TW_delay_R_3 | [uint32](#uint32) | required |  |
| MT_TW_delay_L_4 | [uint32](#uint32) | required |  |
| MT_TW_delay_R_4 | [uint32](#uint32) | required |  |
| MUTE_L_R | [uint32](#uint32) | required | mute setting : [8:24] format |


<a name="Proto.SoundWall.MuteMode"/>
### MuteMode


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| mode | [MuteMode.Mode](#Proto.SoundWall.MuteMode.Mode) | optional |  |


<a name="Proto.SoundWall.NodeIndex"/>
### NodeIndex
in case of specify node, 
/ e.g version_info, ntc_info, write_dsp_param

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| nodeIndex | [uint32](#uint32) | optional |  |


<a name="Proto.SoundWall.PowerMode"/>
### PowerMode


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| mode | [PowerMode.Mode](#Proto.SoundWall.PowerMode.Mode) | optional |  |


<a name="Proto.SoundWall.ReqTestTone"/>
### ReqTestTone


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| nodeIndex | [uint32](#uint32) | required |  |
| speakerTile | [uint32](#uint32) | required |  |


<a name="Proto.SoundWall.RespGetTotalNodes"/>
### RespGetTotalNodes
soundwall response the total soundwall nodes

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| totalNodes | [uint32](#uint32) | optional |  |



<a name="Proto.SoundWall.A2Bmode.Mode"/>
### A2Bmode.Mode


| Name | Number | Description |
| ---- | ------ | ----------- |
| A2B_STANDALONE | 0 |  |
| A2B_MASTER | 1 |  |
| A2B_SLAVE | 2 |  |

<a name="Proto.SoundWall.MuteMode.Mode"/>
### MuteMode.Mode


| Name | Number | Description |
| ---- | ------ | ----------- |
| MUTE | 0 |  |
| UNMUTE | 1 |  |

<a name="Proto.SoundWall.PowerMode.Mode"/>
### PowerMode.Mode


| Name | Number | Description |
| ---- | ------ | ----------- |
| STANDBY | 0 |  |
| WORKING | 1 |  |




<a name="spdif.proto"/>
<p align="right"><a href="#top">Top</a></p>

## spdif.proto



<a name="Proto.Spdif.AudioFormatChanged"/>
### AudioFormatChanged
Event from Fep (SPDIF_AUDIO_FORMAT_CHANGED)
/ PCM/NON-PCM and changes in fs or channelstatusdata

| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| sampleFrequency | [AudioFormatChanged.SampleFrequency](#Proto.Spdif.AudioFormatChanged.SampleFrequency) | optional |  |
| channelStatusData | [bytes](#bytes) | optional |  |
| audioMode | [AudioFormatChanged.AudioMode](#Proto.Spdif.AudioFormatChanged.AudioMode) | optional |  |



<a name="Proto.Spdif.AudioFormatChanged.AudioMode"/>
### AudioFormatChanged.AudioMode


| Name | Number | Description |
| ---- | ------ | ----------- |
| PCM | 0 |  |
| NONPCM | 1 |  |

<a name="Proto.Spdif.AudioFormatChanged.SampleFrequency"/>
### AudioFormatChanged.SampleFrequency


| Name | Number | Description |
| ---- | ------ | ----------- |
| FS_ERROR | 0 |  |
| FS_8000 | 1 |  |
| FS_11025 | 2 |  |
| FS_12000 | 3 |  |
| FS_16000 | 4 |  |
| FS_22050 | 5 |  |
| FS_24000 | 6 |  |
| FS_32000 | 7 |  |
| FS_44100 | 8 |  |
| FS_48000 | 9 |  |
| FS_64000 | 10 |  |
| FS_88200 | 11 |  |
| FS_96000 | 12 |  |
| FS_128000 | 13 |  |
| FS_176400 | 14 |  |
| FS_192000 | 15 |  |




<a name="system.proto"/>
<p align="right"><a href="#top">Top</a></p>

## system.proto



<a name="Proto.System.AudioCue"/>
### AudioCue


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| name | [string](#string) | optional |  |


<a name="Proto.System.Log"/>
### Log


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| msg | [string](#string) | optional |  |


<a name="Proto.System.NetworkInfo"/>
### NetworkInfo


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| networkInterface | [NetworkInfo.NetworkInterface](#Proto.System.NetworkInfo.NetworkInterface) | repeated |  |


<a name="Proto.System.NetworkInfo.NetworkInterface"/>
### NetworkInfo.NetworkInterface


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [NetworkInfo.NetworkInterface.Type](#Proto.System.NetworkInfo.NetworkInterface.Type) | optional |  |
| state | [NetworkInfo.NetworkInterface.State](#Proto.System.NetworkInfo.NetworkInterface.State) | optional |  |
| wifi | [NetworkInfo.NetworkInterface.WiFi](#Proto.System.NetworkInfo.NetworkInterface.WiFi) | optional | WiFi member is only present when type equals WIFI |


<a name="Proto.System.NetworkInfo.NetworkInterface.WiFi"/>
### NetworkInfo.NetworkInterface.WiFi


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| Configured | [bool](#bool) | optional | When true, WiFi setup has been performed. |
| quality | [NetworkInfo.NetworkInterface.WiFi.Quality](#Proto.System.NetworkInfo.NetworkInterface.WiFi.Quality) | optional | Connection quality. Because RSSI is vendor specific it is converted to the levels specified by Quality |


<a name="Proto.System.PowerRequest"/>
### PowerRequest


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [PowerRequest.Type](#Proto.System.PowerRequest.Type) | optional | Power request type |
| delay_ms | [uint32](#uint32) | optional | delay after which the FEP should do the power action specified in field 'type' |


<a name="Proto.System.PowerStatus"/>
### PowerStatus


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| acLineStatus | [PowerStatus.ACLineStatus](#Proto.System.PowerStatus.ACLineStatus) | optional |  |
| batteryStatus | [PowerStatus.BatteryStatus](#Proto.System.PowerStatus.BatteryStatus) | optional |  |
| batteryLevel | [uint32](#uint32) | optional | The battery level in percent, 0-100% |
| healthStatus | [PowerStatus.BatteryHealthStatus](#Proto.System.PowerStatus.BatteryHealthStatus) | optional |  |
| charging | [bool](#bool) | optional |  |


<a name="Proto.System.PowerSupplyVoltage"/>
### PowerSupplyVoltage


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| voltage | [Voltage](#Proto.System.Voltage) | repeated |  |


<a name="Proto.System.RespGetBoardVersion"/>
### RespGetBoardVersion


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| bomVariantString | [string](#string) | optional |  |
| bomVariantLetter | [string](#string) | optional |  |
| pcbaVariant | [uint32](#uint32) | optional |  |
| pcbaVariantString | [string](#string) | optional |  |


<a name="Proto.System.Voltage"/>
### Voltage


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [Voltage.Type](#Proto.System.Voltage.Type) | optional |  |
| value | [double](#double) | optional |  |



<a name="Proto.System.NetworkInfo.NetworkInterface.State"/>
### NetworkInfo.NetworkInterface.State


| Name | Number | Description |
| ---- | ------ | ----------- |
| UNKNOWN | 0 | The device's state is unknown |
| SCANNING | 1 | Searching for an available access point |
| CONNECTING | 2 | Currently setting up data connection |
| AUTHENTICATING | 3 | Network link established, performing authentication |
| ACQUIRING | 4 | Awaiting response from DHCP server in order to assign IP address information |
| CONNECTED | 5 | IP traffic should be available |
| DISCONNECTED | 6 | IP traffic not available |
| FAILED | 7 | Attempt to connect failed |

<a name="Proto.System.NetworkInfo.NetworkInterface.Type"/>
### NetworkInfo.NetworkInterface.Type


| Name | Number | Description |
| ---- | ------ | ----------- |
| WIFI | 1 |  |
| ETHERNET | 2 |  |
| SOFT_AP | 3 |  |

<a name="Proto.System.NetworkInfo.NetworkInterface.WiFi.Quality"/>
### NetworkInfo.NetworkInterface.WiFi.Quality


| Name | Number | Description |
| ---- | ------ | ----------- |
| EXCELLENT | 0 |  |
| GOOD | 1 |  |
| POOR | 2 |  |

<a name="Proto.System.PowerRequest.Type"/>
### PowerRequest.Type


| Name | Number | Description |
| ---- | ------ | ----------- |
| POWER_OFF | 0 | FEP should power off the ASE |
| POWER_RESTART | 1 | FEP should do a power restart of ASE |
| POWER_ONLINE | 2 | FEP should go to online state, synchronous (ASE waits for completion) |

<a name="Proto.System.PowerStatus.ACLineStatus"/>
### PowerStatus.ACLineStatus


| Name | Number | Description |
| ---- | ------ | ----------- |
| UNPLUGGED | 0 |  |
| PLUGGED | 1 |  |

<a name="Proto.System.PowerStatus.BatteryHealthStatus"/>
### PowerStatus.BatteryHealthStatus


| Name | Number | Description |
| ---- | ------ | ----------- |
| UNKNOWN | 0 |  |
| GOOD | 1 |  |
| POOR | 2 |  |

<a name="Proto.System.PowerStatus.BatteryStatus"/>
### PowerStatus.BatteryStatus


| Name | Number | Description |
| ---- | ------ | ----------- |
| NO_BATTERY | 0 | The product does not have a battery |
| LEVEL_CRITICAL | 1 |  |
| LEVEL_LOW | 2 |  |
| LEVEL_MIDDLE | 3 |  |
| LEVEL_HIGH | 4 |  |

<a name="Proto.System.Voltage.Type"/>
### Voltage.Type


| Name | Number | Description |
| ---- | ------ | ----------- |
| PSV_24V | 0 |  |
| PSV_14V_SB | 1 |  |
| PSV_12V | 2 |  |
| PSV_Plus12VA | 3 |  |
| PSV_5VA | 4 |  |
| PSV_5V | 5 |  |
| PSV_3V3 | 6 |  |
| PSV_1V2 | 7 |  |
| PSV_1V1_DSP3 | 8 |  |
| PSV_1V1_DSP2 | 9 |  |
| PSV_1V1_DSP1 | 10 |  |
| PSV_1V05 | 11 |  |
| PSV_3V3A | 12 |  |




<a name="wpl.proto"/>
<p align="right"><a href="#top">Top</a></p>

## wpl.proto



<a name="Proto.Wpl.Event"/>
### Event


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [uint32](#uint32) | optional |  |


<a name="Proto.Wpl.Request"/>
### Request


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [Command](#Proto.Wpl.Command) | optional |  |
| raw | [bytes](#bytes) | optional |  |
| param | [uint32](#uint32) | optional |  |


<a name="Proto.Wpl.Response"/>
### Response


| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| type | [Command](#Proto.Wpl.Command) | optional |  |
| status | [Status](#Proto.Wpl.Status) | optional |  |
| raw | [bytes](#bytes) | optional |  |
| param | [uint32](#uint32) | optional |  |



<a name="Proto.Wpl.Command"/>
### Command


| Name | Number | Description |
| ---- | ------ | ----------- |
| Init | 0 |  |
| Shutdown | 1 |  |
| DiscoveryFull | 2 |  |
| DiscoveryFast | 3 |  |
| StoreSpkConfig | 4 |  |
| RemoveSpk | 5 |  |
| ResetSpkConfig | 6 |  |
| GetSummitModuleInfo | 7 |  |
| GetSpkDesc | 8 |  |
| GetSpkState | 9 |  |
| Mute | 10 |  |
| SetNetworkQualityThresholdLevel | 11 |  |
| RawCmd | 12 |  |
| GetSummitFwData | 13 |  |
| GetSpeakerMap | 14 |  |
| SetSpeakerMap | 15 |  |
| GetFwVersion | 16 |  |
| GetDFSRev | 17 |  |
| GetAPIVersion | 18 |  |
| SetTestSpkMac | 19 |  |
| ClearTestSpkMac | 20 |  |
| ResetTXMaster | 21 |  |
| SetSpeakersOff | 22 |  |
| SetSpeakersOn | 23 |  |
| SetMasterMfgData | 24 |  |

<a name="Proto.Wpl.Status"/>
### Status


| Name | Number | Description |
| ---- | ------ | ----------- |
| Done | 0 |  |
| Error | 1 |  |





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
