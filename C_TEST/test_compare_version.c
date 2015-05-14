#include <stdio.h>
#include <string.h>
#define XL_DEBUG(a,fmt,arg...) printf(fmt,##arg) 

static int do_compare_img_version(char *current_version,char *img_version);
int  main(int argc ,char * argv[])
{
    if (argc <3 )
    {
        printf("argc less ,return \n");
        return;
    }

    char firstver[12]={0};
    char secondver[12]={0};

    strcpy(firstver,argv[1]);
    strcpy(secondver,argv[2]);

    do_compare_img_version(firstver,secondver);
}


static int do_compare_img_version(char *current_version,char *img_version)
{
    char current_ver[10]={0};
    char img_ver[10] = {0};
    int nCurVer = 0,nImgVer = 0;
    char * pCurVer=NULL;
    char * pImgVer = NULL;
    char * pTmp0 = NULL;
    char * pTmp1 = NULL;
    int nRet = -1;
    int i = 0,offset = 0,len0 = 0,len1=0;    
    if ((NULL == current_version) || (NULL == img_version))
    {
        XL_DEBUG(EN_PRINT_ERROR,"current_version or img_version is NULL \n");
        return -1;
    }
    
    XL_DEBUG(EN_PRINT_INFO,"Do compare version  \n");
    pCurVer = current_version;
    pImgVer = img_version;
    
    /*Compare the version number*/
    do {
        
        if(i == 0 )
        {
            pCurVer++;
            pImgVer++;
        }
        pTmp0 =  strstr(pCurVer,".");
        len0 = pTmp0-pCurVer;

        pTmp1 =  strstr(pImgVer,".");
        len1 = pTmp1-pImgVer;

        if (NULL == pTmp0 || NULL == pTmp1)
        {
            len0 = strlen(pCurVer);
            len1 = strlen(pImgVer);
        }
        memset(current_ver,0,sizeof(current_ver));
        memset(img_ver,0,sizeof(img_ver));

        memcpy(current_ver,pCurVer,len0);    
        memcpy(img_ver,pImgVer,len1);    
        
        pCurVer = pTmp0+1;
        pImgVer = pTmp1+1;

        nCurVer = atoi (current_ver);
        nImgVer = atoi (img_ver);
        if (nCurVer < nImgVer)
        {
            nRet = 0;
            break;
        }

        if (nCurVer > nImgVer)
        {
            nRet = -1;
            break;
        }

        if (nCurVer == nImgVer)
        {
            i++;
            continue;
        }
    }while(i<3);

    if (nRet == 0)
    {
        XL_DEBUG(EN_PRINT_INFO,"The img version is large than current version\n");
    }
    else
    {
        XL_DEBUG(EN_PRINT_INFO,"The img version is less than current version\n");
    }

    XL_DEBUG(EN_PRINT_INFO,"Do Compare Version exit\n");
    return nRet;
}
