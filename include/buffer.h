#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdint.h>
#include <stddef.h>

typedef enum {
  TYPE_SONAR_DATA = 0,
  TYPE_SONAR_STATUS,
  /* ... */
  TYPE_COUNT
} Type_t;

typedef struct {
  Type_t type;
  uint32_t length;
  void * data;
} Buffer_t;

Buffer_t * bufferAllocate(Type_t type, size_t length, void * data);
uint32_t bufferDispose(Buffer_t * b);

#endif /* __BUFFER_H__ */