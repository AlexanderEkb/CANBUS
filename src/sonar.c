#include <string.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/f1/nvic.h>
#include "sonar.h"
#include "FreeRTOS.h"
#include "queue.h"

static QueueHandle_t bits;
static QueueHandle_t words;
static uint64_t result = 0;
static uint32_t counter = 0;

static uint32_t const BITS_PER_FRAME = 33;
static uint32_t const START_MARKER   = 1500;
static uint32_t const ONE            = 175;

static void onReceive(uint64_t value);
static void initHW(void);
static void reset(void);

void sonarTask(void * p)
{
  words = p;
  bits = xQueueCreate(64, sizeof(uint16_t));

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
        result <<= 1;
        bit = (time > ONE) ? 1 : 0;
        result |= bit;
        if(++counter >= BITS_PER_FRAME)
        {
          onReceive(result);
          reset();
        }
      }
    }
  }
}

static void onReceive(uint64_t value)
{
  SonarData_t data;
  data.start = (value & 0x100000000ULL) ? 1 : 0;
  data.ch1 = (uint8_t)((value >> 24) & 0xFF);
  data.ch2 = (uint8_t)((value >> 16) & 0xFF);
  data.ch3 = (uint8_t)((value >>  8) & 0xFF);
  data.ch4 = (uint8_t)((value >>  0) & 0xFF);
  if(words != NULL) xQueueSend(words, &data, 0);
}

static void reset()
{
  counter = 0;
  result = 0;
}

static void initHW()
{
  /* GPIO pin: */
  rcc_periph_clock_enable(RCC_GPIOA);
  gpio_set_mode(GPIO_BANK_TIM1_CH1, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO_TIM1_CH1);

  /* Time base: */
  rcc_periph_clock_enable(RCC_TIM1);
  timer_disable_counter(TIM1);
  uint32_t const clk = rcc_get_timer_clk_freq(TIM1);
  timer_set_prescaler(TIM1, clk / 1000000);
  timer_set_period(TIM1, 0xFFFF);
  
  /* Reset timer on the rising edge of every pulse: */
  timer_ic_set_input(TIM1, TIM_IC1, TIM_IC_IN_TI1);
  timer_ic_set_polarity(TIM1, TIM_IC1, TIM_IC_RISING);
  timer_slave_set_trigger(TIM1, TIM_SMCR_TS_TI1FP1);
  timer_slave_set_mode(TIM1, TIM_SMCR_SMS_RM);
  timer_ic_enable(TIM1, TIM_IC1);

  /* Capture time on the falling edge of every pulse: */
  timer_ic_set_input(TIM1, TIM_IC2, TIM_IC_IN_TI1);
  timer_ic_set_polarity(TIM1, TIM_IC2, TIM_IC_FALLING);
  timer_ic_enable(TIM1, TIM_IC2);
  timer_clear_flag(TIM1, TIM_SR_CC2IF);
  timer_enable_irq(TIM1, TIM_DIER_CC2IE);

  /* Interrupt: */
  nvic_clear_pending_irq(NVIC_TIM1_CC_IRQ);
  nvic_set_priority(NVIC_TIM1_CC_IRQ, 128);
  nvic_enable_irq(NVIC_TIM1_CC_IRQ);

  /* GO! */
  timer_enable_counter(TIM1);
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