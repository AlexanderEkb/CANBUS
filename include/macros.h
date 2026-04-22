#ifndef __MACROS_H__
#define __MACROS_H__

#define CON(A, B) A##B

#define _GPIO_PERIPH(A) CON(GPIO, A)
#define _GPIO_RCC(A) CON(RCC_GPIO, A)
#define _GPIO_PIN(A) CON(GPIO, A)
#define _USART_PERIPH(A) CON(USART, A)
#define _USART_RCC(A) CON(RCC_USART, A)
#define _TIMER_PERIPH(A) CON(TIM, A)
#define _TIMER_RCC(A) CON(RCC_TIM, A)

#endif /* __MACROS_H__ */