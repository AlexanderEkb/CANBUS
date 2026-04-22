I. Structure overview
=====================

                                   Parking
RPM ──────────────────╮ ╭───────── brake
                      │ │    
Speed ──────────────╮ │ │ ╭──────────── Ambient T° (ADC)
                    │ │ │ │ 
                    │ │ │ │ ╭────────── Engine T° (ADC)
AT selector         │ │ │ │ │ 
position (5) ─────╮ │ │ │ │ │           Door
                  │ │ │ │ │ │ ╭──────── sensors (4)
                  │ │ │ │ │ │ │
Sonar ──────────╮ │ │ │ │ │ │ │ ╭────── Power (IGN,
data in         │ │ │ │ │ │ │ │ │       ACC), GND
                ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓  
            ╭────────────────────────╮
            │                        │
            │                        │
            │                        │
            │                        │─→ SWD (4)
            │                        │─→ UART (console) (3)
            │                        │
            │                        │
            │                        │
            ╰────────────────────────╯
                │ │ │ │ │ │ │ │
Parking         │ │ ↓ ↓ │ │ │ ╰───→ CAN (2)
assist  ←───────╯ │     │ │ │
power (+12)       │     │ │ ╰─────→ Radio link (2)
                  │     │ │
Camera  ←─────────╯     │ ╰───────→ Reversing (+12V)
power (+12)             │           (to the radio)
                        │
                        ╰─────────→ DRL On (OD)

II. Pinouts
===========
X1: Debug
---     
1. SWDIO
2. SWCLK
3. +3.3V
4. GND
5. RX
6. TX 

X2: Radio, sonar and camera
---
| Pin | Function     | Wire color |│| Pin | Function     | Wire color |
|-----|--------------|------------|-|-----|--------------|------------|
| 1.  | Link RX      |            |│| 2.  | Link TX      |            |
| 3.  | GND          |            |│| 4.  | GND          |            |
| 5.  | Reverse      |            |│| 6.  | ACC          |            |
| 7.  | Sonar in     |            |│| 8.  | Sonar beep   |            |
| 9.  | Sonar PWR    |            |│| 10. | GND          |            |
| 11. | Camera PWR   |            |│| 12. |              |            |

X3: Inputs and CAN
---
| Pin | Function       | Wire color |│| Pin | Function     | Wire color |
|-----|----------------|------------|-|-----|--------------|------------|
| 1.  | "R"            |            |│| 2.  | "D"          |            |
| 3.  | "S"            |            |│| 4.  | "L"          |            |
| 5.  | !("P" or "N")  |            |│| 6.  | GND          |            |
| 7.  | RPM            |            |│| 8.  | Speed        |            |
| 9.  | Parking brake  |            |│| 10. | Seatbelts    |            |
| 11. | Lights         |            |│| 12. | -            |            |
| 13. | Engine T°      |            |│| 14. | GND          |            |
| 15. | Ambient T°     |            |│| 16. | GND          |            |
| 17. | Slide door     |            |│| 18. | Trunk door   |            |
| 19. | FL door        |            |│| 20. | FR door      |            |
| 21. | CANH           |            |│| 22. | CANL         |            |
| 23. |                |            |│| 24. |              |            |

III. MCU resources utilization
=====================================

GPIO
----

| Pin      |          | Pin      |          | Pin      |          |
|----------|----------|----------|----------|----------|----------|
| **PA0**  |          | **PB0**  |          |          |          |
| **PA1**  |          | **PB1**  |          |          |          |
| **PA2**  |          | **PB2**  | BOOT1    |          |          |
| **PA3**  |          | **PB3**  |          |          |          |
| **PA4**  |          | **PB4**  |          |          |          |
| **PA5**  |          | **PB5**  |          |          |          |
| **PA6**  |          | **PB6**  | Log TX   |          |          |
| **PA7**  | Sonar on | **PB7**  | Log RX   |          |          |
| **PA8**  | Sonar in | **PB8**  | CAN_RX   |          |          |
| **PA9**  |          | **PB9**  | CAN_TX   |          |          |
| **PA10** |          | **PB10** | Link_TX  |          |          |
| **PA11** |          | **PB11** | Link_RX  |          |          |
| **PA12** |          | **PB12** |          |          |          |
| **PA13** | SWDIO    | **PB13** |          | **PC13** |          |
| **PA14** | SWCLK    | **PB14** |          | **PC14** |          |
| **PA15** |          | **PB15** |          | **PC15** |          |

Peripherals
-----------
| Periph | Function     |
|--------|--------------|
| TIM1   | Sonar        |
| USART1 | Log/console  |

