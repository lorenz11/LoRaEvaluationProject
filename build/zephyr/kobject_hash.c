/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: /usr/bin/gperf --output-file kobject_hash_preprocessed.c kobject_hash.gperf  */
/* Computed positions: -k'1-2' */

#line 6 "kobject_hash.gperf"

#include <kernel.h>
#include <toolchain.h>
#include <syscall_handler.h>
#include <string.h>
#line 12 "kobject_hash.gperf"
struct z_object;
static struct z_futex_data futex_data[1] = {
Z_FUTEX_DATA_INITIALIZER(futex_data[0])};
static uint8_t Z_GENERIC_SECTION(.priv_stacks.noinit)  __aligned(Z_KERNEL_STACK_OBJ_ALIGN) priv_stacks[2][Z_KERNEL_STACK_LEN(CONFIG_PRIVILEGED_STACK_SIZE)];
static struct z_stack_data stack_data[2] = {
	{ 4096, (uint8_t *)(&priv_stacks[0]) },
	{ 1024, (uint8_t *)(&priv_stacks[1]) }
};

#define TOTAL_KEYWORDS 53
#define MIN_WORD_LENGTH 4
#define MAX_WORD_LENGTH 4
#define MIN_HASH_VALUE 0
#define MAX_HASH_VALUE 120
/* maximum key range = 121, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
/*ARGSUSED*/
static unsigned int
hash ( const char *str,  size_t len)
{
  static unsigned char asso_values[] =
    {
       10,  21,  36,  15,  40, 121,  16, 121, 121,  16,
       11,  20,  60, 121, 121, 121,  60, 121, 121, 121,
        7, 121, 121, 121,   6, 121, 121, 121, 121,  60,
      121, 121,   5, 121, 121,  55, 121, 121, 121, 121,
       50, 121, 121, 121, 121, 121, 121, 121,   1, 121,
      121, 121, 121, 121, 121, 121, 121, 121, 121, 121,
      121, 121, 121, 121,  61, 121, 121, 121, 121, 121,
      121, 121, 121, 121, 121, 121,  27, 121, 121, 121,
      121, 121, 121, 121, 121, 121, 121, 121,  51, 121,
      121, 121, 121, 121, 121, 121,   0, 121, 121, 121,
      121, 121, 121, 121,  56, 121, 121, 121, 121, 121,
      121, 121,  60, 121, 121, 121,  15,  60,  16, 121,
       55,  35, 121,   5,   0,  30, 121, 121,  16, 121,
      121, 121, 121, 121, 121, 121, 121, 121, 121, 121,
      121, 121, 121, 121,  45, 121, 121, 121, 121, 121,
      121, 121,  16, 121, 121, 121,  56, 121, 121, 121,
       55, 121, 121, 121,   5, 121, 121, 121,   2, 121,
      121, 121, 121, 121, 121, 121, 121, 121, 121, 121,
        2, 121, 121, 121,  50, 121, 121, 121,  61, 121,
      121, 121,  35, 121, 121, 121, 121, 121, 121, 121,
      121, 121, 121, 121, 121, 121, 121, 121,  41, 121,
      121, 121, 121, 121, 121, 121,  31, 121, 121, 121,
      121, 121, 121, 121, 121, 121, 121, 121,  60, 121,
      121, 121, 121, 121, 121, 121, 121, 121, 121, 121,
       26, 121, 121, 121, 121, 121, 121, 121,  20, 121,
      121, 121, 121, 121, 121, 121
    };
  return asso_values[(unsigned char)str[1]] + asso_values[(unsigned char)str[0]];
}

static unsigned char __unused lengthtable[] =
  {
     4,  4,  4,  0,  0,  4,  4,  4,  0,  0,  4,  4,  4,  0,
     0,  4,  4,  4,  0,  0,  4,  4,  4,  0,  0,  4,  4,  4,
     0,  0,  4,  4,  4,  0,  0,  4,  4,  4,  0,  0,  4,  4,
     0,  0,  0,  4,  4,  0,  0,  0,  4,  4,  0,  0,  0,  4,
     4,  0,  0,  0,  4,  4,  0,  0,  0,  4,  4,  0,  0,  0,
     4,  4,  0,  0,  0,  4,  4,  0,  0,  0,  4,  4,  0,  0,
     0,  4,  4,  0,  0,  0,  4,  4,  0,  0,  0,  4,  4,  0,
     0,  0,  4,  0,  0,  0,  0,  4,  0,  0,  0,  0,  4,  0,
     0,  0,  0,  4,  0,  0,  0,  0,  4
  };

static struct z_object wordlist[] =
  {
#line 62 "kobject_hash.gperf"
    {(char *)0x0000000020007c60, {}, K_OBJ_SEM, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 60 "kobject_hash.gperf"
    {(char *)0x0000000020007c30, {}, K_OBJ_MEM_SLAB, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 65 "kobject_hash.gperf"
    {(char *)0x0000000020007ca8, {}, K_OBJ_SEM, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
    {}, {},
#line 54 "kobject_hash.gperf"
    {(char *)0x0000000020007b60, {}, K_OBJ_DRIVER_GPIO, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 53 "kobject_hash.gperf"
    {(char *)0x0000000020007b30, {}, K_OBJ_DRIVER_ENTROPY, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 59 "kobject_hash.gperf"
    {(char *)0x0000000020007c14, {}, K_OBJ_MEM_SLAB, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
    {}, {},
#line 43 "kobject_hash.gperf"
    {(char *)0x0000000020006000, {}, K_OBJ_THREAD_STACK_ELEMENT, 0, { .stack_data = &stack_data[0] }},
#line 52 "kobject_hash.gperf"
    {(char *)0x0000000020007b18, {}, K_OBJ_DRIVER_UART, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 38 "kobject_hash.gperf"
    {(char *)0x00000000200014a4, {}, K_OBJ_SEM, 0, { .unused = 0 }},
    {}, {},
#line 22 "kobject_hash.gperf"
    {(char *)0x0000000020000020, {}, K_OBJ_TIMER, 0, { .unused = 0 }},
#line 34 "kobject_hash.gperf"
    {(char *)0x0000000020000a20, {}, K_OBJ_THREAD, 0, { .thread_id = 5 }},
#line 45 "kobject_hash.gperf"
    {(char *)0x00000000200074b4, {}, K_OBJ_SEM, 0, { .unused = 0 }},
    {}, {},
#line 21 "kobject_hash.gperf"
    {(char *)0x0000000020000000, {}, K_OBJ_FUTEX, 0, { .futex_data = &futex_data[0] }},
#line 31 "kobject_hash.gperf"
    {(char *)0x0000000020000620, {}, K_OBJ_THREAD, 0, { .thread_id = 3 }},
#line 49 "kobject_hash.gperf"
    {(char *)0x0000000020007618, {}, K_OBJ_POLL_SIGNAL, 0, { .unused = 0 }},
    {}, {},
#line 58 "kobject_hash.gperf"
    {(char *)0x0000000020007bf8, {}, K_OBJ_MEM_SLAB, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 68 "kobject_hash.gperf"
    {(char *)0x0000000020007cf0, {}, K_OBJ_QUEUE, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 61 "kobject_hash.gperf"
    {(char *)0x0000000020007c4c, {}, K_OBJ_MUTEX, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
    {}, {},
#line 24 "kobject_hash.gperf"
    {(char *)0x00000000200000f8, {}, K_OBJ_THREAD, 0, { .thread_id = 1 }},
#line 67 "kobject_hash.gperf"
    {(char *)0x0000000020007cd8, {}, K_OBJ_QUEUE, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 33 "kobject_hash.gperf"
    {(char *)0x0000000020000980, {}, K_OBJ_THREAD, 0, { .thread_id = 6 }},
    {}, {},
#line 66 "kobject_hash.gperf"
    {(char *)0x0000000020007cc0, {}, K_OBJ_QUEUE, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 69 "kobject_hash.gperf"
    {(char *)0x0000000020007d18, {}, K_OBJ_QUEUE, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 25 "kobject_hash.gperf"
    {(char *)0x0000000020000198, {}, K_OBJ_THREAD, 0, { .thread_id = 2 }},
    {}, {},
#line 57 "kobject_hash.gperf"
    {(char *)0x0000000020007bc0, {}, K_OBJ_TIMER, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 47 "kobject_hash.gperf"
    {(char *)0x00000000200074f0, {}, K_OBJ_SEM, 0, { .unused = 0 }},
    {}, {}, {},
#line 64 "kobject_hash.gperf"
    {(char *)0x0000000020007c90, {}, K_OBJ_SEM, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 35 "kobject_hash.gperf"
    {(char *)0x0000000020000ac0, {}, K_OBJ_THREAD, 0, { .thread_id = 7 }},
    {}, {}, {},
#line 56 "kobject_hash.gperf"
    {(char *)0x0000000020007b90, {}, K_OBJ_DRIVER_SPI, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 32 "kobject_hash.gperf"
    {(char *)0x00000000200006c0, {}, K_OBJ_THREAD, 0, { .thread_id = 4 }},
    {}, {}, {},
#line 63 "kobject_hash.gperf"
    {(char *)0x0000000020007c78, {}, K_OBJ_SEM, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 46 "kobject_hash.gperf"
    {(char *)0x00000000200074d0, {}, K_OBJ_SEM, 0, { .unused = 0 }},
    {}, {}, {},
#line 55 "kobject_hash.gperf"
    {(char *)0x0000000020007b78, {}, K_OBJ_DRIVER_GPIO, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 23 "kobject_hash.gperf"
    {(char *)0x0000000020000058, {}, K_OBJ_THREAD, 0, { .thread_id = 0 }},
    {}, {}, {},
#line 40 "kobject_hash.gperf"
    {(char *)0x0000000020002070, {}, K_OBJ_SEM, 0, { .unused = 0 }},
#line 27 "kobject_hash.gperf"
    {(char *)0x0000000020000358, {}, K_OBJ_QUEUE, 0, { .unused = 0 }},
    {}, {}, {},
#line 44 "kobject_hash.gperf"
    {(char *)0x0000000020007000, {}, K_OBJ_THREAD_STACK_ELEMENT, 0, { .stack_data = &stack_data[1] }},
#line 28 "kobject_hash.gperf"
    {(char *)0x000000002000039c, {}, K_OBJ_QUEUE, 0, { .unused = 0 }},
    {}, {}, {},
#line 72 "kobject_hash.gperf"
    {(char *)0x0000000020007d90, {}, K_OBJ_QUEUE, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 36 "kobject_hash.gperf"
    {(char *)0x0000000020000b9c, {}, K_OBJ_MEM_SLAB, 0, { .unused = 0 }},
    {}, {}, {},
#line 73 "kobject_hash.gperf"
    {(char *)0x0000000020007db8, {}, K_OBJ_QUEUE, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 37 "kobject_hash.gperf"
    {(char *)0x0000000020000bbc, {}, K_OBJ_MEM_SLAB, 0, { .unused = 0 }},
    {}, {}, {},
#line 51 "kobject_hash.gperf"
    {(char *)0x00000000200079b8, {}, K_OBJ_SEM, 0, { .unused = 0 }},
#line 71 "kobject_hash.gperf"
    {(char *)0x0000000020007d68, {}, K_OBJ_QUEUE, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
    {}, {}, {},
#line 50 "kobject_hash.gperf"
    {(char *)0x00000000200079a0, {}, K_OBJ_SEM, 0, { .unused = 0 }},
#line 70 "kobject_hash.gperf"
    {(char *)0x0000000020007d40, {}, K_OBJ_QUEUE, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
    {}, {}, {},
#line 29 "kobject_hash.gperf"
    {(char *)0x00000000200004a0, {}, K_OBJ_QUEUE, 0, { .unused = 0 }},
#line 26 "kobject_hash.gperf"
    {(char *)0x0000000020000270, {}, K_OBJ_QUEUE, 0, { .unused = 0 }},
    {}, {}, {},
#line 30 "kobject_hash.gperf"
    {(char *)0x00000000200004e4, {}, K_OBJ_QUEUE, 0, { .unused = 0 }},
    {}, {}, {}, {},
#line 42 "kobject_hash.gperf"
    {(char *)0x0000000020002328, {}, K_OBJ_SEM, 0, { .unused = 0 }},
    {}, {}, {}, {},
#line 39 "kobject_hash.gperf"
    {(char *)0x0000000020001db8, {}, K_OBJ_SEM, 0, { .unused = 0 }},
    {}, {}, {}, {},
#line 41 "kobject_hash.gperf"
    {(char *)0x0000000020002310, {}, K_OBJ_SEM, 0, { .unused = 0 }},
    {}, {}, {}, {},
#line 48 "kobject_hash.gperf"
    {(char *)0x000000002000750c, {}, K_OBJ_QUEUE, 0, { .unused = 0 }}
  };

static inline struct z_object *
z_object_lookup ( const char *str,  size_t len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
       unsigned int key = hash((const char *)&str, len);

      if (key <= MAX_HASH_VALUE)
        if (len == sizeof(void *))
          {
             const char *s = wordlist[key].name;

            if (str == s)
              return &wordlist[key];
          }
    }
  return 0;
}
#line 74 "kobject_hash.gperf"

struct z_object *z_object_gperf_find(const void *obj)
{
    return z_object_lookup((const char *)obj, sizeof(void *));
}

void z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, void *context)
{
    int i;

    for (i = MIN_HASH_VALUE; i <= MAX_HASH_VALUE; i++) {
        if (wordlist[i].name != NULL) {
            func(&wordlist[i], context);
        }
    }
}

#ifndef CONFIG_DYNAMIC_OBJECTS
struct z_object *z_object_find(const void *obj)
	ALIAS_OF(z_object_gperf_find);

void z_object_wordlist_foreach(_wordlist_cb_func_t func, void *context)
	ALIAS_OF(z_object_gperf_wordlist_foreach);
#endif

Z_GENERIC_SECTION(.kobject_data.data) uint8_t _thread_idx_map[2] = { 0x0,  0xff, };
