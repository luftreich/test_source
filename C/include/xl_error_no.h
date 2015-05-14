#ifndef _XL_ERROT_NO_H_
#define _XL_ERROT_NO_H_
#ifdef __cplusplus
extern "C"{
#endif
#define RET_OK        0
#define XL_ENABLE     1
#define XL_DISABLE    0

typedef enum tagEN_ERROR_NUM
{
    EN_OK = 0,
    EN_ERROR_FAILED,
    EN_ERR_PARA,
    EN_NULL_P,
    EN_ERROR_UNKNOW,
    EN_ERROR_NO_MEMORY,
    EN_ERROR_MAX
}EN_ERROR_NUM;

#ifdef __cplusplus
}
#endif
#endif

