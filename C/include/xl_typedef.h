#ifndef _XL_TYPEDEF_H_
#define _XL_TYPEDEF_H_
#ifdef __cplusplus 
extern "C"{
#endif

#include "xl_error_no.h"
#ifndef DEFINED_BOOL
#define DEFINED_BOOL
typedef int BOOL;
#endif

/*Common unsigned types */
#ifndef DEFINED_U8
#define DEFINED_U8
typedef unsigned char  UCHAR;
typedef unsigned char  U8;
#endif

#ifndef DEFINED_U16
#define DEFINED_U16
typedef unsigned short USHORT;
typedef unsigned short U16;
#endif

#ifndef DEFINED_U32
#define DEFINED_U32
typedef unsigned int   ULONG;
typedef unsigned int   U32;
#endif



#ifndef DEFINED_U64
#define DEFINED_U64
typedef unsigned long long U64;
typedef unsigned long long   DULONG;
#endif

/* Common signed types */
#ifndef DEFINED_VOID
#define DEFINED_VOID
typedef void VOID;
#endif

#ifndef DEFINED_S8
#define DEFINED_S8
typedef char  CHAR;
typedef signed char  S8;
#endif

#ifndef DEFINED_S16
#define DEFINED_S16
typedef signed short SHORT;
typedef signed short S16;
#endif

#ifndef DEFINED_S32
#define DEFINED_S32
typedef signed int   LONG;
typedef signed int   S32;
#endif

#ifndef DEFINED_S32
#define DEFINED_S32
typedef signed long   SLONG;
#endif

#ifndef DEFINED_S64
#define DEFINED_S64
typedef signed long long S64;
#endif

#ifndef DEFINED_STR
#define DEFINED_STR
typedef char *  STR;
#endif

#ifndef DEFINED_HANDLE
#define DEFINED_HANDLE
typedef U32 * HANDLE;
#endif

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#define _DESC(x)      (1)
#define XL_GETBIT(n, i)     (((n)>>(i))&0x1)   
#define XL_SETBIT(n, i)     ((n)|= (1<<(i))) 

#define XL_CHECK_NULL(ptr) \
{\
    if (NULL == ptr)\
    {\
        XL_DEBUG(EN_PRINT_ERROR, "error NULL ptr, %s:%d\r\n",__FILE__, __LINE__);\
        return EN_NULL_P;\
    }\
}

#define XL_CHECK_PARA(contion) \
{\
    if (contion)\
    {\
        XL_DEBUG(EN_PRINT_ERROR, "error para, %s:%d\r\n",__FILE__, __LINE__);\
        return EN_ERR_PARA;\
    }\
}

#define XL_CHECK_RET(ulRet) \
{\
    if (ulRet != RET_OK)\
    {\
        XL_DEBUG(EN_PRINT_ERROR, "error ret:%d, %s:%d\r\n", ulRet, __FILE__, __LINE__);\
        return EN_ERR_PARA;\
    }\
}


#define XL_CHECK_ERRNO(err_num) \
{\
    if (err_num != EN_MSG_NO_ERROR)\
    {\
        XL_DEBUG(EN_PRINT_ERROR, "errno:%d, %s:%d", err_num, __FILE__, __LINE__);\
    }\
}


#define XL_ASSERT(cond) \
{   \
    if(!(cond)) \
    {   \
        XL_DEBUG(EN_PRINT_ERROR,"XL ASSERT Error \n");\
    }   \
}

#ifdef __cplusplus
}
#endif
#endif


