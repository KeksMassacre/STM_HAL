// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_GPIO_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_GPIO_CONFIG_DESCRIPTION_H_

enum Description {
    // ===PORTA===
    USER_BUTTON,
    BLDC_A,
    BLDC_B,
    BLDC_C,
    LED_BACK_SPI2_MOSI,
    SWDIO,
    SWCLK,
    // ===PORTB===
    NWP_INTERRUPT,
    MPU_INTERRUPT,
    DEBUG_UART_TX,
    DEBUG_UART_RX,
    I2C_1_SCL,
    I2C_1_SDA,
    VESC_UART_TX,
    VESC_UART_RX,
    BLDC_AN,
    BLDC_BN,
    BLDC_CN,
    // ===PORTC===
    BATTERY_U,
    BATTERY_I,
    NTC_BATTERY,
    DMS,
    NTC_MOTOR,
    NTC_FET,
    HALL1,
    HALL2,
    HALL3,
    N_IR_STANDBY,
    ENABLE_5V8_SUPPLY,
    LED_FRONT_SPI3_MOSI,
    // ===PORTD===
    ENABLE_5V0_SUPPLY,
    MSCOM_UART_TX,
    MSCOM_UART_RX,
    IB_FB_ADC,
    IA_FB_ADC,
    HELLOWORLD,
    I_TOTAL_FB_ADC,
    // ===PORTE===
    CONFIG,
    HALL1_SIM,
    HALL2_SIM,
    HALL3_SIM,
    LED_4,
    LED_3,
    LED_5,
    LED_7,
    LED_9,
    LED_10,
    LED_8,
    LED_6,
    // ===PORTF===
    BUZZER,
    PWM1,
    // ===PORTG===
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_GPIO_CONFIG_CONTAINER_H_
#define SOURCES_PMD_GPIO_CONFIG_CONTAINER_H_

static constexpr const std::array<const Gpio, Gpio::__ENUM__SIZE + 1> Container =
{ {
      // ===================PORTA=================
      Gpio(Gpio::USER_BUTTON,
           GPIOA_BASE,
           GPIO_InitTypeDef { GPIO_Pin_0, GPIO_Mode_IN, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL },
           GPIO_PinSource0),

      Gpio(Gpio::BLDC_A,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_8, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},
           GPIO_PinSource8,
           GPIO_AF_6),
      Gpio(Gpio::BLDC_B,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_9, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},
           GPIO_PinSource9,
           GPIO_AF_6),
      Gpio(Gpio::BLDC_C,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_10, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},
           GPIO_PinSource10,
           GPIO_AF_6),
      Gpio(Gpio::LED_BACK_SPI2_MOSI,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_11, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_OD, GPIO_PuPd_DOWN},
           GPIO_PinSource11,
           GPIO_AF_5),
      Gpio(Gpio::SWDIO,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_13, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_UP},
           GPIO_PinSource13,
           GPIO_AF_0),
      Gpio(Gpio::SWCLK,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_14, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},
           GPIO_PinSource14,
           GPIO_AF_0),
      // ===================PORTB=================
      Gpio(Gpio:: NWP_INTERRUPT,
           GPIOB_BASE,
           GPIO_InitTypeDef { GPIO_Pin_0, GPIO_Mode_IN, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource0),
      Gpio(Gpio::MPU_INTERRUPT,
           GPIOB_BASE,
           GPIO_InitTypeDef { GPIO_Pin_1, GPIO_Mode_IN, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource1),
      Gpio(Gpio::DEBUG_UART_TX,
           GPIOB_BASE,
           GPIO_InitTypeDef { GPIO_Pin_6, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL },
           GPIO_PinSource6,
           GPIO_AF_7),
      Gpio(Gpio::DEBUG_UART_RX,
           GPIOB_BASE,
           GPIO_InitTypeDef { GPIO_Pin_7, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL },
           GPIO_PinSource7,
           GPIO_AF_7),
      Gpio(Gpio::I2C_1_SCL,
           GPIOB_BASE,
           GPIO_InitTypeDef { GPIO_Pin_8, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource8,
           GPIO_AF_4),
      Gpio(Gpio::I2C_1_SDA,
           GPIOB_BASE,
           GPIO_InitTypeDef { GPIO_Pin_9, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource9,
           GPIO_AF_4),
      Gpio(Gpio::VESC_UART_TX,
           GPIOB_BASE,
           GPIO_InitTypeDef { GPIO_Pin_10, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_UP },
           GPIO_PinSource10,
           GPIO_AF_7),
      Gpio(Gpio::VESC_UART_RX,
           GPIOB_BASE,
           GPIO_InitTypeDef { GPIO_Pin_11, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource11,
           GPIO_AF_7),
      Gpio(Gpio::BLDC_AN,
           GPIOB_BASE,
           GPIO_InitTypeDef { GPIO_Pin_13, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource13,
           GPIO_AF_6),
      Gpio(Gpio::BLDC_BN,
           GPIOB_BASE,
           GPIO_InitTypeDef { GPIO_Pin_14, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource14,
           GPIO_AF_6),
      Gpio(Gpio::BLDC_CN,
           GPIOB_BASE,
           GPIO_InitTypeDef { GPIO_Pin_15, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource15,
           GPIO_AF_4),
      // ===================PORTC=================
      Gpio(Gpio::BATTERY_U,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_0, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::BATTERY_I,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_1, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::NTC_BATTERY,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_2, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::DMS,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_3, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::NTC_MOTOR,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_4, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::NTC_FET,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_5, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::HALL1,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_6, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_UP},
           GPIO_PinSource6,
           GPIO_AF_2),
      Gpio(Gpio::HALL2,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_7, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_UP },
           GPIO_PinSource7,
           GPIO_AF_2),
      Gpio(Gpio::HALL3,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_8, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_UP },
           GPIO_PinSource8,
           GPIO_AF_2),
      Gpio(Gpio::N_IR_STANDBY,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_10, GPIO_Mode_OUT, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      Gpio(Gpio::ENABLE_5V8_SUPPLY,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_11, GPIO_Mode_OUT, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      Gpio(Gpio::LED_FRONT_SPI3_MOSI,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_12, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_OD, GPIO_PuPd_DOWN },
           GPIO_PinSource12,
           GPIO_AF_6),
      // ===================PORTD=================
      Gpio(Gpio::ENABLE_5V0_SUPPLY,
           GPIOD_BASE,
           GPIO_InitTypeDef { GPIO_Pin_1, GPIO_Mode_OUT, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_UP }),
      Gpio(Gpio::MSCOM_UART_TX,
           GPIOD_BASE,
           GPIO_InitTypeDef { GPIO_Pin_5, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_UP },
           GPIO_PinSource5,
           GPIO_AF_7),
      Gpio(Gpio::MSCOM_UART_RX,
           GPIOD_BASE,
           GPIO_InitTypeDef { GPIO_Pin_6, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource6,
           GPIO_AF_7),
      Gpio(Gpio::IB_FB_ADC,
           GPIOD_BASE,
           GPIO_InitTypeDef { GPIO_Pin_9, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::IA_FB_ADC,
           GPIOD_BASE,
           GPIO_InitTypeDef { GPIO_Pin_10, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::HELLOWORLD,
           GPIOD_BASE,
           GPIO_InitTypeDef { GPIO_Pin_12, GPIO_Mode_IN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      Gpio(Gpio::I_TOTAL_FB_ADC,
           GPIOD_BASE,
           GPIO_InitTypeDef { GPIO_Pin_14, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      // ===================PORTE=================
      Gpio(Gpio::CONFIG,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_0, GPIO_Mode_IN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      Gpio(Gpio::HALL1_SIM,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_2, GPIO_Mode_OUT, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      Gpio(Gpio::HALL2_SIM,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_3, GPIO_Mode_OUT, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      Gpio(Gpio::HALL3_SIM,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_4, GPIO_Mode_OUT, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      Gpio(Gpio::LED_4,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_8, GPIO_Mode_OUT, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      Gpio(Gpio::LED_3,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_9, GPIO_Mode_OUT, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      Gpio(Gpio::LED_5,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_10, GPIO_Mode_OUT, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      Gpio(Gpio::LED_7,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_11, GPIO_Mode_OUT, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      Gpio(Gpio::LED_9,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_12, GPIO_Mode_OUT, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      Gpio(Gpio::LED_10,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_13, GPIO_Mode_OUT, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      Gpio(Gpio::LED_8,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_14, GPIO_Mode_OUT, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL },
           GPIO_PinSource14, //TODO Configure as Output. Currently used for Trigger of ADC of PhaseCurrentMeasurement
           GPIO_AF_2),
      Gpio(Gpio::LED_6,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_15, GPIO_Mode_OUT, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      // ===================PORTF=================
      Gpio(Gpio::BUZZER,
           GPIOF_BASE,
           GPIO_InitTypeDef { GPIO_Pin_9, GPIO_Mode_OUT, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      Gpio(Gpio::PWM1,
           GPIOF_BASE,
           GPIO_InitTypeDef { GPIO_Pin_10, GPIO_Mode_OUT, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      // ===================PORTG=================
      Gpio(Gpio::__ENUM__SIZE,
           GPIOG_BASE,
           GPIO_InitTypeDef { GPIO_Pin_All, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_UP })
  } };

#endif /* SOURCES_PMD_GPIO_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_GPIO_CONFIG_DESCRIPTION_H_ */
