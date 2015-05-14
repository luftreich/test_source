/**********************************************************
Copyright (C), 2003-2013, Xunlei Network Tech. Ltd.
File Name:xl_decode.c
Author:     chenjianhua
Version:    1.0
Date:        2013-11-11
Description:  实现对协议编码和解码
History:     
    1. Date:2013-11-11
       Author:chenjianhua
       Modification:create file 
***********************************************************/
#include "xl_common.h"
#include "xl_typedef.h"


/***********************************************************
Function:  xl_decode_u16
Description: u16解码
Input:     CHAR *pcBuf, 
           USHORT *pusData   
Output:    无
Return:     // 函数返回值的说明
Others:     // 其他需要说明的内容
History:      // 修改历史记录列表,每条修改记录应包括修改日期、
              //修改及修改内容简述
    1. Date: 2013.11.12
       Author: chenjianhua
       Modification: 生成函数
    2. ...
************************************************************/
ULONG xl_decode_u16(CHAR *pcBuf, USHORT *pusData)
{
    CHAR *pcIn    = pcBuf;
    USHORT usData = 0;
	
	
    usData = pcIn[0];
	usData = (usData<<8) + pcIn[1];
    *pusData = usData;
	
    return 0;
}

/***********************************************************
Function:  xl_decode_u32
Description: u32解码
Input:     CHAR *pcBuf, 
           DWORD *pulData
Output:    无
Return:     // 函数返回值的说明
Others:     // 其他需要说明的内容
History:      // 修改历史记录列表,每条修改记录应包括修改日期、
              //修改及修改内容简述
    1. Date: 2013.11.12
       Author: chenjianhua
       Modification: 生成函数
    2. ...
************************************************************/
ULONG xl_decode_u32(CHAR *pcBuf, ULONG *pulData)
{
    UCHAR *pcIn = (UCHAR *)pcBuf;
	ULONG ulData = 0;
    	
	
	ulData = pcIn[0];
	printf("uldata 0x%x , line %d ,char 0x%x\n",ulData,__LINE__,pcIn[0]);

    ulData = (ulData<<8) + pcIn[1];
	printf("uldata 0x%x , line %d,char 0x%x \n",ulData,__LINE__,(char)pcIn[1]);
	
    ulData = (ulData<<8) + pcIn[2];
	printf("uldata 0x%x , line %d,char 0x%x \n",ulData,__LINE__,pcIn[2]);
	
    ulData = (ulData<<8) + pcIn[3];
	printf("uldata 0x%x , line %d,char 0x%x \n",ulData,__LINE__,pcIn[3]);
	
    *pulData = ulData;

	return 0;
}

/***********************************************************
Function:  xl_decode_u64
Description: DULONG解码
Input:     CHAR *pcBuf, 
           DULONG *pulData
Output:    无
Return:     // 函数返回值的说明
Others:     // 其他需要说明的内容
History:      // 修改历史记录列表,每条修改记录应包括修改日期、
              //修改及修改内容简述
    1. Date: 2013.11.12
       Author: chenjianhua
       Modification: 生成函数
    2. ...
************************************************************/
ULONG xl_decode_u64(CHAR *pcBuf, DULONG *pdulData)
{
    CHAR *pcIn = pcBuf;
	DULONG dulData = 0;
	ULONG i = 0;
	
	
	dulData = pcIn[0];
	for (i = 1; i < 8; i++)
    {
        dulData = (dulData<<8) + pcIn[i];
    }
	*pdulData = dulData;

	return 0;
}

/***********************************************************
Function:  xl_decode_str
Description: str_decoder
Input:       CHAR *pcBuf,    
Output:     pcData
            pulLen
Return: 
Others:     
History:      // 修改历史记录列表,每条修改记录应包括修改日期、
              //修改及修改内容简述
    1. Date: 2013.11.12
       Author: chenjianhua
       Modification: 生成函数
    2. ...
************************************************************/
ULONG xl_decode_str(CHAR *pcBuf, CHAR *pcData, ULONG *pulLen)	
{
    ULONG ulLen = 0;
    CHAR *pcIn  = pcBuf;
	CHAR *pcOut = pcData;


    (VOID)xl_decode_u32(pcIn, &ulLen);
    memcpy(pcOut, &pcIn[4], ulLen);
	pcOut[ulLen] = '\0';
    *pulLen = ulLen + 4;

	return 0;
}


/***********************************************************
Function:  xl_decode_raw
Description: 解码raw数据
Input:       CHAR *pcBuf:    解码缓冲区 
             CHAR *pcData:   解码出来的数据
             ULONG pulLen:   解码完成后缓冲区要偏移的大小
Output:    
Return:    错误码定义
Others:     
History:  
************************************************************/
ULONG xl_decode_raw(CHAR *pcBuf, CHAR *pcData, ULONG *pulLen)	
{
    ULONG ulLen = 0;
    CHAR *pcIn  = pcBuf;
    CHAR *pcOut = pcData;


    (VOID)xl_decode_u32(pcIn, &ulLen);
    memcpy(pcOut, &pcIn[4], ulLen);
    *pulLen = ulLen + sizeof(U32);

    return 0;
}

/***********************************************************
Function:  xl_decode_str_limit
Description: str解码
Input:       CHAR *pcBuf,         待解码数据             
             ULONG ulMaxLen       待解码的字符串最大长度
Output:      CHAR *pcData,        解码出来的数据
             ULONG *pulLen        返回decode后，偏移长度
Return:     // 函数返回值的说明
Others:     // 其他需要说明的内容
History:      // 修改历史记录列表,每条修改记录应包括修改日期、
              //修改及修改内容简述
    1. Date: 2013.11.12
       Author: chenjianhua
       Modification: 生成函数
    2. ...
************************************************************/
ULONG xl_decode_str_limit(CHAR *pcBuf, UCHAR *pcData,
                             ULONG ulMaxLen, ULONG *pulLen)	
{
    ULONG ulLen = 0;
    CHAR *pcIn  = pcBuf;
	UCHAR *pcOut = pcData;


    (VOID)xl_decode_u32(pcIn, &ulLen);
	if (ulLen > ulMaxLen)
	{
	    XL_DEBUG(EN_PRINT_ERROR,
			    "xl_decode_str_limit fail, ulLen:%d, expect:%d\n",
			    ulLen, ulMaxLen);
	    return -1;
	}
    memcpy(pcOut, &pcIn[4], ulLen);
	pcOut[ulLen] = '\0';
    *pulLen = ulLen + 4;

	return 0;
}


int main(void)
{
    int value = 0x89674523;
    int value1 = 0;

    xl_decode_u32((char *)&value,(ULONG *)&value1);
    printf("decode 0x%x  is 0x%x \n",value,value1);
    
    value = 0xffffffff;
    xl_decode_u32((char *)&value,(ULONG *)&value1);
    printf("decode 0x%x  is 0x%x \n",value,value1);
    return 0;
}
