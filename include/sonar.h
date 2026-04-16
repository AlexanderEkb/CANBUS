/**
 * 
 */

#include <stdint.h>

typedef struct {
  uint8_t start;
  uint8_t ch1;  // A
  uint8_t ch2;  // D
  uint8_t ch3;  // C
  uint8_t ch4;  // B
} SonarData_t;

/**
 * - - - - : 0 C4 C3 C2 C1 - 83 43 C3 23
 * A - - - : 0 C5 C4 C3 C2 - 43 C3 23 A3
 * - - - D : 0 C4 C4 C3 C2
 * - - C - : 0 C4 C3 C3 C2
 * - B - - : 0 C4 C3 C2 C2
 */

void sonarTask(void * p);