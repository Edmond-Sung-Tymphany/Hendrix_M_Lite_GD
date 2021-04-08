#*************************************
#* Download dtc package              *
*                                    *
#*  Author: GavinLee                 *
#*  Date:   2014/2/12                *
#*************************************

wget http://artfiles.org/cygwin.org/pub/cygwinports/x86/release/dtc/dtc-1.4.0-1.tar.xz
tar Jxvf dtc-1.4.0-1.tar.xz
mkdir out
cp -f usr/bin/dtc.exe out/
echo > out/dtc.exe.local
cp -f c:/cygwin/bin/cygwin1.dll out/
cp -f c:/cygwin/bin/cygcc_s-1.dll out/



PS:
1. dtc.exe need 32bit version cygwin1.dll/cyggcc_s-1.dll. To avoid user install 64bit version cygwin, we copy dll to this folder manually.

2. dtc.exe.loacl tell windows to search dll on the same folder first
   http://stackoverflow.com/questions/338406/cygwin1-dll-not-found-when-running-a-program-written-in-c-how-can-i-make-wind