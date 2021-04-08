if exist temp.tmp rm temp.tmp
iconv -f ANSI_X3.4-1986 -t UCS-2LE signals.conf >> temp.tmp
mv temp.tmp ../tp_sneak_dbzze/signals.conf
rm signals.conf
