#ifndef FDK_TYPES_H
#define FDK_TYPES_H

#include <stdint.h>
#include <stddef.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef size_t   usize;

typedef enum {
    FDK_OK     =  0,
    FDK_ERR    = -1,
    FDK_EINVAL = -2,
    FDK_ENOMEM = -3,
    FDK_ENOENT = -4,
    FDK_EIO    = -5,
    FDK_EBUSY  = -6,
    FDK_ENOTSUP= -7,
} fdk_status;

#define FDK_PACKED       __attribute__((packed))
#define FDK_ALIGN(n)     __attribute__((aligned(n)))
#define FDK_UNUSED(x)    ((void)(x))

#endif /* FDK_TYPES_H */
