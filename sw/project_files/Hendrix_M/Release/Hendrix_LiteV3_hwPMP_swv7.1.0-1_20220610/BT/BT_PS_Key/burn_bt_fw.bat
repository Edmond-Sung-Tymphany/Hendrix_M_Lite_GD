set toolspath=".\tools"
set debugtransport="SPITRANS=USB SPIPORT=0"

:: backup psr
call %toolspath%\bin\pscli.exe -usb 0 query %toolspath%\backup.psr %toolspath%\backup.psq

:: burn image
call %toolspath%\bin\nvscmd.exe -usb 0 erase
call %toolspath%\bin\nvscmd.exe -usb 0 burn Hendrix.xuv
call  %toolspath%\bin\pscli.exe -TRANS %debugtransport% cold_reset

:: merge back psr
sleep 1
call %toolspath%\bin\pscli.exe -usb 0 merge %toolspath%\backup.psr
call %toolspath%\bin\pscli.exe -usb 0 cold_reset

pause 
