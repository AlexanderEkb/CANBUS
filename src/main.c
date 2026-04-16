/**
 * 
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/scb.h>
#include "FreeRTOS.h"
#include "task.h"
#include "sonar.h"

static TaskHandle_t sonar;

static void systemInit(void);
int main(void);

int main()
{
  systemInit();
  xTaskCreate(sonarTask, "example", 1024, NULL, tskIDLE_PRIORITY, &sonar);
  vTaskStartScheduler();
  while(1);
}

static void systemInit()
{
  /* 64 MHz from the HSI is the max possible speed */
  rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_HSI_64MHZ]);
  
  // NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x0 );
  SCB_VTOR = FLASH_BASE;
  scb_set_priority_grouping(SCB_AIRCR_PRIGROUP_GROUP4_SUB4);
}