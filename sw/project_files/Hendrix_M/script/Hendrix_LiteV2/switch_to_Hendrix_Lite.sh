#!/bin/sh

function printmsg()
{
	echo "=============================================="
	echo $1
	echo "=============================================="
}

printmsg "Copy Config"

cp  ../../model_config/Hendrix_Lite.h ../../model_config.h

printmsg "Copy DSP"

cp  ../../model_config/Hendrix_Lite_DSP/ADAU1761_IC_1.h ../../DSP/ADAU1761_IC_1.h
cp  ../../model_config/Hendrix_Lite_DSP/ADAU1761_IC_1_PARAM.h ../../DSP/ADAU1761_IC_1_PARAM.h
cp  ../../model_config/Hendrix_Lite_DSP/ADAU1761_IC_1_REG.h ../../DSP/ADAU1761_IC_1_REG.h
cp  ../../model_config/Hendrix_Lite_DSP/ADAU1761_Init.h ../../DSP/ADAU1761_Init.h
rm  ../../DSP/*.dspproj
cp  ../../model_config/Hendrix_Lite_DSP/*.dspproj ../../DSP/

printmsg "Copy BT"

rm ../../BT_PS_Key/*.rar
cp  ../../model_config/Hendrix_Lite_BT/*.rar ../../BT_PS_Key/

printmsg "Extract BT Firmware"

rm ../../BT_PS_Key/*.xuv

c:/'Program Files'/WinRAR/WinRAR.exe e -Y ../../BT_PS_Key/*.RAR *.xuv ../../BT_PS_Key/

mv ../../BT_PS_Key/*.xuv ../../BT_PS_Key/Hendrix.xuv

