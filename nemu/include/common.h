#ifndef __COMMON_H__
#define __COMMON_H__

// #define DEBUG
// #define DIFF_TEST

/* You will define this macro in PA2 */
#define HAS_IOE

#include "debug.h"
#include "macro.h"

#include <stdint.h>
#include <assert.h>
#include <string.h>

//如果这里定义了DIFF_TEST，运行nemu时会和qemu进行比较，如果9个寄存器状态不同会被输出。
#define DIFF_TEST
//如果这里定义了DEBUG,运行nemu会有额外的输出
// #define DEBUG

typedef uint8_t bool;

typedef uint32_t rtlreg_t;

typedef uint32_t paddr_t;
typedef uint32_t vaddr_t;

typedef uint16_t ioaddr_t;

#define false 0
#define true 1

#endif
