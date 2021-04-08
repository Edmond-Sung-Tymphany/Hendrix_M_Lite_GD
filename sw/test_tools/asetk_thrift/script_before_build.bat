@echo off
:: ========================================================
:: This script will transfer setting header file 
:: from C to C# format
:: ========================================================

::Note path should be split by /, not \, because it is for cat (from Cygwin)
set PATH=..\..\tool\;%PATH%

::FS1/FS2/CA16 is on branch, do not copy files
::call:FunGenSett "setting_id_fs1.cs"  "eSettingFs1Id"   "../../project_files/BnO_fs1/conf/setting_id.h"
::call:FunGenSett "setting_id_fs2.cs"  "eSettingFs2Id"   "../../project_files/BnO_fs2/conf/setting_id.h"
::call:FunGenSett "setting_id_ca16.cs" "eSettingCa16Id"  "../../project_files/ca16/conf/setting_id.h"

::Copy CA17 files
call:FunGenSett "setting_id_ca17.cs" "eSettingCa17Id"  "../../project_files/ca17/conf/setting_id.h"
goto EOF


:FunGenSett 
	set PARM_SETT_FILE=%~1& shift
	set PARM_SETT_NAME=%~1& shift
	set PARM_SETT_SRC_FILE=%~1& shift

	if exist "%PARM_SETT_SRC_FILE%" (
		echo Generating %PARM_SETT_FILE%
		echo //Auto generated from %PARM_SETT_FILE%, %DATE% %TIME% > "%PARM_SETT_FILE%"
		echo. >> "%PARM_SETT_FILE%"
		echo namespace thrift2 {  >> "%PARM_SETT_FILE%"
		echo     public partial class Sett {  >> "%PARM_SETT_FILE%"
		echo         public enum %PARM_SETT_NAME%   >> "%PARM_SETT_FILE%"

		..\..\tool\cat "%PARM_SETT_SRC_FILE%" | ..\..\tool\grep -v typedef | ..\..\tool\grep -v SETTING_ID_H | ..\..\tool\grep -v eSettingId | ..\..\tool\grep -v #endif >>  "%PARM_SETT_FILE%"
	
		echo         }  >> "%PARM_SETT_FILE%"
		echo     }  >> "%PARM_SETT_FILE%"
		echo } >> "%PARM_SETT_FILE%"

	) else (	
		echo "%PARM_SETT_SRC_FILE% is not exist, do not generate %PARM_SETT_FILE%" && goto EOF
	)

	goto EOF


:EOF
