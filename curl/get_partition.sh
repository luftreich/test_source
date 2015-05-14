do_while ()
{
while :
do
    echo ""
    echo "-----------------------------------------------------------------"
    curl -X POST http://192.168.111.1:9999/getstatus?cvc=101'&'ov=23'&'device=huaweiske'&'imei=28383882'&'pdtid=457388'&'uid=41873004'&'statusId=27'&'routerId=xxhhll -d '' -v
    sleep 1
    echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
    echo ""
done
}

do_once ()
{
    curl -X POST http://192.168.111.1:9999/getstatus?cvc=101'&'ov=23'&'device=huaweiske'&'imei=28383882'&'pdtid=457388'&'uid=41873004'&'statusId=27'&'routerId=xxhhll -d '' -v
}

do_once

