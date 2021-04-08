@rem environment
set project=ca16
set hw_ver=ES2
set sw_ver=1.1.0.0
set date=20160318

set tp_target=debug

set iar_builder="C:/Program Files (x86)/IAR Systems/Embedded Workbench 7.2/common/bin/IarBuild.exe"
set zip="C:/Program Files (x86)/7-zip/7z.exe"
set hexmate_path="../../../tool/"

set package_folder=%project%_hw%hw_ver%_swv%sw_ver%_%date%_%tp_target%
set ota_folder="ota"
set production_folder="production"

set piu_hex="%project%_piu.hex"
set ubl_hex="%project%_ubl.hex"
set fw_hex="%project%_fw.hex"
set bootloader_status_hex="BnO_ca_info.hex"

set out_hex=%project%_all.hex
set fepaddr=..\stm32\include\fep_addr.h

@rem intermediate files
set tmp_hex="tmp.hex"

@rem log files
set tmp_log="hexmerger_t.log"
set out_log="hexmerger.log"

@rem clean previous build
%iar_builder% iar/piu.ewp -clean %tp_target%
%iar_builder% iar/ubl.ewp -clean %tp_target%
%iar_builder% iar/%project%.ewp -clean %tp_target%

@rem build
%iar_builder% iar/piu.ewp -build %tp_target% -parallel 4
%iar_builder% iar/ubl.ewp -build %tp_target% -parallel 4
%iar_builder% iar/%project%.ewp -build %tp_target% -parallel 4

@rem prepare hex
rmdir %package_folder% /s /q
mkdir %package_folder%
copy "iar\%tp_target%\Exe\*.hex" %package_folder%

pushd %package_folder%
copy %hexmate_path%\BnO_ca_info.hex .

@rem remove unused line containing "Start Linear Address"
sed -i "/:04000005/d" *.hex

%hexmate_path%hexmate.exe %piu_hex% %ubl_hex% %fw_hex% --edf=%hexmate_path%en_msgs.txt -o%tmp_hex% -LOGFILE=%tmp_log%
%hexmate_path%hexmate.exe %tmp_hex% %bootloader_status_hex% --edf=%hexmate_path%en_msgs.txt -o%out_hex% -LOGFILE=%out_log%

mkdir %package_folder%
mkdir %package_folder%\%production_folder%
copy %out_hex% %package_folder%\%production_folder%

@rem remove filled 0xFF
sed -i.bak "/00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF/d" *.hex

mkdir %package_folder%\%ota_folder%
copy %ubl_hex% %package_folder%\%ota_folder%
copy %fw_hex% %package_folder%\%ota_folder%
copy %fepaddr% %package_folder%\%ota_folder%

md5sum %package_folder%/*/* > md5sum.txt

%zip% a %package_folder%.zip %package_folder%

popd
