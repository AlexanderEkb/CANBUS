#include "link-head.h"
#include "sonar.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "libopencm3/stm32/usart.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/f1/nvic.h>

static QueueHandle_t data;

static void initLink(void);
static void send(void * p);

void headTask(void * p)
{
  initLink();

  data = p;
  while(1)
  {
    SonarData_t d;
    xQueueReceive(data, &d, portMAX_DELAY);
    send(&d);
  }
}

static void initLink()
{
  /* GPIO: */
  /* USART: */
}

static void send(void * p)
{
  (void)p;
}
