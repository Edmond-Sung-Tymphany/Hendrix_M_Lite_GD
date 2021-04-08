[DTB File list]
----------------------------------------------------------------------------------------------------------------------
 DTB Name                                                 Update Manifest          Update   Auto      Comment
                                                                               Server   enable
----------------------------------------------------------------------------------------------------------------------
 PolkAllplay_TymSrv_NormalUpdate_EnaAutoUpdate.dtb.sam    tym_normal.xml           tym      enable    for testing

 PolkAllplay_TymSrv_RepeatedUpdate_EnaAutoUpdate.dtb.sam  tym_repeated.xml         tym      enable    for testing

 PolkAllplay_Production.dtb.sam                           production.xml           polk     enable    official dtb

 PolkAllplay_Production_ReleatedUpdate.dtb.sam            production_repeated.xml  polk     enable    unused

 PolkAllplay_Default.dtb.sam                              n/a                      n/a      disable   unused

 PolkAllplay_HwTest.dtb.sam                               tym_hw_test.xml          tym      enable    unused
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
..\gen_tym_bundle_dtb.bat
