.data_miniDSP_A SWdetect_input @mixer1_MixedOut
.data_miniDSP_A D_to_C_1_Ch_In_D @SWdetect_output
.data_miniDSP_A split_InputDataL @SWsourceSwitch_MuxOut_L
.data_miniDSP_A split_InputDataR @SWsourceSwitch_MuxOut_R
.data_miniDSP_D swap2_MuxInL_1 @swap1_splitOut_1L
.data_miniDSP_D swap2_MuxInR_2 @swap1_splitOut_2L
.data_miniDSP_D swap2_MuxInL_2 @swap1_splitOut_2R
.data_miniDSP_D swap2_MuxInR_1 @swap1_splitOut_1R
.data_miniDSP_A SWsourceSwitch_MuxInR_3 @SWversionControl_Ch_Out
.data_miniDSP_A mixer1_MixIn_2 @split_splitOut_2R
.data_miniDSP_A mixer1_MixIn_1 @split_splitOut_2L
.data bypass3_InputDataL @AD_Ch1DataOut
.data bypass3_InputDataR @AD_Ch2DataOut
.data_miniDSP_D swap1_InputDataL @SWbypass4_MuxOut_L
.data_miniDSP_D swap1_InputDataR @SWbypass4_MuxOut_R
.data_miniDSP_A bypass1_InputDataL @split_splitOut_1L
.data_miniDSP_A bypass1_InputDataR @split_splitOut_1R
.data_miniDSP_A SWbypass2_MuxInL_2 @bypass1_splitOut_2L
.data_miniDSP_A SWbypass2_MuxInR_2 @bypass1_splitOut_2R
.data_miniDSP_A SWbypass2_MuxInR_1 @SWvolume_workingoutval_2
.data_miniDSP_A SWbypass2_MuxInL_1 @SWvolume_workingoutval_1
.data AD_Ch1DataIn @SWbypass2_MuxOut_L
.data AD_Ch2DataIn @SWbypass2_MuxOut_R
.data_miniDSP_A preGain1_Ch_In @bypass1_splitOut_1L
.data_miniDSP_D SWbypass4_MuxInL_2 @bypass3_splitOut_2L
.data_miniDSP_D SWbypass4_MuxInR_2 @bypass3_splitOut_2R
.data_miniDSP_A Biquad_6_BIQUADIN_1_D @preGain1_Ch_Out
.data_miniDSP_A SWvolume_workingval_1 @Biquad_6_yOut
.data_miniDSP_A Neg_7_Ch_In @Split_5_splitOut_1
.data_miniDSP_A Mono_Mux_1_1_MuxIn_1 @Neg_7_Ch_Out
.data_miniDSP_A Mono_Mux_1_1_MuxIn_2 @Split_5_splitOut_2
.data_miniDSP_D SWbypass4_MuxInL_1 @Split_6_splitOut_1
.data_miniDSP_A Mono_Mixer_2_MixIn_1 @Dec4xIn_1_DecimatorOutL(n)
.data_miniDSP_A SWsourceSwitch_MuxInL_1 @Mono_Mixer_2_MixedOut
.data_miniDSP_D SWbypass4_MuxInR_1 @Split_6_splitOut_2
.data_miniDSP_D Int4xOut_1_WorkingvalL(n) @swap2_MuxOut_L
.data_miniDSP_D DRC_3_DRCIN_1_D @Biquad_8_yOut
.data_miniDSP_D DRC_1_DRCIN_1_D @Biquad_1_yOut
.data_miniDSP_D AddData_1_Ch1_In @DRC_1_DRCOUT_1_D
.data_miniDSP_D AddData_1_Ch2_In @DRC_3_DRCOUT_1_D
.data_miniDSP_D Neg_1_Ch_In @swap2_MuxOut_R
.data_miniDSP_D Int4xOut_1_WorkingvalR(n) @Neg_1_Ch_Out
.data_miniDSP_A Split_5_InputData @Dec4xIn_1_DecimatorOutR(n)
.data_miniDSP_A Mono_Mixer_2_MixIn_2 @Mono_Mux_1_1_MuxOut
.data_miniDSP_D Split_1_InputData @bypass3_splitOut_1L
.data_miniDSP_D Split_6_InputData @AddData_1_Ch_Out
.data_miniDSP_D Volume_1_workingval_1 @Split_1_splitOut_1
.data_miniDSP_D Volume_1_workingval_2 @Split_1_splitOut_2
.data_miniDSP_D Volume_2_workingval_1 @Volume_1_workingoutval_1
.data_miniDSP_D Volume_2_workingval_2 @Volume_1_workingoutval_2
.data_miniDSP_D Biquad_1_BIQUADIN_1_D @Volume_2_workingoutval_1
.data_miniDSP_D Biquad_8_BIQUADIN_1_D @Volume_2_workingoutval_2
