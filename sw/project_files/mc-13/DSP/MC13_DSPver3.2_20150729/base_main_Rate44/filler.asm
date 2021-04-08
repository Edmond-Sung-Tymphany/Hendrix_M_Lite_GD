.coeff_miniDSP_A MinusOne_Int =-1 $common
.coeff_miniDSP_A Anonymous_Block_0_Count =3
.codeblock Anonymous_Block_0 cycles=12, mincycle=156, maxcycle=167, target=miniDSP_A
acc_init Anonymous_Block_0_Count data_one ; Acc = N (start a delay loop)
nop
Anonymous_Block_0_NopLoop:
acc      MinusOne_Int data_one  ; Acc = N-1
jump     jmp_sz Anonymous_Block_0_NopLoop 5 ; stop when loop iterates n+2 times 
.endcodeblock
.coeff_miniDSP_A MinusOne_Int =-1 $common
.coeff_miniDSP_A Anonymous_Block_1_Count =155
.codeblock Anonymous_Block_1 cycles=316, mincycle=298, maxcycle=613, target=miniDSP_A
acc_init Anonymous_Block_1_Count data_one ; Acc = N (start a delay loop)
nop
Anonymous_Block_1_NopLoop:
acc      MinusOne_Int data_one  ; Acc = N-1
jump     jmp_sz Anonymous_Block_1_NopLoop 157 ; stop when loop iterates n+2 times 
.endcodeblock
.coeff_miniDSP_A MinusOne_Int =-1 $common
.coeff_miniDSP_A Anonymous_Block_2_Count =91
.codeblock Anonymous_Block_2 cycles=188, mincycle=710, maxcycle=897, target=miniDSP_A
acc_init Anonymous_Block_2_Count data_one ; Acc = N (start a delay loop)
nop
Anonymous_Block_2_NopLoop:
acc      MinusOne_Int data_one  ; Acc = N-1
jump     jmp_sz Anonymous_Block_2_NopLoop 93 ; stop when loop iterates n+2 times 
.endcodeblock
.codeblock Anonymous_Block_0 mincycle=2, maxcycle=3, target=miniDSP_D
nop
nop
.endcodeblock
.coeff_miniDSP_D MinusOne_Int =-1 $common
.coeff_miniDSP_D Anonymous_Block_1_Count =10
.codeblock Anonymous_Block_1 cycles=26, mincycle=247, maxcycle=272, target=miniDSP_D
acc_init Anonymous_Block_1_Count data_one ; Acc = N (start a delay loop)
nop
Anonymous_Block_1_NopLoop:
acc      MinusOne_Int data_one  ; Acc = N-1
jump     jmp_sz Anonymous_Block_1_NopLoop 12 ; stop when loop iterates n+2 times 
.endcodeblock
.coeff_miniDSP_D MinusOne_Int =-1 $common
.coeff_miniDSP_D Anonymous_Block_2_Count =14
.codeblock Anonymous_Block_2 cycles=35, mincycle=455, maxcycle=489, target=miniDSP_D
acc_init Anonymous_Block_2_Count data_one ; Acc = N (start a delay loop)
nop
Anonymous_Block_2_NopLoop:
acc      MinusOne_Int data_one  ; Acc = N-1
jump     jmp_sz Anonymous_Block_2_NopLoop 16 ; stop when loop iterates n+2 times 
nop
.endcodeblock
.coeff_miniDSP_D MinusOne_Int =-1 $common
.coeff_miniDSP_D Anonymous_Block_3_Count =120
.codeblock Anonymous_Block_3 cycles=247, mincycle=649, maxcycle=895, target=miniDSP_D
acc_init Anonymous_Block_3_Count data_one ; Acc = N (start a delay loop)
nop
Anonymous_Block_3_NopLoop:
acc      MinusOne_Int data_one  ; Acc = N-1
jump     jmp_sz Anonymous_Block_3_NopLoop 122 ; stop when loop iterates n+2 times 
nop
.endcodeblock
