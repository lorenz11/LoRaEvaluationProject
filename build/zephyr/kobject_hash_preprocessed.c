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
	{ 512, (uint8_t *)(&priv_stacks[1]) }
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
hash (register const char *str, register size_t len)
{
  static unsigned char asso_values[] =
    {
       10,  21,  30,  15,  55, 121,  11, 121, 121,  60,
       60,  20,  60, 121, 121, 121, 121, 121, 121, 121,
       46, 121, 121, 121,  60, 121, 121, 121, 121,   1,
      121, 121,  40, 121, 121,  11, 121, 121, 121, 121,
      121, 121, 121, 121, 121, 121, 121, 121,  11, 121,
      121, 121, 121, 121, 121, 121, 121, 121, 121, 121,
      121, 121, 121, 121,  46, 121, 121, 121, 121, 121,
      121, 121,  16, 121, 121, 121,   7, 121, 121, 121,
      121, 121, 121, 121, 121, 121, 121, 121,  51, 121,
      121, 121, 121, 121, 121, 121,  25, 121, 121, 121,
      121, 121, 121, 121,  41, 121, 121, 121, 121, 121,
      121, 121,   7, 121,  10,  55,  50, 121, 121,  50,
        1,   5,   0,  45, 121, 121, 121, 121,  60, 121,
      121, 121, 121, 121, 121, 121, 121, 121, 121, 121,
      121, 121, 121, 121,   0, 121, 121, 121, 121, 121,
      121, 121,  21, 121, 121, 121,  56, 121, 121, 121,
       35, 121, 121, 121, 121, 121, 121, 121,   2, 121,
      121, 121, 121, 121, 121, 121, 121, 121, 121, 121,
        2, 121, 121, 121,  30, 121, 121, 121,  61, 121,
      121, 121,  10, 121, 121, 121,  10, 121, 121, 121,
      121, 121, 121, 121, 121, 121, 121, 121,  31, 121,
      121, 121, 121, 121, 121, 121,  31, 121, 121, 121,
      121, 121, 121, 121, 121, 121, 121, 121,  40, 121,
      121, 121, 121, 121, 121, 121, 121, 121, 121, 121,
       26, 121, 121, 121, 121, 121, 121, 121,  50, 121,
      121, 121, 121, 121, 121, 121
    };
  return asso_values[(unsigned char)str[1]] + asso_values[(unsigned char)str[0]];
}

static unsigned char lengthtable[] =
  {
     4,  4,  4,  0,  0,  4,  4,  4,  0,  0,  4,  4,  4,  0,
     0,  4,  4,  4,  0,  0,  4,  4,  4,  0,  0,  4,  4,  4,
     0,  0,  4,  4,  4,  0,  0,  4,  4,  4,  0,  0,  4,  4,
     4,  0,  0,  4,  4,  0,  0,  0,  4,  4,  0,  0,  0,  4,
     4,  0,  0,  0,  4,  4,  0,  0,  0,  4,  4,  0,  0,  0,
     4,  4,  0,  0,  0,  4,  4,  0,  0,  0,  4,  4,  0,  0,
     0,  4,  4,  0,  0,  0,  4,  4,  0,  0,  0,  4,  0,  0,
     0,  0,  4,  0,  0,  0,  0,  4,  0,  0,  0,  0,  4,  0,
     0,  0,  0,  4,  0,  0,  0,  0,  4
  };

static struct z_object wordlist[] =
  {
#line 64 "kobject_hash.gperf"
    {"\220z\000 ", {}, K_OBJ_SEM, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 63 "kobject_hash.gperf"
    {"xz\000 ", {}, K_OBJ_SEM, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 65 "kobject_hash.gperf"
    {"\250z\000 ", {}, K_OBJ_SEM, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
    {""}, {""},
#line 56 "kobject_hash.gperf"
    {"\220y\000 ", {}, K_OBJ_DRIVER_SPI, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 55 "kobject_hash.gperf"
    {"xy\000 ", {}, K_OBJ_DRIVER_GPIO, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 61 "kobject_hash.gperf"
    {"Lz\000 ", {}, K_OBJ_MUTEX, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
    {""}, {""},
#line 66 "kobject_hash.gperf"
    {"\300z\000 ", {}, K_OBJ_QUEUE, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 60 "kobject_hash.gperf"
    {"0z\000 ", {}, K_OBJ_MEM_SLAB, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 45 "kobject_hash.gperf"
    {"\264r\000 ", {}, K_OBJ_SEM, 0, { .unused = 0 }},
    {""}, {""},
#line 57 "kobject_hash.gperf"
    {"\300y\000 ", {}, K_OBJ_TIMER, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 53 "kobject_hash.gperf"
    {"0y\000 ", {}, K_OBJ_DRIVER_ENTROPY, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 44 "kobject_hash.gperf"
    {"\000p\000 ", {}, K_OBJ_THREAD_STACK_ELEMENT, 0, { .stack_data = &stack_data[1] }},
    {""}, {""},
#line 21 "kobject_hash.gperf"
    {"\000\000\000 ", {}, K_OBJ_FUTEX, 0, { .futex_data = &futex_data[0] }},
#line 32 "kobject_hash.gperf"
    {"\300\006\000 ", {}, K_OBJ_THREAD, 0, { .thread_id = 4 }},
#line 41 "kobject_hash.gperf"
    {"0#\000 ", {}, K_OBJ_SEM, 0, { .unused = 0 }},
    {""}, {""},
#line 62 "kobject_hash.gperf"
    {"`z\000 ", {}, K_OBJ_SEM, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 68 "kobject_hash.gperf"
    {"\360z\000 ", {}, K_OBJ_QUEUE, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 42 "kobject_hash.gperf"
    {"H#\000 ", {}, K_OBJ_SEM, 0, { .unused = 0 }},
    {""}, {""},
#line 54 "kobject_hash.gperf"
    {"`y\000 ", {}, K_OBJ_DRIVER_GPIO, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 67 "kobject_hash.gperf"
    {"\330z\000 ", {}, K_OBJ_QUEUE, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 39 "kobject_hash.gperf"
    {"\330\035\000 ", {}, K_OBJ_SEM, 0, { .unused = 0 }},
    {""}, {""},
#line 43 "kobject_hash.gperf"
    {"\000`\000 ", {}, K_OBJ_THREAD_STACK_ELEMENT, 0, { .stack_data = &stack_data[0] }},
#line 47 "kobject_hash.gperf"
    {"\360r\000 ", {}, K_OBJ_SEM, 0, { .unused = 0 }},
#line 26 "kobject_hash.gperf"
    {"p\002\000 ", {}, K_OBJ_QUEUE, 0, { .unused = 0 }},
    {""}, {""},
#line 40 "kobject_hash.gperf"
    {"\220 \000 ", {}, K_OBJ_SEM, 0, { .unused = 0 }},
#line 46 "kobject_hash.gperf"
    {"\320r\000 ", {}, K_OBJ_SEM, 0, { .unused = 0 }},
#line 25 "kobject_hash.gperf"
    {"\230\001\000 ", {}, K_OBJ_THREAD, 0, { .thread_id = 2 }},
    {""}, {""},
#line 72 "kobject_hash.gperf"
    {"\220{\000 ", {}, K_OBJ_QUEUE, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 59 "kobject_hash.gperf"
    {"\024z\000 ", {}, K_OBJ_MEM_SLAB, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
    {""}, {""}, {""},
#line 22 "kobject_hash.gperf"
    {" \000\000 ", {}, K_OBJ_TIMER, 0, { .unused = 0 }},
#line 31 "kobject_hash.gperf"
    {" \006\000 ", {}, K_OBJ_THREAD, 0, { .thread_id = 3 }},
    {""}, {""}, {""},
#line 58 "kobject_hash.gperf"
    {"\370y\000 ", {}, K_OBJ_MEM_SLAB, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 38 "kobject_hash.gperf"
    {"\304\024\000 ", {}, K_OBJ_SEM, 0, { .unused = 0 }},
    {""}, {""}, {""},
#line 24 "kobject_hash.gperf"
    {"\370\000\000 ", {}, K_OBJ_THREAD, 0, { .thread_id = 1 }},
#line 23 "kobject_hash.gperf"
    {"X\000\000 ", {}, K_OBJ_THREAD, 0, { .thread_id = 0 }},
    {""}, {""}, {""},
#line 52 "kobject_hash.gperf"
    {"\030y\000 ", {}, K_OBJ_DRIVER_UART, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 27 "kobject_hash.gperf"
    {"X\003\000 ", {}, K_OBJ_QUEUE, 0, { .unused = 0 }},
    {""}, {""}, {""},
#line 35 "kobject_hash.gperf"
    {"\300\012\000 ", {}, K_OBJ_THREAD, 0, { .thread_id = 7 }},
#line 28 "kobject_hash.gperf"
    {"\234\003\000 ", {}, K_OBJ_QUEUE, 0, { .unused = 0 }},
    {""}, {""}, {""},
#line 73 "kobject_hash.gperf"
    {"\270{\000 ", {}, K_OBJ_QUEUE, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 36 "kobject_hash.gperf"
    {"\234\013\000 ", {}, K_OBJ_MEM_SLAB, 0, { .unused = 0 }},
    {""}, {""}, {""},
#line 51 "kobject_hash.gperf"
    {"\270w\000 ", {}, K_OBJ_SEM, 0, { .unused = 0 }},
#line 37 "kobject_hash.gperf"
    {"\274\013\000 ", {}, K_OBJ_MEM_SLAB, 0, { .unused = 0 }},
    {""}, {""}, {""},
#line 50 "kobject_hash.gperf"
    {"\240w\000 ", {}, K_OBJ_SEM, 0, { .unused = 0 }},
#line 71 "kobject_hash.gperf"
    {"h{\000 ", {}, K_OBJ_QUEUE, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
    {""}, {""}, {""},
#line 29 "kobject_hash.gperf"
    {"\240\004\000 ", {}, K_OBJ_QUEUE, 0, { .unused = 0 }},
#line 70 "kobject_hash.gperf"
    {"@{\000 ", {}, K_OBJ_QUEUE, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
    {""}, {""}, {""},
#line 30 "kobject_hash.gperf"
    {"\344\004\000 ", {}, K_OBJ_QUEUE, 0, { .unused = 0 }},
    {""}, {""}, {""}, {""},
#line 34 "kobject_hash.gperf"
    {" \012\000 ", {}, K_OBJ_THREAD, 0, { .thread_id = 5 }},
    {""}, {""}, {""}, {""},
#line 69 "kobject_hash.gperf"
    {"\030{\000 ", {}, K_OBJ_QUEUE, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
    {""}, {""}, {""}, {""},
#line 49 "kobject_hash.gperf"
    {"\030t\000 ", {}, K_OBJ_POLL_SIGNAL, 0, { .unused = 0 }},
    {""}, {""}, {""}, {""},
#line 48 "kobject_hash.gperf"
    {"\014s\000 ", {}, K_OBJ_QUEUE, 0, { .unused = 0 }},
    {""}, {""}, {""}, {""},
#line 33 "kobject_hash.gperf"
    {"\200\011\000 ", {}, K_OBJ_THREAD, 0, { .thread_id = 6 }}
  };

struct z_object *
z_object_lookup (register const char *str, register size_t len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        if (len == lengthtable[key])
          {
            register const char *s = wordlist[key].name;

            if (*str == *s && !memcmp (str + 1, s + 1, len - 1))
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
