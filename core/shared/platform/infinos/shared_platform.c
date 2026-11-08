#include "platform_api_vmcore.h"

/*
 * InfinOS: single execution context, interpreter-only WAMR platform.
 */

int
bh_platform_init(void)
{
    return BHT_OK;
}

void
bh_platform_destroy(void)
{
}

void *
os_malloc(unsigned size)
{
    (void)size;
    return NULL;
}

void *
os_realloc(void *ptr, unsigned size)
{
    (void)ptr;
    (void)size;
    return NULL;
}

void
os_free(void *ptr)
{
    (void)ptr;
}

int
os_printf(const char *format, ...)
{
    (void)format;
    return 0;
}

int
os_vprintf(const char *format, va_list ap)
{
    (void)format;
    (void)ap;
    return 0;
}

uint64
os_time_get_boot_us(void)
{
    return 0;
}

uint64
os_time_thread_cputime_us(void)
{
    return 0;
}

korp_tid
os_self_thread(void)
{
    return 0;
}

uint8 *
os_thread_get_stack_boundary(void)
{
    return NULL;
}

void
os_thread_jit_write_protect_np(bool enabled)
{
    (void)enabled;
}

void
os_dcache_flush(void)
{
}

void
os_icache_flush(void *start, size_t len)
{
    (void)start;
    (void)len;
}

/* Mutexes */

int
os_mutex_init(korp_mutex *mutex)
{
    if (!mutex)
        return BHT_ERROR;

    __atomic_store_n(&mutex->state, 0, __ATOMIC_RELAXED);
    return BHT_OK;
}

int
os_mutex_destroy(korp_mutex *mutex)
{
    if (!mutex)
        return BHT_ERROR;

    return __atomic_load_n(&mutex->state, __ATOMIC_ACQUIRE) == 0
               ? BHT_OK : BHT_ERROR;
}

int
os_mutex_lock(korp_mutex *mutex)
{
    if (!mutex)
        return BHT_ERROR;

    while (__atomic_exchange_n(&mutex->state, 1,
                               __ATOMIC_ACQUIRE) != 0) {
        __asm__ volatile("yield" ::: "memory");
    }

    return BHT_OK;
}

int
os_mutex_unlock(korp_mutex *mutex)
{
    if (!mutex ||
        __atomic_load_n(&mutex->state, __ATOMIC_RELAXED) == 0)
        return BHT_ERROR;

    __atomic_store_n(&mutex->state, 0, __ATOMIC_RELEASE);
    return BHT_OK;
}

/*
 * InfinOS linear-memory arena.
 *
 * MicroKit maps this entire region RW at 0x05000000.
 * WAMR's mmap/mremap API is implemented as bounded allocation
 * within this single, statically mapped region.
 */
#define INFINOS_WASM_LINEAR_BASE ((uintptr_t)0x05000000UL)
#define INFINOS_WASM_LINEAR_SIZE ((size_t)0x04000000UL)

static size_t infinos_wasm_linear_used;

static bool
infinos_wasm_linear_range_valid(const void *addr, size_t size)
{
    uintptr_t base = INFINOS_WASM_LINEAR_BASE;
    uintptr_t start = (uintptr_t)addr;

    if (!addr || start < base)
        return false;

    if (start - base > infinos_wasm_linear_used)
        return false;

    return size <= infinos_wasm_linear_used - (size_t)(start - base);
}

void *
os_mmap(void *hint, size_t size, int prot, int flags, os_file_handle file)
{
    if (hint != NULL || size == 0 || size > INFINOS_WASM_LINEAR_SIZE)
        return NULL;

    if (flags != MMAP_MAP_NONE || file != os_get_invalid_handle())
        return NULL;

    if (prot != MMAP_PROT_NONE
        && prot != (MMAP_PROT_READ | MMAP_PROT_WRITE))
        return NULL;

    if (infinos_wasm_linear_used != 0)
        return NULL;

    memset((void *)INFINOS_WASM_LINEAR_BASE, 0, size);
    infinos_wasm_linear_used = size;

    return (void *)INFINOS_WASM_LINEAR_BASE;
}

void *
os_mremap(void *old_addr, size_t old_size, size_t new_size)
{
    if (old_addr != (void *)INFINOS_WASM_LINEAR_BASE)
        return NULL;

    if (old_size != infinos_wasm_linear_used)
        return NULL;

    if (new_size <= old_size || new_size > INFINOS_WASM_LINEAR_SIZE)
        return NULL;

    memset((void *)(INFINOS_WASM_LINEAR_BASE + old_size),
           0, new_size - old_size);

    infinos_wasm_linear_used = new_size;
    return old_addr;
}

void
os_munmap(void *addr, size_t size)
{
    if (addr != (void *)INFINOS_WASM_LINEAR_BASE
        || size != infinos_wasm_linear_used)
        return;

    memset(addr, 0, size);
    infinos_wasm_linear_used = 0;
}

int
os_mprotect(void *addr, size_t size, int prot)
{
    if (!infinos_wasm_linear_range_valid(addr, size))
        return -1;

    if (prot != MMAP_PROT_NONE
        && prot != (MMAP_PROT_READ | MMAP_PROT_WRITE))
        return -1;

    /*
     * Protection is logical only: MicroKit currently maps the
     * complete arena RW. WAMR interpreter bounds checks remain
     * responsible for WebAssembly linear-memory isolation.
     */
    return 0;
}

int
os_dumps_proc_mem_info(char *out, unsigned int size)
{
    (void)out;
    (void)size;
    return -1;
}
