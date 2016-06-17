#include "xl_common.h"
#include "cJSON.h"

int convert_jsonp_to_json (char * jsonp_name,char * json_name);
int parse_update(char * filename);


/*
{

"PeerID":"String",

"PeerIP":"String",

"PeerPort":"String",

"DeviceID":"String", #设备的sn号

"UploadConnections":Int, #当前上传链接数

"DownloadConnections":Int, #当前下行链接数

"UploadSpeed":Int, #当前上行速度

"DownloadSpeed":Int, #当前下载速度

"MaxUploadSpeed":Int, #最大上行速度

"MaxDownloadSpeed":Int, #最大下行速度

"timestamps":Int, #5分钟的总上传流量

"token":"String",

"version":"plugin_version", #固件版本

"DownloadFlow":Int,   #

"DownToken":"String"  #

"NewFileCount":Int,  #新增下载的文件数，以已经下载完成为准 

"TotalFileCount":Int,  #赚钱宝当前内存中总共包含的文件数

"HitCount":Int,           #被sdk命中的文件数，不包含新下载的文件

"M3U8List":[

{"<m3u8_value0>":int},  #  "M3U8"的值：此M3U8的当前连接数

{"<m3u8_value1>:int},

...,

{"<m3u8_valueN>:int},

]

}
*/

int test1_create_tracker_report_json(char * buffer)
{
    cJSON *root,*root1 ,*fmt,*img,*thm,*fld;
    char *out;
    int i;
    
    fld = cJSON_CreateObject();
    cJSON_AddItemToObject(fld,"name",cJSON_CreateString("Jack"));
    cJSON_Delete(fld);
    
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root,"name",cJSON_CreateString("Jack"));
    
    root1 = cJSON_CreateArray();
    
#if 1
    for (i=0;i<5;i++)
    {
        fld = cJSON_CreateObject();
        cJSON_AddItemToArray(root1,fld);
        char tmp[8] = {0};
        sprintf(tmp,"item %d",i);
        cJSON_AddItemToObject(fld,tmp,cJSON_CreateNumber(i));
    }
#endif
    cJSON_AddItemReferenceToObject(root,"array",root1);
    out = cJSON_Print(root);
    cJSON_Delete(root);
    printf("%s \n ",out);
    free(out);
    return;
}









int main(int argc,char * argv[])
{
    if (argc != 2)
    {
        printf (" The argument request more items\n");
        return -1;
    }
    int nRet = 0;
#if 0
    char filename[256] = {0};
    char json_name[256] = {0};
    strcpy(filename,argv[1]);
    printf("Input filename %s \n",filename);
    sprintf(json_name,"%s_js",filename);

    nRet = convert_jsonp_to_json (filename,json_name); 
    nRet = parse_update(json_name);
    
    printf("main return %d \n",nRet);
#endif
    char buffer[1024] = {0};
    test1_create_tracker_report_json(buffer);
    return nRet;

}

int convert_jsonp_to_json (char * jsonp_name,char * json_name)
{
    if (NULL == json_name || NULL == jsonp_name)
    {
        printf("unknow file name \n");
        return -1;
    }

    FILE * jsonp_fd = NULL;
    FILE * json_fd = NULL;
    char buf[1024] = {0};
    char * pre_ch = NULL;
    char * last_ch = NULL;
    jsonp_fd = fopen(jsonp_name,"r");
    if (NULL == jsonp_fd)
    {
        printf("Can't open file %s line %d \n",json_name,__LINE__);
        return -1;
    }
    
    json_fd = fopen(json_name,"w");
    if (NULL == json_fd)
    {
        printf("Can't open file %s line %d strerr %s \n",json_name,__LINE__,strerror(errno));
        fclose(jsonp_fd);
        return -1;
    }

    while(fgets(buf,sizeof(buf),jsonp_fd))
    {
        pre_ch =  strstr(buf,"{");
        last_ch =  strstr(buf,"}");
        if (pre_ch != NULL)
        {
            fwrite (pre_ch,strlen(pre_ch),1,json_fd);
            memset(buf,0,sizeof(buf));
            continue;
        }
        if (last_ch != NULL) 
        {
            fwrite (buf,strlen(buf)-strlen(last_ch)+1,1,json_fd);
            memset(buf,0,sizeof(buf));
            continue;
        }

        fwrite (buf,strlen(buf),1,json_fd);
        memset(buf,0,sizeof(buf));
    }

    
    fclose(json_fd);
    fclose(jsonp_fd);
    return 0;
}

int parse_update(char * filename)
{
	char json[2048] = {0};
	cJSON * root;
    cJSON * product;
	int value_int;
	int value_bool;
	FILE * fd = NULL;
    char *tmp_json = "{\"version\": \"V1.5.0088\",     \"product\": [\"RS1403\"],     \"description\": \"1.解决某些情况下samba配置丢失导致无法访问的问题\n2.解决某些情况下恢复出厂后迅雷账号无法绑定的问题\n3.网络错误页面增加开关选项\n4.解决若干稳定性问题\n5.解决某些浏览器造成的路由页面卡顿问题\n6.打包Android APP版本1.1.1235\",     \"url\": \"http://down.sandai.net/peiluyou/XL_ROM_V1.5.0088.img\",     \"md5\": \"78a2677c7151e447b2a643ff1e4dfe08\",     \"size\": 24346688,     \"forcedUpdate\": true }";	
	fd = fopen(filename,"r");
	if (NULL == fd)
	{
		printf("Can't open file %s \n",filename);
		return -1;
	}
    
    int ch = 0;
    char * strPtr = json;
    while ( !feof(fd) )
    {
        ch = fgetc(fd);   
        if(ch != EOF)
        { 
            *strPtr=ch;  
            strPtr++; 
        }
    }
    

    if (NULL != fd)
    {
        fclose(fd);
    }
    printf("Get the json file stream ok\n");

    printf("json stream \n  %s \n",json);
    if (0)
    {
        memset(json,0,sizeof(json));
        memcpy(json,tmp_json,strlen(tmp_json));
    }
	root = cJSON_Parse(json); 
    char * value_version = cJSON_GetObjectItem(root,"version")->valuestring;
    value_int = cJSON_GetObjectItem(root,"size")->valueint; 
	value_bool = cJSON_GetObjectItem(root,"forcedUpdate")->valueint; 
	char * value_model ;
    product = cJSON_GetObjectItem(root,"product"); 
    int array_size = cJSON_GetArraySize(product);
    value_model = cJSON_GetArrayItem(product,0)->valuestring;
	printf( "size is %d\n", value_int );
	printf( "product array size is %d\n", array_size);
	printf( "product array is %s\n", value_model);
	printf( "forceUpdate %d\n", value_bool );
	printf( "verson is  %s\n", value_version );
	cJSON_Delete(root);
    return 0; 
}


int parse_demo(int argc,char * argv[])
{
	char * json = "{ \"json\" : { \"id\":1, \"nodeId\":11, \"deviceId\":111, \"deviceName\":\"aaa\", \"ieee\":\"01212\", \"ep\":\"1111\", \"type\":\"bbb\" }}";
	char * json1 = "{\"id\":1, \"nodeId\":11, \"deviceId\":111, \"deviceName\":\"aaa\"}";
	cJSON * root;
	cJSON * format;
	int value_int;
	char * value_string;

	root = cJSON_Parse(json); 
	format = cJSON_GetObjectItem(root,"json");   
	value_int = cJSON_GetObjectItem(format,"nodeId")->valueint; 
	value_string = cJSON_GetObjectItem(format,"ieee")->valuestring; 
	printf( "%d\n", value_int );
	printf( "%s\n", value_string );
	cJSON_Delete(root);
  
	root = cJSON_Parse(json1); 
	value_int = cJSON_GetObjectItem(root,"id")->valueint; 
	value_string = cJSON_GetObjectItem(root,"deviceName")->valuestring; 
	printf( "%d\n", value_int );
	printf( "%s\n", value_string );
	cJSON_Delete(root);
    return 0;
}
