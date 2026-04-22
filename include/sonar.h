/**
 * 
 */
#ifndef __SONAR_H__
#define __SONAR_H__
#include <stdint.h>

typedef struct {
  uint8_t ch1;  // A
  uint8_t ch2;  // D
  uint8_t ch3;  // C
  uint8_t ch4;  // B
} SonarData_t;

typedef uint32_t SonarStatus_t;

/**
 *              A  D  C  B                          A        D        C        B       
 * - - - - : 0 C4 C3 C2 C1 - 83 43 C3 23 - 0 11000100 11000011 11000010 11000001
 * A - - - : 0 C5 C4 C3 C2 - 43 C3 23 A3 - 0 11000101 11000100 11000011 11000010 - C1
 * - - - D : 0 C4 C4 C3 C2 - 43 C3 23 23 - 0 11000100 11000100 11000011 11000010 - C1
 * - - C - : 0 C4 C3 C3 C2 - 43 C3 C3 23 - 0 11000100 11000011 11000011 11000010 - C1
 * - B - - : 0 C4 C3 C2 C2 - 43 43 C3 23 - 0 11000100 11000011 11000010 11000010
 * 
 * 0хС4 - это ошибка Е1
 * 0хС3 - Е2
 * 0хС2 - Е3
 * 0хС1 - Е4
 */

void sonarTask(void * p);
void sonarPower(uint32_t p);

#endif /* __SONAR_H__ */