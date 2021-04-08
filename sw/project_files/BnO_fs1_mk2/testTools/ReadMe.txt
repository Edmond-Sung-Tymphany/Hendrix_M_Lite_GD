ServiceToolEmul.exe
--------------------------------------------------------
Input param:
1. ip address (check what ip ASE-TK obtained)
2. port (use 8525)

Example:
ServiceToolEmul.exe "169.254.138.32" 8525
--------------------------------------------------------
Input stream to send through tunnel in hex or dec separated with '-' or ' '

Example: 0 5 15 0x43-0x13-12 0x88 [Press Enter]
--------------------------------------------------------
Response from MCU with come as byte stream

Example:
>> >> 2 [Press Enter]
>> 3-18-0-0-100-0-255-255-255-0-243-2-0-1-0-32-8-7-208-0-
--------------------------------------------------------
Troubleshooting:
If cannot see any response for keyboard typing - connection cannot be established. Check ip and port used.