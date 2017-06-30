/**************************************************************************************************
 > Project Name : MVR - mobile vacuum robot
 > File Name    : md5.h
 > Description  : This package supports both compile-time and run-time 
                  determination of CPU byte order
 > Author       : Yu Jie
 > Create Time  : 2017年06月20日
 > Modify Time  : 2017年06月20日
***************************************************************************************************/
#ifndef MD5_H
#define MD5_H
#include "MvrExport.h"

/*
 * This package supports both compile-time and run-time determination of CPU
 * byte order.  If ARCH_IS_BIG_ENDIAN is defined as 0, the code will be
 * compiled to run only on little-endian CPUs; if ARCH_IS_BIG_ENDIAN is
 * defined as non-zero, the code will be compiled to run only on big-endian
 * CPUs; if ARCH_IS_BIG_ENDIAN is not defined, the code will be compiled to
 * run on either big- or little-endian CPUs, but will run slightly less
 * efficiently on either one than if ARCH_IS_BIG_ENDIAN is defined.
 */
typedef unsigned char md5_byte_t;  /// 8-bit byte
typedef unsigned int md5_word_t;   /// 32-bit byte

/* Define the state of the MD5 Algorithm*/
typedef struct md5_state_s 
{
    md5_word_t count[2];	/* message length in bits, lsw first */
    md5_word_t abcd[4];		/* digest buffer */
    md5_byte_t buf[64];		/* accumulate block */
} md5_state_t;

// MPL (ActivMedia) taking out the extern since its only being used in c++
/*
#ifdef __cplusplus
extern "C" 
{
#endif
*/

// RH (ActivMedia/MobileRobots) Added "AREXPORT" symbol to all public functions. On Windows, this must be defined
// to either import or export the symbols to/from DLLs. On other platforms, it should be defined to be nothing.

/* Initialize the algorithm. */
MVREXPORT void md5_init(md5_state_t *pms);

/* Append a string to the message. */
MVREXPORT void md5_append(md5_state_t *pms, const md5_byte_t *data, int nbytes);

/* Finish the message and return the digest. */
MVREXPORT void md5_finish(md5_state_t *pms, md5_byte_t digest[16]);

/// MPL (ActivMedia) taking out the extern since its only being used in c++
/*
#ifdef __cplusplus
}  // end extern "C"
#endif
*/

#endif  // MD5_H
