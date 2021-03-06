// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_GPIO_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_GPIO_CONFIG_DESCRIPTION_H_

enum Description {
    // ===PORTA===
    NTC_BAT,
    NTC_MOT,
    CONFIG,
    DMS_OFFSET,
    PHSTA,
    MEMS_CLK,
    IR_TXD,
    IR_RXD,
    SWDIO,
    SWCLK,
    // ===PORTB===
    MEMS_INT,
    TEST_PIN_OUT,
    DEBUG_TXD,
    DEBUG_RXD,
    COMP_3_INP, //IR_RXDANA COMP3_INP
    // ===PORTC===
    NTC_FETS,
    BAT_VOL,
    BAT_CUR_ADC,
    DMS,
    COMP_3_OUT,
    MEMS_SDA,
    LED_FRONT_PIN,
    // ===PORTD===
    HALL1,
    HALL2,
    HALL3,
    COMP_3_INM,
    // ===PORTE===
    A_HS,
    B_HS,
    A_LS,
    B_LS,
    C_LS,
    PHSTB,
    PHSTSORC,
    // ===PORTF===
    C_HS,
    // ===PORTG===
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_GPIO_CONFIG_CONTAINER_H_
#define SOURCES_PMD_GPIO_CONFIG_CONTAINER_H_

static constexpr const std::array<const Gpio, Gpio::__ENUM__SIZE + 1> Container =
{ {
      // ===================PORTA=================
      Gpio(Gpio::NTC_BAT,
           GPIOA_BASE,
           GPIO_InitTypeDef { GPIO_Pin_0, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::NTC_MOT,
           GPIOA_BASE,
           GPIO_InitTypeDef { GPIO_Pin_1, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::CONFIG,
           GPIOA_BASE,
           GPIO_InitTypeDef { GPIO_Pin_2, GPIO_Mode_IN, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN }),
      Gpio(Gpio::DMS_OFFSET, // TODO CHECK //DAC1_OUT2
           GPIOA_BASE,
           GPIO_InitTypeDef { GPIO_Pin_5, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::PHSTA,
           GPIOA_BASE,
           GPIO_InitTypeDef { GPIO_Pin_6, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::MEMS_CLK, // I2C3
           GPIOA_BASE,
           GPIO_InitTypeDef { GPIO_Pin_8, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource8,
           GPIO_AF_3),
      Gpio(Gpio::IR_TXD, //UART1
           GPIOA_BASE,
           GPIO_InitTypeDef { GPIO_Pin_9, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource9,
           GPIO_AF_7),
      Gpio(Gpio::IR_RXD,
           GPIOA_BASE,
           GPIO_InitTypeDef { GPIO_Pin_10, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL },
           GPIO_PinSource10,
           GPIO_AF_7),
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
      Gpio(Gpio::MEMS_INT,
           GPIOB_BASE,
           GPIO_InitTypeDef { GPIO_Pin_1, GPIO_Mode_IN, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource1),
      Gpio(Gpio::TEST_PIN_OUT,
           GPIOB_BASE,
           GPIO_InitTypeDef { GPIO_Pin_8, GPIO_Mode_OUT, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_UP }),
      Gpio(Gpio::DEBUG_TXD, //UART3
           GPIOB_BASE,
           GPIO_InitTypeDef { GPIO_Pin_10, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL },
           GPIO_PinSource10,
           GPIO_AF_7),
      Gpio(Gpio::DEBUG_RXD, // UART3
           GPIOB_BASE,
           GPIO_InitTypeDef { GPIO_Pin_11, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL },
           GPIO_PinSource11,
           GPIO_AF_7),
      Gpio(Gpio::COMP_3_INP,
           GPIOB_BASE,
           GPIO_InitTypeDef { GPIO_Pin_14, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      // ===================PORTC=================
      Gpio(Gpio::NTC_FETS,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_0, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::BAT_VOL,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_1, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::BAT_CUR_ADC,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_2, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::DMS,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_3, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::COMP_3_OUT,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_8, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource8,
           GPIO_AF_7),
      Gpio(Gpio::MEMS_SDA, //I2C3
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_9, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource9,
           GPIO_AF_3),
      Gpio(Gpio::LED_FRONT_PIN, // SPI 3
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_12, GPIO_Mode_AF, GPIO_Speed_50MHz, GPIO_OType_OD, GPIO_PuPd_DOWN },
           GPIO_PinSource12,
           GPIO_AF_6),
      // ===================PORTD=================
      Gpio(Gpio::HALL1, // TIM4
           GPIOD_BASE,
           GPIO_InitTypeDef { GPIO_Pin_12, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_UP},
           GPIO_PinSource12,
           GPIO_AF_2),
      Gpio(Gpio::HALL2,
           GPIOD_BASE,
           GPIO_InitTypeDef { GPIO_Pin_13, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_UP },
           GPIO_PinSource13,
           GPIO_AF_2),
      Gpio(Gpio::HALL3,
           GPIOD_BASE,
           GPIO_InitTypeDef { GPIO_Pin_14, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_UP },
           GPIO_PinSource14,
           GPIO_AF_2),
      Gpio(Gpio::COMP_3_INM,
           GPIOD_BASE,
           GPIO_InitTypeDef { GPIO_Pin_15, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      // ===================PORTE=================
      Gpio(Gpio::A_HS, //TIM20
           GPIOE_BASE,
           GPIO_InitTypeDef {GPIO_Pin_2, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},
           GPIO_PinSource2,
           GPIO_AF_6),
      Gpio(Gpio::B_HS,
           GPIOE_BASE,
           GPIO_InitTypeDef {GPIO_Pin_3, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},
           GPIO_PinSource3,
           GPIO_AF_6),
      Gpio(Gpio::A_LS,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_4, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource4,
           GPIO_AF_6),
      Gpio(Gpio::B_LS,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_5, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource5,
           GPIO_AF_6),
      Gpio(Gpio::C_LS,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_6, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN },
           GPIO_PinSource6,
           GPIO_AF_6),
      Gpio(Gpio::PHSTB,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_9, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::PHSTSORC,
           GPIOE_BASE,
           GPIO_InitTypeDef { GPIO_Pin_14, GPIO_Mode_AN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL}),
      // ===================PORTF=================
      Gpio(Gpio::C_HS,
           GPIOF_BASE,
           GPIO_InitTypeDef {GPIO_Pin_2, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},
           GPIO_PinSource2,
           GPIO_AF_2),
      // ===================PORTG=================
      Gpio(Gpio::__ENUM__SIZE,
           GPIOG_BASE,
           GPIO_InitTypeDef { GPIO_Pin_All, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_UP })
  }};

#endif /* SOURCES_PMD_GPIO_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_GPIO_CONFIG_DESCRIPTION_H_ */
