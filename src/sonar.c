#include <string.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/f1/nvic.h>
#include "sonar.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "log.h"
#include "buffer.h"
#include "macros.h"

#define TAG "SONA"
#define TIMER_DEF   1
#define GPIO_IN_DEF A
#define PIN_IN_DEF  8
#define GPIO_ON_DEF A
#define PIN_ON_DEF  8

static enum rcc_periph_clken const TIMER_RCC = _TIMER_RCC(TIMER_DEF);
static uint32_t const TIMER = _TIMER_PERIPH(TIMER_DEF);
static uint32_t const GPIO_IN = _GPIO_PERIPH(GPIO_IN_DEF);
static uint32_t const PIN_IN = _GPIO_PIN(PIN_IN_DEF);
static uint32_t const GPIO_ON = _GPIO_PERIPH(GPIO_ON_DEF);
static uint32_t const PIN_ON = _GPIO_PIN(PIN_ON_DEF);

static int const VERSION_MAJOR = 0;
static int const VERSION_MINOR = 1;

static uint32_t const STARTUP_PKT_COUNT = 3;
static QueueHandle_t bits;
static QueueHandle_t words;
static uint64_t data = 0;
static uint32_t bitCnt = 0;
static uint32_t pktCnt = 0;

static uint32_t const BITS_PER_FRAME = 33;
static uint32_t const START_MARKER   = 1500;
static uint32_t const ONE            = 175;

static void onReceive(uint64_t value);
static void initHW(void);
static void reset(void);

void sonarPower(uint32_t p)
{
  uint32_t s = (GPIOB_ODR & GPIO7) ? 1 : 0;
  if(s != p)
  {
    pktCnt = 0;
    if(p)
      gpio_set(GPIOB, GPIO7);
    else
      gpio_clear(GPIOB, GPIO7);
  }
}

void sonarTask(void * p)
{
  static char const * const date = __DATE__;
  logI(TAG, "sonar module v%i.%i built@%s", VERSION_MAJOR, VERSION_MINOR, date);
  words = p;
  bits = xQueueCreate(64, sizeof(uint16_t));
  if(bits == NULL)
  {
    logE(TAG, "Unable to create queue!");
    vTaskDelete(NULL);
  }

  initHW();
  while(1)
  {
    uint16_t time;
    uint32_t bit = 0;

    if(pdTRUE == xQueueReceive(bits, &time, portMAX_DELAY))
    {
      if(time > START_MARKER)
      {
        reset();
      } else {
        data <<= 1;
        bit = (time > ONE) ? 1 : 0;
        data |= bit;
        if(++bitCnt >= BITS_PER_FRAME)
        {
          onReceive(data);
          reset();
        }
      }
    }
  }
}

static void onReceive(uint64_t value)
{
  SonarData_t data;
  uint32_t start = (value & 0x100000000ULL) ? 1 : 0;
  if(start != 0)
  {
    logW(TAG, "Something new here! Start bit is non-zero!");
  }
  data.ch1 = (uint8_t)((value >> 24) & 0xFF);
  data.ch2 = (uint8_t)((value >> 16) & 0xFF);
  data.ch3 = (uint8_t)((value >>  8) & 0xFF);
  data.ch4 = (uint8_t)((value >>  0) & 0xFF);
  logV(TAG, "frame: %02X %02X %02X %02X", data.ch1, data.ch2, data.ch3, data.ch4);

  if(words == NULL) {
    logE(TAG, "No outbound queue!");
    return;
  }

  Buffer_t * b;
  if(pktCnt < STARTUP_PKT_COUNT) {
    b = bufferAllocate(TYPE_SONAR_DATA, sizeof(data), &data);
  } else {
    SonarStatus_t status;
    b = bufferAllocate(TYPE_SONAR_STATUS, sizeof(status), &status);
  }
  
  if(b == NULL) {
    logE(TAG, "Can't allocate buffer!");
    return;
  }
  if(xQueueSend(words, &b, 0) != pdTRUE)
  {
    logE(TAG, "Failed to send a packet!");
    bufferDispose(b);
  }
}

static void reset()
{
  bitCnt = 0;
  data = 0;
}

static void initHW()
{
  /* GPIO pin: */
  rcc_periph_clock_enable(RCC_GPIOA);
  gpio_set_mode(GPIO_IN, GPIO_MODE_INPUT,        GPIO_CNF_INPUT_PULL_UPDOWN, PIN_IN);
  gpio_set_mode(GPIO_ON, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,   PIN_ON);
  gpio_clear(GPIO_ON, PIN_ON);

  /* Time base: */
  rcc_periph_clock_enable(TIMER_RCC);
  timer_disable_counter(TIMER);
  uint32_t const clk = rcc_get_timer_clk_freq(TIMER);
  timer_set_prescaler(TIMER, clk / 1000000);
  timer_set_period(TIMER, 0xFFFF);
  
  /* Reset timer on the rising edge of every pulse: */
  timer_ic_set_input(TIMER, TIM_IC1, TIM_IC_IN_TI1);
  timer_ic_set_polarity(TIMER, TIM_IC1, TIM_IC_RISING);
  timer_slave_set_trigger(TIMER, TIM_SMCR_TS_TI1FP1);
  timer_slave_set_mode(TIMER, TIM_SMCR_SMS_RM);
  timer_ic_enable(TIMER, TIM_IC1);

  /* Capture time on the falling edge of every pulse: */
  timer_ic_set_input(TIMER, TIM_IC2, TIM_IC_IN_TI1);
  timer_ic_set_polarity(TIMER, TIM_IC2, TIM_IC_FALLING);
  timer_ic_enable(TIMER, TIM_IC2);
  timer_clear_flag(TIMER, TIM_SR_CC2IF);
  timer_enable_irq(TIMER, TIM_DIER_CC2IE);

  /* Interrupt: */
  nvic_clear_pending_irq(NVIC_TIM1_CC_IRQ);
  nvic_set_priority(NVIC_TIM1_CC_IRQ, 239);
  nvic_enable_irq(NVIC_TIM1_CC_IRQ);

  /* GO! */
  timer_enable_counter(TIMER);
}

void tim1_cc_isr(void)
{
  if(timer_get_flag(TIM1, TIM_SR_CC2IF))
  {
    timer_clear_flag(TIM1, TIM_SR_CC2IF);
    uint16_t time = TIM1_CCR2;
    portBASE_TYPE foo = pdFALSE;
    xQueueSendFromISR(bits, &time, &foo);
    portYIELD_FROM_ISR(foo);
  }
}