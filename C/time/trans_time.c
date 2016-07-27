int demo_0()
{
    time_t timetTime;
    struct tm *pTmTime;
    char   szTime[24] = {0};
    //获取当前系统时间
    timetTime = time(NULL);
    printf("timetTime=%d\n", timetTime);
    //time_t 结构转换成tm结构
    pTmTime = localtime(&timetTime);
    //验证tm类型数据是否正确

    snprintf(szTime, sizeof(szTime)-1,
     "%d-%02d-%02d %02d:%02d:%02d",
        pTmTime->tm_year+1900,
        pTmTime->tm_mon+1,
        pTmTime->tm_mday,
        pTmTime->tm_hour,
        pTmTime->tm_min,
        pTmTime->tm_sec);
    printf("szTime=%s\n", szTime);
    return 0；
}
int demo_1()
{
  //构建tm结构体
 tmTime.tm_year = year-1900;
 tmTime.tm_mon = month-1;
 tmTime.tm_mday = day;
 tmTime.tm_hour = hour;
 tmTime.tm_min  = min;
 tmTime.tm_sec  = sec;
//tm结构转换成time_t结构
 timetTime = mktime(&tmTime);
 printf("timetTime=%d\n", timetTime);
 //用ctime函数校验下，上面转换是否正确

 printf("After transfer, time is: %s\n", ctime((time_t*)&timetTime));
 return 0;

}
