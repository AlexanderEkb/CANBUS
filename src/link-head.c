#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/f1/nvic.h>
#include "link-head.h"
#include "log.h"
#include "sonar.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "buffer.h"
#include "macros.h"

#define TAG "LINK"

#define LINK_USART 1
#define LINK_GPIO B
#define LINK_RX_PIN 7
#define LINK_TX_PIN 6

#define BUFFER_SIZE (80)

static int const VERSION_MAJOR = 0;
static int const VERSION_MINOR = 1;

static enum rcc_periph_clken const GPIO_RCC = _GPIO_RCC(LINK_GPIO);
static uint32_t const GPIO = _GPIO_PERIPH(LINK_GPIO);
static uint16_t const GPIO_RX = _GPIO_PIN(LINK_RX_PIN);
static uint16_t const GPIO_TX = _GPIO_PIN(LINK_TX_PIN);
static uint32_t const USART = _USART_PERIPH(LINK_USART);
static enum rcc_periph_clken const USART_RCC = _USART_RCC(LINK_USART);

static QueueHandle_t data;
static QueueHandle_t outbound;

static void initLink(void);

void headTask(void * p)
{
  static char const * const date = __DATE__;
  logI(TAG, "link module v%i.%i built@%s", VERSION_MAJOR, VERSION_MINOR, date);
  outbound = xQueueCreate(1024, sizeof(char));
  if(outbound == NULL)
  {
    logE(TAG, "Unable to create queue!");
    vTaskDelete(NULL);
  }

  initLink();

  data = p;
  while(1)
  {
    Buffer_t * b;
    xQueueReceive(data, &b, portMAX_DELAY);
    switch(b->type)
    {
      case TYPE_SONAR_DATA:
        break;
      case TYPE_SONAR_STATUS:
        break;
      default:
        logW(TAG, "Unknown packet type: 0x%08X", b->type);
    }
    bufferDispose(b);
  }
}

static void initLink()
{
  /* GPIO */
  rcc_periph_clock_enable(GPIO_RCC);
  gpio_set_mode(GPIO, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO_RX);
  gpio_set_mode(GPIO, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_TX);

  /* USART */
  rcc_periph_clock_enable(USART_RCC);
  usart_disable(USART);
  usart_set_mode(USART, USART_MODE_TX_RX);
  usart_set_baudrate(USART, 115200);
  usart_set_databits(USART, 8);
  usart_set_parity(USART, USART_PARITY_NONE);
  usart_enable_rx_interrupt(USART);
  usart_disable_tx_interrupt(USART);
  nvic_clear_pending_irq(NVIC_USART3_IRQ);
  nvic_set_priority(NVIC_USART3_IRQ, 255);
  nvic_enable_irq(NVIC_USART3_IRQ);
  usart_enable(USART);
}
