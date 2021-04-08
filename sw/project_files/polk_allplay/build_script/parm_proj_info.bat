@echo off
::================================================
:: Setting
::================================================
::Version
set PRJ_NAME=PolkAllPlay
set SAM_VER=1.5.29-s
set SAM_THEME=polk-theme
set HW_VER=1.00


::================================================
:: Bundle file
::================================================
::Enable/disable generate bundle/xml files
set ENA_GEN_BUNDLE_DEBUG=1
set ENA_GEN_BUNDLE_DBG_R=1
set ENA_GEN_BUNDLE_RELEA=1

::Bundle file could include MCU, SAM and DTB image, or one of them.
::Note-1 dtb bundle is for internal test only, should *NOT* for shipping
::Note-2 When support dtb in bundle, SAM always show update notification if user do not factory reset after upgrading dtb

::debug bundle file
set ENA_BUNDLE_DEBUG_INCLUDE_SAM=1
set ENA_BUNDLE_DEBUG_INCLUDE_MCU=1
set ENA_BUNDLE_DEBUG_INCLUDE_DTB=1

::repeated debug bundle file
set ENA_BUNDLE_DBG_R_INCLUDE_SAM=1
set ENA_BUNDLE_DBG_R_INCLUDE_MCU=1
set ENA_BUNDLE_DBG_R_INCLUDE_DTB=1

::release bundle file
set ENA_BUNDLE_RELEA_INCLUDE_SAM=1
set ENA_BUNDLE_RELEA_INCLUDE_MCU=1
set ENA_BUNDLE_RELEA_INCLUDE_DTB=0


::================================================
:: DTB Setting
::================================================
::Enable/disable generate dtb files. Enable dtb will also enable XML file.
set ENA_GEN_DTB_DEBUG=1
set ENA_GEN_DTB_DBG_R=1
set ENA_GEN_DTB_RELEA=1

::DTB file names 
::(We use dtb's device name and manufacturer to generate bundle file)
set DTB_MAIN_FN_DEBUG=PolkAllPlay_TymSrv_NormalUpdate_EnaAutoUpdate
set DTB_MAIN_FN_DBG_R=PolkAllPlay_TymSrv_RepeatedUpdate_EnaAutoUpdate
set DTB_MAIN_FN_RELEA=PolkAllPlay_Production


::================================================
:: XML Setting
::================================================
::When upgrading, SAM query dtb's upgrade-url to get XML file, and XML context include this path
::We generate XML file when (ENA_GEN_BUNDLE_XXX=1 and ENA_GEN_DTB_XXX=1)
set UPDATE_IMG_URL_BASE_DEBUG=http:\/\/10.13.3.199\/allplay\/eng\/gavin\/fw_test
set UPDATE_IMG_URL_BASE_DBG_R=http:\/\/10.13.3.199\/allplay\/eng\/gavin\/fw_test
set UPDATE_IMG_URL_BASE_RELEA=http:\/\/allplay.polkaudio.com\/firmware\/camden\/release


::================================================
:: Project Path (should be absolute path)
::================================================
set PATH_BASE=%~dp0\..\
set PATH_PRJ_AP=%~dp0\..\mplab
set NAME_BL=bl_PolkAllPlay
set PATH_PRJ_BL=%~dp0\..\..\..\bootloaders\bl_pic32\mplab\%NAME_BL%
set PATH_MCU_HEADER=%~dp0\..\pic32\include\attachedDevicesMcu.h
set PATH_DTB=%~dp0\..\PolkAllPlay_dts_files
set PATH_CERT=%~dp0\..\sam-certs


::================================================
:: Print
::================================================
:FINISH
echo **************************************************************
::echo  [Project Setting]
::echo    PRJ_NAME:  %PRJ_NAME%
::echo    SAM_VER:   %SAM_VER%
::echo    SAM_THEME: %SAM_THEME%
::echo    ENA_GEN_BUNDLE_DEBUG: %ENA_GEN_BUNDLE_DEBUG%
::echo    ENA_GEN_BUNDLE_DBG_R: %ENA_GEN_BUNDLE_DBG_R%
::echo    ENA_GEN_BUNDLE_RELEA: %ENA_GEN_BUNDLE_RELEA%
::echo    UPDATE_IMG_URL_BASE_DEBUG: %UPDATE_IMG_URL_BASE_DEBUG%
::echo    UPDATE_IMG_URL_BASE_DBG_R: %UPDATE_IMG_URL_BASE_DBG_R%
::echo    UPDATE_IMG_URL_BASE_RELEA: %UPDATE_IMG_URL_BASE_RELEA%

