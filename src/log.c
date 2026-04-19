#include <stdio.h>
#include <stdarg.h>
#include "log.h"
#include "libopencm3/stm32/rcc.h"
#include "libopencm3/stm32/gpio.h"
#include "libopencm3/stm32/usart.h"
#include "libopencm3/cm3/nvic.h"
#include "FreeRTOS.h"
#include "queue.h"

#define CON(A, B) A##B

#define LOG_USART 1
#define USART_GPIO B
#define USART_RX_PIN 7
#define USART_TX_PIN 6

#define _GPIO_PERIPH(A) CON(GPIO, A)
#define _GPIO_RCC(A) CON(RCC_GPIO, A)
#define _GPIO_PIN(A) CON(GPIO, A)
#define _USART_PERIPH(A) CON(USART, A)
#define _USART_RCC(A) CON(RCC_USART, A)
#define BUFFER_SIZE (80)

static enum rcc_periph_clken const GPIO_RCC = _GPIO_RCC(USART_GPIO);
static uint32_t const GPIO = _GPIO_PERIPH(USART_GPIO);
static uint16_t const GPIO_RX = _GPIO_PIN(USART_RX_PIN);
static uint16_t const GPIO_TX = _GPIO_PIN(USART_TX_PIN);
static uint32_t const USART = _USART_PERIPH(LOG_USART);
static enum rcc_periph_clken const USART_RCC = _USART_RCC(LOG_USART);
#ifdef COLOR_LOG
typedef char * ColorCode_t;
static uint32_t const COLOR_TABLE[4] = {96, 37, 93, 91};
#endif /* COLOR_LOG */
static LogLevel_t level = LL_VERBOSE;
static QueueHandle_t outbound;

static void _putch(char c);

void logInit()
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
  nvic_clear_pending_irq(NVIC_USART1_IRQ);
  nvic_enable_irq(NVIC_USART1_IRQ);
  usart_enable(USART);

  /* other stuff */
  outbound = xQueueCreate(1024, sizeof(char));
  _puts("hello!\r\n");
}

void logLevel(LogLevel_t l)
{
  level = l;
}

static void _putch(char c)
{
  if(xQueueSend(outbound, &c, 0) == pdTRUE) usart_enable_tx_interrupt(USART);
}

void _puts(char const * const s)
{
  char const * _s = s;
  while(*_s) _putch(*_s++);
}

void _log(LogLevel_t const verbosity, char const * const tag, char const * const fmt, ...)
{
  static char const * const format = 
#ifdef COLOR_LOG
  "\r\x1B[%lum[%010lu][%s][%c]";
#else
  "\r[%010lu][%s][%c]";
#endif
  /* \r\x1B[K\x1B[<DC>m[__time__]\x1B[<color>m[verbosity]<text>\x1B[<DC>m\r\n<prompt><user_input>*/
  char buf[BUFFER_SIZE];
  if(verbosity >= level)
  {
    uint32_t const timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    snprintf(buf, BUFFER_SIZE, format,
#ifdef COLOR_LOG
      COLOR_TABLE[verbosity],
#endif
      timestamp, 
      tag,
      (verbosity == LL_VERBOSE) ? 'V' : ((verbosity == LL_INFO) ? 'I' : ((verbosity == LL_WARNING) ? 'W' : 'E'))
    );
    _puts(buf);
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, BUFFER_SIZE, fmt, args);
    va_end(args);
    _puts(buf);
    _puts("\r\n");
  }
}

void usart1_isr()
{
  if(usart_get_flag(USART, USART_FLAG_TXE)) {
    char c;
    portBASE_TYPE foo;
    if(xQueueReceiveFromISR(outbound, &c, &foo)) {
      usart_send(USART, c);
    } else {
      usart_disable_tx_interrupt(USART);
    }
  }
  if(usart_get_flag(USART, USART_FLAG_RXNE)) {
    // dumb read
    (void)usart_recv(USART);
  }
}