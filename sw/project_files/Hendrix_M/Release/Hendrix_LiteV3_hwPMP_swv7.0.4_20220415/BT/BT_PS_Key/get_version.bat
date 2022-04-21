set toolspath=".\tools"
set debugtransport="SPITRANS=USB SPIPORT=0"

del version.psr

:: backup psr
call %toolspath%\bin\pscli.exe -usb 0 query version.psr %toolspath%\version.psq

