[DTB File list]
----------------------------------------------------------------------------------------------------------------------
 DTB Name                                                 Update Manifest          Update   Auto      Comment
                                                                               Server   enable
----------------------------------------------------------------------------------------------------------------------
MOF_AP_Debug.dts                                          sam.xml             http://10.13.3.199/allplay/eng/danielq/mofa/fw_debug/sam.xml

MOF_AP_repeat_upgrade.dts                                 sam.xml             http://10.13.3.199/allplay/eng/danielq/mofa/fw_test/sam.xml

MOF_AP_Release.dts                                        sam.xml             http://10.13.3.199/allplay/eng/danielq/mofa/fw_release/sam.xml
----------------------------------------------------------------------------------------------------------------------


[dts --> dtb] Execute on Linux:
dtc -o cus227_PolkAllplay.dtb -O dtb cus227_PolkAllplay.dts

 OR

for f in $(ls *.dts); do dtc -o ${f/dts/dtb} -O dtb $f; done


[Write dtb] Execute on SAM:
dd if=cus227_PolkAllplay.dtb of=/dev/mtdblock9


[dtb --> dts] Execute on Linux:
dtc -I dtb -O dts -o output.dts input.dtb


[dtb --> sam] Execute on Windows:
gen_tym_bundle_dtb2.bat
