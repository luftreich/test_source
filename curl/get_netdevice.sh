while :
do
echo " -------------------------------------------------------------------------------"
curl -X POST http://192.168.111.1:9999/getstatus?cvc=101'&'ov=23'&'device=huaweiske'&'imei=28383882'&'pdtid=2'&'interval=5'&'uid=121748275'&'statusId=36'&'routerId=xxhhll -d "" -v
echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> "
echo ""
sleep 1
done

