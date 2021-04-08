/// run B&O thrift dll from console.
/// author: ds.xie@tymphany.com
/// 2016-05-16  1.0   

using System;
using System.Windows.Forms;
using System.Text;
using System.Text.RegularExpressions;
using System.Linq;
using System.Diagnostics;
using System.Net;
using System.Threading;
using System.Runtime.InteropServices;
using Thrift.Protocol;
using Thrift.Transport;
using SslClient;
using System.Reflection;
using System.Collections.Generic;


[assembly: AssemblyProduct("thrift2")]
[assembly: AssemblyTitle("thrift2")]
[assembly: AssemblyDescription("run B&O thrift dll from command line")]
[assembly: AssemblyCompany("Tymphany PTE")]
[assembly: AssemblyVersion("1.0")]
[assembly: AssemblyFileVersion("1.0")]



namespace thrift2
{
    public partial class thrift2 
    {
        /**************************************
         * Type
         **************************************/
        enum eAseTkTunnelCommand
        {
            ASETK_TUNNEL_COMMAND_ECHO_REQ  = 0,
            ASETK_TUNNEL_COMMAND_ECHO_RESP = 1,
            ASETK_TUNNEL_COMMAND_BI_REQ    = 2,
            ASETK_TUNNEL_COMMAND_BI_RESP   = 3,
            ASETK_TUNNEL_COMMAND_SETT_REQ  = 4,
            ASETK_TUNNEL_COMMAND_SETT_RESP = 5,
            ASETK_TUNNEL_COMMAND_MUTE_REQ = 6,
            ASETK_TUNNEL_COMMAND_MUTE_RESP = 7,
            ASETK_TUNNEL_COMMAND_TP_MONITOR_START_REQ = 8,
            ASETK_TUNNEL_COMMAND_TP_MONITOR_START_RESP = 9,
            ASETK_TUNNEL_COMMAND_TP_MONITOR_REQ = 10,
            ASETK_TUNNEL_COMMAND_TP_MONITOR_RESP = 11,
        };

        enum eAseTkTunnelMessageOffset
        {
            ASETK_TUNNEL_MO_MESSAGE_ID = 0,
            ASETK_TUNNEL_MO_SIZE       = 1,
            ASETK_TUNNEL_MO_DATA       = 2,
        };




        /**************************************
         * Utility
         **************************************/
        static void PrintLine(string format, params object[] arg)
        {
            String date_str = DateTime.Now.ToString("[yyyy-MM-dd HH:mm:ss] ");
            Console.Write(date_str + format + "\n", arg);
        }



        /**************************************
         * Data Parser
         **************************************/
        static string byteArrToStr(byte[] bytes)
        {
            string str = "";
            for (int ii = 0; ii < bytes.Count(); ii++)
            {
                if (bytes[ii] == '\0')
                    break;
                str = str + Convert.ToChar(bytes[ii]);
            }
            return str;
        }

        static Int32 byteArrToInt32(byte[] sett_data)
        {
            Int32 value = 0;
            if (sett_data.Count() == 1)
                value = sett_data[0];
            else if (sett_data.Count() == 2)
                value = (Int32)BitConverter.ToInt16(sett_data, 0);
            else if (sett_data.Count() == 4)
                value = (Int32)BitConverter.ToInt32(sett_data, 0);
            else
                value = 0;

            return value;
        }

        static string listedByteToStr(List<byte> list)
        {
            string str = "";

            for (int ii = 0; ii < list.Count(); ii++)
            {
                str = str + list[ii];
            }
            return str;
        }



        /**************************************
         * FEP Communication
         **************************************/
        static byte[] getSettData(TestdASEProductionRpcGen.Client ase, byte sett_size, byte sett_id)
        {
            byte[] retBuf = null;
            int limit= 2;
            string str_error = "";

            //Someteims send fail and need retry
            int retry;
            for (retry = 0; retry < limit; retry++)
            {
                List<byte> tempByteListRecv = null;
                List<byte> tempByteListSend = new List<byte>(100);

                //send commend
                tempByteListSend.Add((byte)eAseTkTunnelCommand.ASETK_TUNNEL_COMMAND_SETT_REQ);
                tempByteListSend.Add(sett_size);
                tempByteListSend.Add(sett_id);
                ase.AseFepTunnel(tempByteListSend);
                tempByteListRecv = ase.FepAseTunnel();

                //check result
                if (tempByteListRecv.Count == 0)
                {
                    str_error = "Received nothing";
                }
                else if (tempByteListRecv[(int)eAseTkTunnelMessageOffset.ASETK_TUNNEL_MO_MESSAGE_ID] != (byte)eAseTkTunnelCommand.ASETK_TUNNEL_COMMAND_SETT_RESP)
                {
                    str_error= "Incorrect response, command id mismatch";
                }
                else if (tempByteListRecv[(int)eAseTkTunnelMessageOffset.ASETK_TUNNEL_MO_SIZE] != tempByteListSend[(int)eAseTkTunnelMessageOffset.ASETK_TUNNEL_MO_SIZE])
                {
                    str_error= "Incorrect response, size mismatch";
                }
                else
                {
                    retBuf = new byte[sett_size];
                    for (int i = 0; i != sett_size; i++)
                    {
                        retBuf[i] = tempByteListRecv[i + (int)eAseTkTunnelMessageOffset.ASETK_TUNNEL_MO_DATA];
                    }
                    break;
                }
            }

            if (retry == limit)
            {
                //Console.Write("Read fail: {0}\n", str_error);
            }

            return retBuf;
        }

        static String getSettString(TestdASEProductionRpcGen.Client ase, byte sett_size, byte sett_id)
        {
            String str = "read-fail";
            byte[] sett_data = getSettData(ase, sett_size, sett_id);
            if (sett_data != null)
            {
                str = byteArrToStr(sett_data);
            }
            return str;
        }
		

        static String getSettCa16Pos(TestdASEProductionRpcGen.Client ase, byte sett_size, byte sett_id)
        {
            String str = "read-fail";
            byte[] sett_data = getSettData(ase, sett_size, sett_id);
            if (sett_data != null)
            {
                Int32 value = sett_data[8]; //[8] is position data
                str = Convert.ToString(value);
            }
            return str;
        }


        static String getSettInt32(TestdASEProductionRpcGen.Client ase, byte sett_size, byte sett_id)
        {
            String str = "read-fail";
            byte[] sett_data = getSettData(ase, sett_size, sett_id);
            if (sett_data != null)
            {
                Int32 value = byteArrToInt32(sett_data);
                str = Convert.ToString(value);
            }
            return str;
        }

        static String getSettDouble(TestdASEProductionRpcGen.Client ase, byte sett_size, byte sett_id)
        {
            String str = "read-fail";
            byte[] sett_data = getSettData(ase, sett_size, sett_id);
            if (sett_data != null)
            {
                Double value = BitConverter.ToDouble(sett_data, 0);
                //str = Convert.ToString(value);
                str = String.Format("{0:0.000000}", value);
            }
            return str;
        }

        static String getSettFloat(TestdASEProductionRpcGen.Client ase, byte sett_size, byte sett_id)
        {
            String str = "read-fail";
            byte[] sett_data = getSettData(ase, sett_size, sett_id);
            if (sett_data != null)
            {
                Double value = BitConverter.ToSingle(sett_data, 0); //ToSingle() means float
                //str = Convert.ToString(value);
                str= String.Format("{0:0.0}", value);
            }
            return str;
        }

        static String getSettInt32_m10(TestdASEProductionRpcGen.Client ase, byte sett_size, byte sett_id)
        {
            String str = "read-fail";
            byte[] sett_data = getSettData(ase, sett_size, sett_id);
            if (sett_data != null)
            {
                Int32 value = byteArrToInt32(sett_data);
                str = Convert.ToString(value);
                str = String.Format("{0}.{1}", value / 10, value % 10);
            }
            return str;
        }

        static String getSettInt32_m1000(TestdASEProductionRpcGen.Client ase, byte sett_size, byte sett_id)
        {
            String str = "read-fail";
            byte[] sett_data = getSettData(ase, sett_size, sett_id);
            if (sett_data != null)
            {
                Int32 value = byteArrToInt32(sett_data);
                str = Convert.ToString(value);
                str = String.Format("{0}.{1}", value / 1000, value % 1000);
            }
            return str;
        }

        static String getSettHex(TestdASEProductionRpcGen.Client ase, byte sett_size, byte sett_id)
        {
            String str = "read-fail";
            byte[] sett_data = getSettData(ase, sett_size, sett_id);
            if (sett_data != null)
            {
                Int32 value = byteArrToInt32(sett_data);
                str = String.Format("0x{0:x}", value);
            }
            return str;
        }



        /**************************************
         * Print Information
         **************************************/
        static void print_fs1_info(TestdASEProductionRpcGen.Client ase)
        {
            //Version
            String sw_ver_piu = getSettString(ase, /*sett_size:*/8, (byte)Sett.eSettingFs1Id.SETID_SW_PIU_VER);
            String sw_ver_ubl = getSettString(ase, /*sett_size:*/8, (byte)Sett.eSettingFs1Id.SETID_SW_UBL_VER);
            String sw_ver_fw = getSettString(ase, /*sett_size:*/8, (byte)Sett.eSettingFs1Id.SETID_SW_FW_VER);
            String hw_ver = getSettString(ase, /*sett_size:*/10, (byte)Sett.eSettingFs1Id.SETID_HW_VER);
            String batt_ver = getSettHex(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_SW);
            String dspVer = getSettString(ase, /*sett_size:*/10, (byte)Sett.eSettingFs1Id.SETID_DSP_VER);
            PrintLine("Ver:   PIU v{0} / UBL v{1} / FW v{2} / HW {3} / BATT {4} / Dsp v{5}", sw_ver_piu, sw_ver_ubl, sw_ver_fw, hw_ver, batt_ver, dspVer);

            //Error
            String err_sys = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingFs1Id.SETID_ERROR_REASON);
            String err_amp = getSettHex(ase, /*sett_size:*/4, (byte)Sett.eSettingFs1Id.SETID_AMP_ERROR_REASON);
            String err_tch_572 = getSettInt32(ase, /*sett_size:*/1, (byte)Sett.eSettingFs1Id.SETID_IQS572_CONNECTED);
            String err_tch_360 = getSettInt32(ase, /*sett_size:*/1, (byte)Sett.eSettingFs1Id.SETID_IQS360A_CONNECTED);
            PrintLine("Error: sys={0} / amp={1} / tch-572={2}, tch-360={3}", err_sys, err_amp, err_tch_572, err_tch_360);

            //Battery
            String batt_rsoc_user = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_CAPACITY_RSOC_USER);
            String batt_rsoc = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_CAPACITY_RSOC);
            String batt_asoc = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_CAPACITY_ASOC);
            String batt_cap_level = getSettInt32(ase, /*sett_size:*/1, (byte)Sett.eSettingFs1Id.SETID_BATTERY_CAPACITY_LEVEL);
            String batt_health_soh = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_HEALTH_SOH);
            String batt_health_level = getSettInt32(ase, /*sett_size:*/1, (byte)Sett.eSettingFs1Id.SETID_BATTERY_HEALTH_LEVEL);
            PrintLine("Batt:  RSOC_USER={0}%[L{1}] / RSOC={2}% / ASOC={3}% / SOH={4}%[L{5}]",
                batt_rsoc_user, batt_cap_level, batt_rsoc, batt_asoc, batt_health_soh, batt_health_level);

            //Battery part2
            String batteryTotalVol = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_TOTAL_VOL);
            String batteryCellVol_1 = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_VOLT_CELL1);
            String batteryCellVol_2 = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_VOLT_CELL2);
            String batteryCurrent = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_CURRENT);
            //String batteryAvgCurrent = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_AVG_CURRENT);
            String batteryRemainCap = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_REMAIN_CAPACITY);
            String batteryFullCh = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_FULL_CH_CAPACITY);
            String batteryDesignCap = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_DESIGN_CAPACITY);
            String batt_pack_status = getSettHex(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_PACK_STATUS);
            PrintLine("Batt:  I=cur:{1}mA, cap=remain:{3}mAh / full-ch:{4}mAh / design:{5}mA, V={6}mV(c1:{7}mV, c2:{8}mV)",
                0, batteryCurrent, batteryRemainCap, batteryFullCh, batteryDesignCap, batt_pack_status, batteryTotalVol, batteryCellVol_1, batteryCellVol_2);

            //Battery part3
            String batterySN = getSettHex(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_SN);
            String batteryExist = getSettInt32(ase, /*sett_size:*/1, (byte)Sett.eSettingFs1Id.SETID_BATTERY_EXIST);
            String batteryCycle = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_CYCLE);
            String batteryStatus = getSettHex(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_STATUS);
            String batterySafetyStatus = getSettHex(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_SAFETY_STATUS);
            String batteryPackStatus = getSettHex(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_PACK_STATUS);
            String batteryPfStatus = getSettHex(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_PF_STATUS);
            PrintLine("Batt:  SN={0}, exist={1}, cycle={2}, status={3}, safety={4}, pack={5}, pf={6}",
                batterySN, batteryExist, batteryCycle, batteryStatus, batterySafetyStatus, batteryPackStatus, batteryPfStatus);
            
            //Voltage/DC
            String dcInStatus = getSettInt32(ase, /*sett_size:*/1, (byte)Sett.eSettingFs1Id.SETID_IS_DC_PLUG_IN);
            String chargeStatus = getSettInt32(ase, /*sett_size:*/1, (byte)Sett.eSettingFs1Id.SETID_CHARGER_STATUS);
            String _5vSen = getSettInt32_m1000(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_5V_SEN);
            PrintLine("Power: DC={0}, ch-status={1}, 5V={2}V", dcInStatus, chargeStatus, _5vSen);
           
            //temperature
            String temp_wf_amp = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_TEMP_WF_AMP);
            String temp_wf_spk = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_TEMP_WF_SPK);
            String temp_tw_spk = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_TEMP_TW_SPK);
            String batt_temp = getSettInt32_m10(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_TEMP);
            String batt_temp_level = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_BATTERY_TEMP_LEVEL);
            String temp_level_audio = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs2Id.SETID_TEMP_LEVEL_AUDIO);
            PrintLine("Temp:  wf-amp={0}C / wf-spk={1}C / tw-spk={2}C / audioTempLevel=[L{3}] / batt={4}C[L{5}]",
                temp_wf_amp, temp_wf_spk, temp_tw_spk, temp_level_audio, batt_temp, batt_temp_level);

            //DSP
            //Double gainWfAsetk = ase.DseCompensateInternalSpeakerGet(DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_CENTRE, DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_WOOFER);
            //Double gainTwAsetk = ase.DseCompensateInternalSpeakerGet(DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_CENTRE, DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_FULLRANGE);
            String gainWfFep = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingFs1Id.SETID_DSP_CAL_GAIN1_WF);
            String gainTwFep = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingFs1Id.SETID_DSP_CAL_GAIN2_TW);
            String volume = getSettInt32(ase, /*sett_size:*/1, (byte)Sett.eSettingFs1Id.SETID_VOLUME);
            //String volInput = getSettFloat(ase, /*sett_size:*/1, (byte)Sett.eSettingFs1Id.SETID_AUDIO_SIGNAL_VOL_INPUT);
            //String volWfOutput = getSettFloat(ase, /*sett_size:*/1, (byte)Sett.eSettingFs1Id.SETID_AUDIO_SIGNAL_VOL_WF_OUTPUT);
            //String volTwOutput = getSettFloat(ase, /*sett_size:*/1, (byte)Sett.eSettingFs1Id.SETID_AUDIO_SIGNAL_VOL_TW_OUTPUT);
            //String gainAllow = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingFs1Id.SETID_GAIN_ALLOW);
            //PrintLine("DSP:   vol={0}.  gain=wf {1:N1}dB / tw {2:N1}dB.  real-vol=in {3} / wf-out {4} / tw-out {5}",
            //    volume, gainWfFep, gainTwFep, volInput, volWfOutput, volTwOutput);
            PrintLine("DSP:   vol={0}.  gain: wf={1:N1}dB / tw={2:N1}dB",
                volume, gainWfFep, gainTwFep);

            //DSP Parameters
            String dspBass = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_DSP_PARAM_BASS);
            String dspTreble = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_DSP_PARAM_TREBLE);
            String dspLoudness = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_DSP_PARAM_LOUDNESS);
            String dspPosition = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_DSP_PARAM_POSITION);
            PrintLine("DSP:   Bass:{0}, Treble:{1}, Loudness:{2}, Position:{3}",
                            dspBass, dspTreble, dspLoudness, dspPosition);

            //Audio
            String pvddSen = getSettInt32_m1000(ase, /*sett_size:*/2, (byte)Sett.eSettingFs1Id.SETID_PVDD_SEN);
            String dspOverheatGainWf = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingFs1Id.SETID_DSP_OVERHEAT_GAIN_WF);
            String dspOverheatGainTw = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingFs1Id.SETID_DSP_OVERHEAT_GAIN_TW);
            String dspOverheatCoilTempWf = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingFs1Id.SETID_DSP_OVERHEAT_COIL_TEMP_WF);
            String dspOverheatCoilTempTw = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingFs1Id.SETID_DSP_OVERHEAT_COIL_TEMP_TW);
            PrintLine("DSP:   PVDD={0}V. Overheat-gain: wf={1}% / tw={2}%. Coil-Temp: wf={3}C / tw={4}C",
                            pvddSen, dspOverheatGainWf, dspOverheatGainTw, dspOverheatCoilTempWf, dspOverheatCoilTempTw);
        }        


        static void print_fs2_info(TestdASEProductionRpcGen.Client ase)
        {
            //Version
            String sw_ver_piu = getSettString(ase, /*sett_size:*/8, (byte)Sett.eSettingFs2Id.SETID_SW_PIU_VER);
            String sw_ver_ubl = getSettString(ase, /*sett_size:*/8, (byte)Sett.eSettingFs2Id.SETID_SW_UBL_VER);
            String sw_ver_fw = getSettString(ase, /*sett_size:*/8, (byte)Sett.eSettingFs2Id.SETID_SW_FW_VER);
            String hw_ver = getSettString(ase, /*sett_size:*/10, (byte)Sett.eSettingFs2Id.SETID_HW_VER);
            String dspVer = getSettString(ase, /*sett_size:*/10, (byte)Sett.eSettingFs1Id.SETID_DSP_VER);
            PrintLine("Ver:   PIU v{0} / UBL v{1} / FW v{2} / HW {3} / DSP v{4}", sw_ver_piu, sw_ver_ubl, sw_ver_fw, hw_ver, dspVer);

            //Error
            String err_sys = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingFs2Id.SETID_ERROR_REASON);
            String err_amp = getSettHex(ase, /*sett_size:*/4, (byte)Sett.eSettingFs2Id.SETID_AMP_ERROR_REASON);
            String err_tch_572 = getSettInt32(ase, /*sett_size:*/1, (byte)Sett.eSettingFs2Id.SETID_IQS572_CONNECTED);
            String err_tch_360 = getSettInt32(ase, /*sett_size:*/1, (byte)Sett.eSettingFs2Id.SETID_IQS360A_CONNECTED);
            String temp_level_audio = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs2Id.SETID_TEMP_LEVEL_AUDIO);
            PrintLine("Error: sys={0} / amp={1} / tch-572={2}, tch-360={3} / tempLevelAudio=[L{4}]", err_sys, err_amp, err_tch_572, err_tch_360, temp_level_audio);

            //temperature
            String temp_wf_amp1 = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs2Id.SETID_TEMP_WF_AMP_1);
            String temp_wf_amp2 = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs2Id.SETID_TEMP_WF_AMP_2);
            String temp_wf_spk = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs2Id.SETID_TEMP_WF_SPK);
            String temp_mid_spk_a = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs2Id.SETID_TEMP_MID_SPK_A);
            String temp_mid_spk_b = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs2Id.SETID_TEMP_MID_SPK_B);
            String temp_tw_amp = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs2Id.SETID_TEMP_TW_AMP);
            PrintLine("Temp:  wf-amp1={0}C / wf-amp2={1}C / wf-spk={2}C / mid-spkA={3}C / mid-spkB={4}C / tw-amp={5}C",
                temp_wf_amp1, temp_wf_amp2, temp_wf_spk, temp_mid_spk_a, temp_mid_spk_b, temp_tw_amp);

            //DSP
            //Gain from ASE-TK
            //Double gainWfAsetk = ase.DseCompensateInternalSpeakerGet(DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_CENTRE, DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_WOOFER);
            //Double gainMidAAsetk = ase.DseCompensateInternalSpeakerGet(DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_LEFT, DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_MIDRANGE);
            //Double gainMidBAsetk = ase.DseCompensateInternalSpeakerGet(DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_RIGHT, DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_MIDRANGE);
            //Double gainTwAsetk = ase.DseCompensateInternalSpeakerGet(DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_CENTRE, DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_TWEETER);
            String gainWfFep = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingFs2Id.SETID_DSP_CAL_GAIN1_WF);
            String gainMidAFep = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingFs2Id.SETID_DSP_CAL_GAIN2_MID_A);
            String gainMidBFep = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingFs2Id.SETID_DSP_CAL_GAIN3_MID_B);
            String gainTwFep = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingFs2Id.SETID_DSP_CAL_GAIN4_TW);
            String volume = getSettInt32(ase, /*sett_size:*/1, (byte)Sett.eSettingFs2Id.SETID_VOLUME);
            //String gainAllow = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingFs2Id.SETID_GAIN_ALLOW);
            PrintLine("DSP:   vol={0} / wf-gain={1}dB / midA-gain={2}dB  / midB-gain={3}dB / tw-gain={4:N1}dB", volume, gainWfFep, gainMidAFep, gainMidBFep, gainTwFep);

            //DSP
            String audioAuxInDb = getSettFloat(ase, /*sett_size:*/4, (byte)Sett.eSettingFs2Id.SETID_AUDIO_AUXIN_IN_DB);
            String audioAuxDetect = getSettInt32(ase, /*sett_size:*/1, (byte)Sett.eSettingFs2Id.SETID_MUSIC_DET);
            String audioSysInDb = getSettFloat(ase, /*sett_size:*/4, (byte)Sett.eSettingFs2Id.SETID_AUDIO_SYS_IN_DB);
            String audioOutWf = getSettFloat(ase, /*sett_size:*/4, (byte)Sett.eSettingFs2Id.SETID_AUDIO_OUT_WF_DB);
            String audioOutMidaDb = getSettFloat(ase, /*sett_size:*/4, (byte)Sett.eSettingFs2Id.SETID_AUDIO_OUT_MID_A_DB);
            String audioOutTwDb = getSettFloat(ase, /*sett_size:*/4, (byte)Sett.eSettingFs2Id.SETID_AUDIO_OUT_TW_DB);
            PrintLine("DSP:   aux-in={0}dB[D={1}] / sys-in={2}dB / WfOut={3}dB / MidOut={4}dB / TwOut={5:N1}dB", 
                audioAuxInDb, audioAuxDetect, audioSysInDb, audioOutWf, audioOutMidaDb, audioOutTwDb);

            //DSP Parameters
            String dspBass = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs2Id.SETID_DSP_PARAM_BASS);
            String dspTreble = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs2Id.SETID_DSP_PARAM_TREBLE);
            String dspLoudness = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs2Id.SETID_DSP_PARAM_LOUDNESS);
            String dspPosition = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingFs2Id.SETID_DSP_PARAM_POSITION);
            PrintLine("DSP:   Bass:{0}, Treble:{1}, Loudness:{2}, Position:{3}",
                            dspBass, dspTreble, dspLoudness, dspPosition);

            //DSP
            String dspOverheatGainWf = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingFs2Id.SETID_DSP_OVERHEAT_GAIN_WF);
            String dspOverheatGainMid = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingFs2Id.SETID_DSP_OVERHEAT_GAIN_MID);
            String dspOverheatGainTw = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingFs2Id.SETID_DSP_OVERHEAT_GAIN_TW);
            String dspOverheatCoilTempWf = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingFs2Id.SETID_DSP_OVERHEAT_COIL_TEMP_WF);
            String dspOverheatCoilTempMidA = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingFs2Id.SETID_DSP_OVERHEAT_COIL_TEMP_MID_A);
            String dspOverheatCoilTempMidB = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingFs2Id.SETID_DSP_OVERHEAT_COIL_TEMP_MID_B);
            String dspOverheatCoilTempTw = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingFs2Id.SETID_DSP_OVERHEAT_COIL_TEMP_TW);
            PrintLine("Overheat: Gain: wf={0}% / mid={1}% / tw={2}%. Coil-Temp: wf={3}C / midA={4}C / midB={5}C / tw={6}C",
                            dspOverheatGainWf, dspOverheatGainMid, dspOverheatGainTw, 
                            dspOverheatCoilTempWf, dspOverheatCoilTempMidA, dspOverheatCoilTempMidB, dspOverheatCoilTempTw);
        }


        static void print_ca16_info(TestdASEProductionRpcGen.Client ase)
        {
            //Version
            //String sw_ver_piu = getSettString(ase, /*sett_size:*/8, (byte)Sett.eSettingCa16Id.SETID_SW_PIU_VER);
            //String sw_ver_ubl = getSettString(ase, /*sett_size:*/8, (byte)Sett.eSettingCa16Id.SETID_SW_UBL_VER);
            String sw_ver_fw = getSettString(ase, /*sett_size:*/8, (byte)Sett.eSettingCa16Id.SETID_SW_VER);
            String hw_ver = getSettString(ase, /*sett_size:*/10, (byte)Sett.eSettingCa16Id.SETID_HW_VER);
            String dspVer = getSettString(ase, /*sett_size:*/10, (byte)Sett.eSettingCa16Id.SETID_DSP_VER);
            PrintLine("Ver:   FW v{0} / HW {1} / DSP {2}", sw_ver_fw, hw_ver, dspVer);

            //Error
            String err_amp = getSettHex(ase, /*sett_size:*/4, (byte)Sett.eSettingCa16Id.SETID_AMP_ERROR_REASON);
            PrintLine("Error: amp={0}", err_amp);

            //temperature
            //String temp_wf = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingCa16Id.SETID_WF_TEMP);
            //String temp_mid = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingCa16Id.SETID_MID_TEMP);
            String temp_amp1 = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingCa16Id.SETID_AMP1_TEMP);
            String temp_amp2 = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingCa16Id.SETID_AMP2_TEMP);
            PrintLine("Temp:  amp1={0}C / amp2={1}C",
                temp_amp1, temp_amp2);

            //DSP
            String volume = getSettInt32(ase, /*sett_size:*/1, (byte)Sett.eSettingCa16Id.SETID_VOLUME);
            String pos = getSettCa16Pos(ase, /*sett_size:*/20, (byte)Sett.eSettingCa16Id.SETID_POSITION_SOUND_MODE);
            PrintLine("DSP:   vol={0}, pos={1}", volume, pos);

            //Tone Touch
            String toneTouchEnabled = getSettInt32(ase, /*sett_size:*/1, (byte)Sett.eSettingCa16Id.SETID_TONE_TOUCH_ENABLED);
            String toneTouchGx1 = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingCa16Id.SETID_TONE_TOUCH_GX1);
            String toneTouchGx2 = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingCa16Id.SETID_TONE_TOUCH_GX2);
            String toneTouchGy1 = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingCa16Id.SETID_TONE_TOUCH_GY1);
            String toneTouchGy2 = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingCa16Id.SETID_TONE_TOUCH_GY2);
            String toneTouchGz = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingCa16Id.SETID_TONE_TOUCH_GZ);
            String toneTouchK5 = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingCa16Id.SETID_TONE_TOUCH_K5);
            String toneTouchK6 = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingCa16Id.SETID_TONE_TOUCH_K6);
            PrintLine("TT:    Enable={0}, Gx1={1}, Gx2={2}, Gy1={3}, Gy2={4}, Gz={5}, K5={6}, K6={7}",
                toneTouchEnabled, toneTouchGx1, toneTouchGx2, toneTouchGy1, toneTouchGy2, toneTouchGz, toneTouchK5, toneTouchK6);
        }


        static void print_ca17_info(TestdASEProductionRpcGen.Client ase)
        {
            //Version
            //String sw_ver_piu = getSettString(ase, /*sett_size:*/8, (byte)Sett.eSettingCa17Id.SETID_SW_PIU_VER);
            //String sw_ver_ubl = getSettString(ase, /*sett_size:*/8, (byte)Sett.eSettingCa17Id.SETID_SW_UBL_VER);
            String sw_ver_fw = getSettString(ase, /*sett_size:*/8, (byte)Sett.eSettingCa17Id.SETID_SW_VER);
            String hw_ver = getSettString(ase, /*sett_size:*/10, (byte)Sett.eSettingCa17Id.SETID_HW_VER);
            String dsp_ver = getSettString(ase, /*sett_size:*/10, (byte)Sett.eSettingCa17Id.SETID_DSP_VER);
            //String hw_ver_index = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingCa17Id.SETID_HW_VER_INDEX);
            PrintLine("Ver:   FW v{0} / HW {1} / DSP v{2}", sw_ver_fw, hw_ver, dsp_ver);

            //temperature
            String temp_amp = getSettInt32(ase, /*sett_size:*/2, (byte)Sett.eSettingCa17Id.SETID_AMP_TEMP);
            String temp_level = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingCa17Id.SETID_AMP_TEMP_LEVEL);
            String linein_sen = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingCa17Id.SETID_SENSITIVITY_LINEIN);
            String linein_db = getSettFloat(ase, /*sett_size:*/4, (byte)Sett.eSettingCa17Id.SETID_AUDIO_AUXIN_IN_DB);			
            PrintLine("Audio: temp={0}C[L{1}], LineInSen={2}, LineInDetect={3}dB", temp_amp, temp_level, linein_sen, linein_db);

            //DSP
            String volume = getSettInt32(ase, /*sett_size:*/1, (byte)Sett.eSettingCa17Id.SETID_VOLUME);
            String pos = getSettInt32(ase, /*sett_size:*/4,  (byte)Sett.eSettingCa17Id.SETID_SPEAKER_POSITION);
            PrintLine("DSP:   vol={0}, pos={1}", volume, pos);

            //Error
            String stack_max = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingCa17Id.SETID_MAX_STACK_USAGE);
            int ca17_stack_size = 4608;
            String stack_max_percent = "read-fail";
            if (stack_max != "read-fail")
            {
                stack_max_percent = Convert.ToString(Convert.ToInt16(stack_max) * 100 / ca17_stack_size);
            }
            PrintLine("Sys:   stack-usage={0}bytes ({1}%)", stack_max, stack_max_percent);

            //Tone Touch
            String toneTouchEnabled = getSettInt32(ase, /*sett_size:*/1, (byte)Sett.eSettingCa17Id.SETID_TONE_TOUCH_ENABLED);
            String toneTouchGx1 = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingCa17Id.SETID_TONE_TOUCH_GX1);
            String toneTouchGx2 = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingCa17Id.SETID_TONE_TOUCH_GX2);
            String toneTouchGy1 = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingCa17Id.SETID_TONE_TOUCH_GY1);
            String toneTouchGy2 = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingCa17Id.SETID_TONE_TOUCH_GY2);
            String toneTouchGz = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingCa17Id.SETID_TONE_TOUCH_GZ);
            String toneTouchK5 = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingCa17Id.SETID_TONE_TOUCH_K5);
            String toneTouchK6 = getSettDouble(ase, /*sett_size:*/8, (byte)Sett.eSettingCa17Id.SETID_TONE_TOUCH_K6);	
            PrintLine("TT:    Enable={0}, Gx1={1}, Gx2={2}, Gy1={3}, Gy2={4}, Gz={5}, K5={6}, K6={7}", 
				toneTouchEnabled, toneTouchGx1, toneTouchGx2, toneTouchGy1, toneTouchGy2, toneTouchGz, toneTouchK5, toneTouchK6);

            //Queue
            //String queue_dbg = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingCa17Id.SETID_QUEUE_MIN_DEBUG_SRV);
            //String queue_set = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingCa17Id.SETID_QUEUE_MIN_SETTING_SRV);
            //String queue_audio = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingCa17Id.SETID_QUEUE_MIN_AUDIO_SRV);
            //String queue_led = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingCa17Id.SETID_QUEUE_MIN_LEDS_SRV);
            //String queue_key = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingCa17Id.SETID_QUEUE_MIN_KEYS_SRV);
            //String queue_pwr = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingCa17Id.SETID_QUEUE_MIN_POWER_SRV);
            //String queue_ase = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingCa17Id.SETID_QUEUE_MIN_ASE_TK_SRV);
            //PrintLine("Queue: dbg={0}, set={1}, audio={2}, led={3}, key={4}, pwr={5}, ase={6}", 
            //    queue_dbg, queue_set, queue_audio, queue_led, queue_key, queue_pwr, queue_ase);

            //Error
            //String pool_small = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingCa17Id.SETID_POOL_MIN_SMALL);
            //String pool_med = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingCa17Id.SETID_POOL_MIN_MEDIUM);
            //String pool_large = getSettInt32(ase, /*sett_size:*/4, (byte)Sett.eSettingCa17Id.SETID_POOL_MIN_LARGE);
            //PrintLine("Pool:  small={0}, median={1}, large={2}", pool_small, pool_med, pool_large);
        }


        /**************************************
         * Main
         **************************************/
        [STAThread]
        static int Main()
        {
            string[] args0 = Environment.GetCommandLineArgs();
            string[] args = new string[args0.Length - 1];
            Array.Copy(args0, 1, args, 0, args.Length);

            try
            {
                StringBuilder sb = new StringBuilder();
                if (args.Length < 2)
                {
                    Console.Write("usage:  thrift.exe ip 1/0 cmd [par1] [par2] [par3]\n");
                    Console.Write("\n");
                    Console.Write("  --ip:  product Ethernet ip address\n");
                    Console.Write("  --1/0:  ssl on or off\n");
                    Console.Write("  --cmd:  function cmd\n");
                    Console.Write("  --[par]:  optional parameter\n");
                    Console.Write("\n");
                    Console.Write("example:  thrift2.exe 192.168.1.10 1 SPDIF\n");
                    Console.Write("example:  thrift2.exe 192.168.1.10 1 BT\n");
                    Console.Write("example:  thrift2.exe 192.168.1.10 1 pair\n");
                    Console.Write("example:  thrift2.exe 192.168.1.10 1 unpair\n");
                    Console.Write("example:  thrift2.exe 192.168.1.10 1 maxvol\n");
                    Console.Write("example:  thrift2.exe 192.168.1.10 1 minvol\n");
                    Console.Write("example:  thrift2.exe 192.168.1.10 1 vol 0~90\n");
                    Console.Write("example:  thrift2.exe 192.168.1.10 1 reset\n");
                    Console.Write("\n");
                    Console.Write("[Tuning to FEP]\n");
                    Console.Write("example:  thrift2.exe 192.168.1.10 1 echo 5 1 2 3 4 5\n"); //thrift tuning
                    Console.Write("example:  thrift2.exe 192.168.1.10 1 readinfo\n");

                    //fs1 write gain
                    //Console.Write("example:  thrift2.exe 192.168.1.10 1 compensate centre fr dB\n");
                    //Console.Write("example:  thrift2.exe 192.168.1.10 1 compensate centre wf dB\n");

                    //fs2 write gain
                    //Console.Write("example:  thrift2.exe 192.168.1.10 1 compensate centre tw dB\n");
                    //Console.Write("example:  thrift2.exe 192.168.1.10 1 compensate left  mid dB\n");
                    //Console.Write("example:  thrift2.exe 192.168.1.10 1 compensate right mid dB\n");
                    //Console.Write("example:  thrift2.exe 192.168.1.10 1 compensate centre wf dB\n");
                    Console.Write("\n");
                    Console.Write("  --on successful, response 'successful', exit code 1\n");
                    Console.Write("  --on fail, response 'fail', exit code 0\n");
                    Console.Write("  --on error, response 'error', popup message\n\n");
                    Application.Exit();
                    return -1;
                }

                string ip = args[0];
                string ssl_flag = args[1].ToLower();
                string cmd = args[2].ToLower();
                string p1 = "";
                string p2 = "";
                string p3 = "";
                if (args.Length > 3)
                    p1 = args[3].ToLower();
                if (args.Length > 4)
                    p2 = args[4].ToLower();
                if (args.Length > 5)
                    p3 = args[5].ToLower();

                bool sendMagicPktOk = false;
                IPAddress ipaddr = IPAddress.Parse(ip);
                if (!Regex.IsMatch(ssl_flag, "off|no|false|0", RegexOptions.IgnoreCase))
                {
                    /*
                    try
                    {
                        Console.Write("Send magic packet to ASE-TK\n");
                        IGphSslClient ssl = new GphSslClient(ipaddr, 4334);
                        if (ssl == null) throw new Exception("null ssl\n");
                        //ssl.openservice(MAGIC_PACKET.ASE_TESTD_PRODUKTION_PACKET)
                        sendMagicPktOk = true;
                        Thread.Sleep(200);
                        //Thread.Sleep(1200);
                    }
                    catch (Exception ex2)
                    {
                        //debug version will fail
                        //throw new Exception(ex2.Message);
                    }*/
                }

                //Console.Write("Open ase connection\n");
                TStreamTransport socket = new TSocket(ip, 8525, 6000);
                if (socket == null) throw new Exception("null socket");
                TTransport transport = new TBufferedTransport(socket);
                if (transport == null) throw new Exception("null transport");
                TProtocol protocol = new TBinaryProtocol(transport);
                if (protocol == null) throw new Exception("null protocol");
                TestdASEProductionRpcGen.Client ase = new TestdASEProductionRpcGen.Client(protocol);
                //TestdCommonRpcGen.Client common = new TestdCommonRpcGen.Client(protocol);
                if (ase == null) throw new Exception("null ASE client");

                try
                {
                    //try connect directy, maybe ASE-TK got magic packet before
                    transport.Open();
                }
                catch (Exception ex)
                {
                    //If ASE-TK did not get magic pakcet before, it will cause exception.
                    //Console.Write("ERROR: " + ex.Message);
                    IGphSslClient ssl = new GphSslClient(ipaddr, 4334);
                    if (ssl == null) throw new Exception("null ssl");
                    ssl.openservice(MAGIC_PACKET.ASE_TESTD_PRODUKTION_PACKET);
                    sendMagicPktOk = true;
                    Thread.Sleep(200);
                    //Thread.Sleep(1200);
                    
                    transport.Open();
                }
                //Console.Write("ASE connected\n");

                switch (cmd)
                {
                    case "spdif":
                    case "line":
                        ase.SourceSelect(SourceType.LINEIN);
                        break;
                    case "bt":
                        ase.SourceSelect(SourceType.BLUETOOTH);
                        break;
                    case "wifi":
                        ase.SourceSelect(SourceType.RADIO);
                        break;

                    case "pair":
                        ase.BluetoothStartPairing(BluetoothDeviceType.BluetoothPlayer);
                        break;
                    case "unpair":
                        ase.BluetoothCancelPairing(BluetoothDeviceType.BluetoothPlayer);
                        break;

                    case "maxvol":
                        ase.DseVolume(90);
                        break;
                    case "minvol":
                        ase.DseVolume(0);
                        break;
                    case "vol":
                        ase.DseVolume(byte.Parse(p1));
                        break;
                    case "reset":
                        ase.ResetToDefault();
                        break;

                    //FS1, FS2, CA16 do not support router commend, MCU just ignore it.
                    case "router":
                        if (p1.Contains("left") && Regex.IsMatch(p2, "tw|tweeter"))
                            ase.DseInternalSpeakerRouter(DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_LEFT,
                                                         DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_TWEETER,
                                                         DseInternalSpeakerRouterOptions.INTERNAL_SPEAKER_ROUTER_FILTER);
                        if (p1.Contains("right") && Regex.IsMatch(p2, "tw|tweeter"))
                            ase.DseInternalSpeakerRouter(DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_RIGHT,
                                                         DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_TWEETER,
                                                         DseInternalSpeakerRouterOptions.INTERNAL_SPEAKER_ROUTER_FILTER);
                        if (p1.Contains("centre") && Regex.IsMatch(p2, "tw|tweeter"))
                            ase.DseInternalSpeakerRouter(DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_CENTRE,
                                                         DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_TWEETER,
                                                         DseInternalSpeakerRouterOptions.INTERNAL_SPEAKER_ROUTER_FILTER);
                        if (p1.Contains("centre") && Regex.IsMatch(p2, "wf|woofer"))
                            ase.DseInternalSpeakerRouter(DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_CENTRE,
                                                             DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_WOOFER,
                                                             DseInternalSpeakerRouterOptions.INTERNAL_SPEAKER_ROUTER_FILTER);
                        if (p1.Contains("left") && p2.Contains("mid"))
                            ase.DseInternalSpeakerRouter(DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_LEFT,
                                                             DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_MIDRANGE,
                                                             DseInternalSpeakerRouterOptions.INTERNAL_SPEAKER_ROUTER_FILTER);
                        if (p1.Contains("right") && p2.Contains("mid"))
                            ase.DseInternalSpeakerRouter(DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_RIGHT,
                                                             DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_MIDRANGE,
                                                             DseInternalSpeakerRouterOptions.INTERNAL_SPEAKER_ROUTER_FILTER);
                        if (p1.Contains("left") && Regex.IsMatch(p2, "tw|tweeter"))
                            ase.DseInternalSpeakerRouter(DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_LEFT,
                                                         DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_TWEETER,
                                                         DseInternalSpeakerRouterOptions.INTERNAL_SPEAKER_ROUTER_FILTER);
                        if (p1.Contains("centre") && Regex.IsMatch(p2, "wf|woofer"))
                            ase.DseInternalSpeakerRouter(DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_CENTRE,
                                                             DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_WOOFER,
                                                             DseInternalSpeakerRouterOptions.INTERNAL_SPEAKER_ROUTER_FILTER);
                        if (p1.Contains("left") && p2.Contains("mid"))
                            ase.DseInternalSpeakerRouter(DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_LEFT,
                                                             DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_MIDRANGE,
                                                             DseInternalSpeakerRouterOptions.INTERNAL_SPEAKER_ROUTER_FILTER);
                        if (p1.Contains("right") && p2.Contains("mid"))
                            ase.DseInternalSpeakerRouter(DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_RIGHT,
                                                             DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_MIDRANGE,
                                                             DseInternalSpeakerRouterOptions.INTERNAL_SPEAKER_ROUTER_FILTER);
                        break;

                    case "compensate":
                    case "offset":

                        //Start with FS1 FEP v3.0.5, input dB directly
                        //double cal = Math.Round(Math.Pow(10, double.Parse(p3) / 20), 1);
                        double cal = double.Parse(p3);
                        double gainRead= 0.0;

                        DseInternalSpeakerPosition pos;
                        DseInternalSpeakerType type;
                        if (p1.Contains("left")) {
                            pos = DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_LEFT;
                        }
                        else if (p1.Contains("right")) {
                            pos = DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_RIGHT;
                        }
                        else if (p1.Contains("centre")) {
                            pos = DseInternalSpeakerPosition.INTERNAL_SPEAKER_POSITION_CENTRE;
                        }
                        else {
                            Console.Write("unknown position\n");
                            return 0;
                        }

                        if ( Regex.IsMatch(p2, "fr|fullrange") ) {
                            type = DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_FULLRANGE;
                        }
                        else if (Regex.IsMatch(p2, "tw|tweeter")) {
                            type = DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_TWEETER;
                        }
                        else if (Regex.IsMatch(p2, "wf|woofer")) {
                            type = DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_WOOFER;
                        }
                        else if (Regex.IsMatch(p2, "mid")) {
                            type = DseInternalSpeakerType.INTERNAL_SPEAKER_TYPE_MIDRANGE;
                        }
                        else {
                            Console.Write("unknown type\n");
                            return 0;
                        }

                        ase.DseCompensateInternalSpeakerSet(pos, type, cal);
                        gainRead = ase.DseCompensateInternalSpeakerGet(pos, type);
                        Console.Write("Read back: {0}\n", gainRead);

                        break;

                    case "echo":
                    {
                        List<byte> tempByteListSend = new List<byte>(100);
                        List<byte> tempByteListRecv = new List<byte>(100);

                        tempByteListSend.Add((byte)eAseTkTunnelCommand.ASETK_TUNNEL_COMMAND_ECHO_REQ);

                        tempByteListSend.Add((byte)(Convert.ToByte(args[3])));

                        for (int ii = 4; ii < 4 + Convert.ToByte(args[3]); ii++)
                        {
                            tempByteListSend.Add((byte)(Convert.ToByte(args[ii])));
                        }

                        Console.Write("Sending\n");
                        ase.AseFepTunnel(tempByteListSend);

                        tempByteListRecv = ase.FepAseTunnel();
                        Console.Write("Sending done\n");

                        if (tempByteListRecv.Count == 0)
                        {
                            Console.Write("Received nothing\n");
                            break;
                        }

                        if (tempByteListRecv[(int)eAseTkTunnelMessageOffset.ASETK_TUNNEL_MO_MESSAGE_ID] != (byte)eAseTkTunnelCommand.ASETK_TUNNEL_COMMAND_ECHO_RESP)
                        {
                            Console.Write("Incorrect response1\n");
                            break;
                        }

                        if (tempByteListRecv[(int)eAseTkTunnelMessageOffset.ASETK_TUNNEL_MO_SIZE] != tempByteListSend[(int)eAseTkTunnelMessageOffset.ASETK_TUNNEL_MO_SIZE])
                        {
                            Console.Write("Incorrect response2\n");
                            break;
                        }

                        for (int ii = 0; ii < (int)tempByteListRecv[(int)eAseTkTunnelMessageOffset.ASETK_TUNNEL_MO_SIZE]; ii++)
                        {
                            if (tempByteListSend[(int)eAseTkTunnelMessageOffset.ASETK_TUNNEL_MO_DATA + ii] != tempByteListRecv[(int)eAseTkTunnelMessageOffset.ASETK_TUNNEL_MO_DATA + ii])
                            {
                                Console.Write("Response mismatch\n");
                                break;
                            }
                        }


                        Console.Write("Echo success\n");
                        break;
                    }

                    //read from MCU setting server (via GPB tunning)
                    case "reqset":
                    {
                        byte sett_size = (byte)(Convert.ToByte(args[3]));
                        byte sett_id = (byte)(Convert.ToByte(args[4]));
                        byte[] sett_data = getSettData(ase, sett_size, sett_id);
                                                
                        Console.Write("As char string: \n");
                        for (int ii = 0; ii < sett_size; ii++)
                        {
                            Console.Write("{0}", Convert.ToChar(sett_data[ii]));
                        }

                        Console.Write("As byte string: \n");
                        for (int ii = 0; ii < sett_size; ii++)
                        {
                            Console.Write("{0} ", sett_data[ii]);
                        }

                        Console.Write("As byte string(hex): \n");
                        for (int ii = 0; ii < sett_size; ii++)
                        {
                            Console.Write("0x{0:x} ", sett_data[ii]);
                        }
                        Console.Write("Setting request success\n");
                        break;
                    }

                    //read FS1 information (via GPB tunning to setting server)
                    case "readinfo":
                    {
                        //ASE-TK
                        PcpGetProductIdType prod = ase.PcpGetProductId();             
                        string asetk_serial = listedByteToStr(prod.ProductId.SerialNo);
                        SoftwareVersionType asetk_ver = ase.GetSoftwareVersions();
                        String devName = "";
                        if(asetk_ver.FepVersion.Length>=4 )
                        {
                            devName= asetk_ver.FepVersion.Substring(0, 4);
                        }


                        PrintLine("FepVersion = {0}", asetk_ver.FepVersion);
                        String wifiCountry = ase.NetworkWifiGetCountry();
                        if (wifiCountry == "AA")
                            wifiCountry = wifiCountry + "(US)";
                        else if (wifiCountry == "AB")
                            wifiCountry = wifiCountry + "(EU)";
                        else if (wifiCountry == "AC")
                            wifiCountry = wifiCountry + "(CN)";
                        else
                            wifiCountry = wifiCountry + "(unknown)";

                        //PrintLine("ASETK: v{0} ({1}), serial={2}", asetk_ver.Version, (sendMagicPktOk ? "release" : "debug"), asetk_serial);
                        PrintLine("ASETK: v{0}, serial={1}, wifiCountry={2}, Device={3}", asetk_ver.Version, asetk_serial, wifiCountry, devName);
                        //PrintLine("FEP(query from ASE-TK): UBL {0}, FW {1}", asetk_ver.FepBootVersion, asetk_ver.FepVersion);
                      
                        if (devName == "FS1 ")
                        {
                            print_fs1_info(ase);
                        }
                        else if (devName == "FS2 ")
                        {
                            print_fs2_info(ase);
                        }
                        else if (devName == "CA16")
                        {
                            print_ca16_info(ase);
                        }
                        else /*if (devName == "CA17")*/
                        {
                            print_ca17_info(ase);
                        }
                        /*else 
                        {
                            PrintLine("Unknow device name: {0}", devName);
                        }*/
                        break;
                    }
                    case "tp_monitor":
                    {
                        List<byte> tempByteListSend = new List<byte>(800);
                        List<byte> tempByteListRecv = new List<byte>(800);

                        tempByteListSend.Add((byte)eAseTkTunnelCommand.ASETK_TUNNEL_COMMAND_TP_MONITOR_START_REQ);

                        Console.Write("Sending Tunnel start\n");
                        ase.AseFepTunnel(tempByteListSend);

                        Console.Write("Wait Tunnel ack\n");
                        while (tempByteListRecv.Count == 0)
                        {
                            tempByteListRecv = ase.FepAseTunnel();
                        }

                        tempByteListSend.Clear();
                        tempByteListRecv.Clear();

                        tempByteListSend.Add((byte)eAseTkTunnelCommand.ASETK_TUNNEL_COMMAND_TP_MONITOR_REQ);
                        tempByteListSend.Add((byte)(Convert.ToByte(args[3])));

                        int len = Convert.ToByte(args[3]);
                        for (int ii = 4; ii < 4 + len; ii++)
                        {
                            string str = args[ii];
                            byte value = 0;

                            /* If a hex string is too long (ex. 0xFFFF), pass to Convert.ToByte() to raise exception
                             */
                            if (str.ToLower().IndexOf("0x") == 0 && str.Length <= 4)
                            {
                                string hex = str.Substring(2); //ignore 0x and get later numbers
                                value = (byte)Convert.ToInt32(hex, 16);
                            }
                            else
                            {
                               value = Convert.ToByte(args[ii]);
                            }
                            tempByteListSend.Add((byte)value);
                        }

                        Console.Write("Sending Tunnel data\n");
                        ase.AseFepTunnel(tempByteListSend);
                            
                        /* AUDIO_MUTE_CHANNEL_REQ_SIG commend do not reply ack, thus remove this section
                         */
                        //Console.Write("Wait Tunnel data ack...\n");
                        //while (tempByteListRecv.Count == 0)
                        //{
                        //    tempByteListRecv = ase.FepAseTunnel();
                        //    Console.Write(".");
                        //}

                        tempByteListSend.Clear();
                        tempByteListRecv.Clear();

                        break;
                    }
                    default:
                        Console.Write("Unknown cmd\n");
                        return 0;
                }

                transport.Close();
                socket.Close();
                Console.Write("Successful\n");
                return 1;

            }
            catch (Exception ex)
            {
                Console.Write("ERROR: " + ex.Message);
                //MessageBox.Show(ex.Message, "error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return -1;
            }
        }
    }
}
