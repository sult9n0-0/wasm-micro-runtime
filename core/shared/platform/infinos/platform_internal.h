#ifndef _PLATFORM_INTERNAL_H
#define _PLATFORM_INTERNAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#ifndef BH_PLATFORM_INFINOS
#define BH_PLATFORM_INFINOS
#endif

typedef uintptr_t korp_tid;
typedef struct {
    volatile uint32_t state;
} korp_mutex;

/* Opaque placeholders: synchronization is not implemented yet. */
typedef struct { uintptr_t unused; } korp_cond;
typedef struct { uintptr_t unused; } korp_rwlock;
typedef struct { uintptr_t unused; } korp_sem;

typedef int os_file_handle;
typedef int os_raw_file_handle;

/* Filesystem and polling types: interfaces only for now. */
typedef void *os_dir_stream;
typedef struct os_poll_file_handle os_poll_file_handle;
typedef unsigned int os_nfds_t;

/* InfinOS currently uses a 4 KiB memory page. */
static inline unsigned int
os_getpagesize(void)
{
    return 4096;
}

static inline os_file_handle
os_get_invalid_handle(void)
{
    return -1;
}

#endif
