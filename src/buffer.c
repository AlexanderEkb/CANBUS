#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "log.h"

#define TAG "BUFF"

Buffer_t * bufferAllocate(Type_t type, size_t length, void * data)
{
  Buffer_t * b = malloc(sizeof(Buffer_t));
  if(b == NULL) {
    logE(TAG, "Can't allocate buffer!");
    return NULL;
  }
  b->data = malloc(length);
  if(b->data == NULL)
  {
    logE(TAG, "Can't allocate buffer data!");
    free(b);
    return NULL;
  }
  b->type = type;
  b->length = length;
  memcpy(b->data, data, length);
  return b;
}

uint32_t bufferDispose(Buffer_t * b)
{
  free(b->data);
  free(b);
}
