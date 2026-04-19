/**
 * 
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/gpio.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "sonar.h"
#include "link-head.h"
#include "log.h"

static TaskHandle_t sonar;
static TaskHandle_t link;
static QueueHandle_t dataFlow;

static void systemInit(void);
int main(void);

int main()
{
  systemInit();
  logInit();
  dataFlow = xQueueCreate(16, sizeof(SonarData_t));
  xTaskCreate(headTask,  "head",    1024, (void *)dataFlow, tskIDLE_PRIORITY, &link);
  xTaskCreate(sonarTask, "example", 1024, (void *)dataFlow, tskIDLE_PRIORITY, &sonar);
  vTaskStartScheduler();
  while(1);
}

static void systemInit()
{
  /* 64 MHz from the HSI is the max possible speed */
  rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_HSI_64MHZ]);
  
  SCB_VTOR = FLASH_BASE;
  scb_set_priority_grouping(SCB_AIRCR_PRIGROUP_GROUP4_SUB4);

  rcc_periph_clock_enable(RCC_AFIO);
  gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, AFIO_MAPR_USART1_REMAP);
}