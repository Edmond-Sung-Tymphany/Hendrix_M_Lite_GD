#!/bin/sh
#*************************************************
# Automatic PTE CMD Tranfer Script               *
#*************************************************



#*************************************************
# Fill Config Script                             *
#*************************************************

#Variable
FrontSufix="sdf_"
ProductName="Hendrix"
ResultFile="$FrontSufix$ProductName.Seq.Result"
ConfigFile="$FrontSufix$ProductName.Seq.Config"
TmpFile="$ResultFile".Tp
CMDNumber="16"

#File exist?
if [ ! -w $ResultFile ] || [ ! -w $ConfigFile ] ; then
    echo "Target file not exist"
    sleep 5
    return
fi

#Backup Result
cp $ResultFile $TmpFile

#Clean up
#Remove file head
#sed -i '1,8d' "$TmpFile"

#Remove -> line  such as :  -> DEBUG_RESP_SIG ( evtReturn : RET_SUCCESS ; )
sed -i '/->/d' "$TmpFile" 
#Remove line without >> <<
sed -i -r '/>>|<</!d' "$TmpFile"
#Remove line with  BT_STATE_SIG
sed -i '/BT_STATE_SIG/d' "$TmpFile"

#Reverse file
tac "$TmpFile" > "$TmpFile".tmp
cp "$TmpFile".tmp "$TmpFile"
rm "$TmpFile".tmp

#exit

#Remmove Debug_Resp # this need modify by hand
sed -i '39,77{/DEBUG_RESP/d}' "$TmpFile"
#exit
#sed -i '17,59{/DEBUG_RESP/d}' "$TmpFile"

#Remove line head [ *** ]:
sed -i 's/\[\S*\]: //g' "$TmpFile"

#Remove Key Send
sed -i '/sent.*KEY_STATE_SIG:/ s/:.*/:/g' "$TmpFile"


#Mark DEBUG_RESP_SIG AUDIO_MUTE_RESP_SIG RECV KEY_STATE_SIG lines
sed -i '/DEBUG_RESP_SIG/ s/$/FFF/' "$TmpFile"
sed -i '/AUDIO_MUTE_RESP_SIG/ s/$/111/' "$TmpFile"
sed -i '/recv.*KEY_STATE_SIG/ s/$/000/' "$TmpFile"

#exit

#Shorten DEBUG_RESP_SIG MSG
sed -i '/FFF/ s/0x2 0x1 0x0 0x0.*/0x2 0x1/g' "$TmpFile"

#Shorten AUDIO_MUTE_RESP_SIG MSG
sed -i '/111/ s/0x2 0x1 0x0 0x0.*/0x2 0x1/g' "$TmpFile"

#Shorten RECV KEY_STATE_SIG MSG  {word count}
sed -i '/000/ s/0x0.\{18,20\}000//g' "$TmpFile"

#exit

#Replace send & recv with CMDSEND CMDRECV
sed -i -e 's/^s\S* \S*:/CMDSEND\t=/g' -e 's/^r\S* \S*:/CMDRECV\t=/g' "$TmpFile"

#Add \n before CMDSEND \n CMDRECV
sed -i -e '/CMDSEND/i\\n' -e '/CMDRECV/a\\' "$TmpFile"

#exit

#Copy Line from Config
line=""
count=1
for i in {1..47} ;
do


line=$(sed -n "${count}p" $ConfigFile );
sed -i "${count}c ${line}" $TmpFile ;

count=$[count+1] ;

line="CMDID\t=${i}";
sed -i "${count}c ${line}" $TmpFile ;

count=$[count+4] ;

done

#sleep 10

#Add CMDID and \n
#sed -i -e '/CMDSEND/i\CMDID\t=' -e '/CMDRECV/a\\' "$TmpFile"

#Add ID Number
#for i in {1.."$CMDNumber"}; do sed -i '0,/CMDID\t=$/ s/CMDID\t=$/&'${i}'/' "TmpFile" ; done

#Add [] before CMDID
#sed i '/CMDID/i\\[\]' "TmpFile"

#Load Name from Config


#Seperate Command
#sed -i 's/SIG:/SIG:\n/g' "ResultFile"


























