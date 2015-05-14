#include "xl_common.h"
#include "cJSON.h"

int convert_jsonp_to_json (char * jsonp_name,char * json_name);
int parse_update(char * filename);
int main(int argc,char * argv[])
{
    if (argc != 2)
    {
        printf (" The argument request more items\n");
        return -1;
    }

    char filename[256] = {0};
    char json_name[256] = {0};
    int nRet = 0;
    strcpy(filename,argv[1]);
    printf("Input filename %s \n",filename);
    sprintf(json_name,"%s_js",filename);

    nRet = convert_jsonp_to_json (filename,json_name); 
    nRet = parse_update(json_name);
    
    printf("main return %d \n",nRet);
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
