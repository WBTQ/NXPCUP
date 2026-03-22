/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_debug_console.h"
#include "fsl_ctimer.h"
#include "fsl_utick.h"
#include "nxpcupTSA.h"
#include "freemaster_cfg.h"
#include "cmsis_gcc.h"
#include "board.h"
#include "app.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "control_loop.h"
#include "fsl_lpi2c_edma.h"
#include "pixy.h"
#include "Config.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SERVO_MIN_US 1000U
#define SERVO_MAX_US 2000U
#define SERVO_LIMITS_US(x)  ((x)<SERVO_MIN_US ? SERVO_MIN_US : ((x)>SERVO_MAX_US ? SERVO_MAX_US : (x)) ) 

#define MAX_VECTORS 10U
#define MAX_AUTO_DUTYCYCLE 40.0

typedef enum _app_mode {
    kappMode_remotecontrolled,
    kappMode_autonomous,
} app_mode_t;

typedef enum _app_status {
    kstatus_ok,
    kstatus_fault,
} app_status_t;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
app_status_t appStatus = kstatus_ok;
app_mode_t appMode = kappMode_remotecontrolled; 
float voltDCB = 12.0;    
bool_t AppSwitch = false;
float global_dutycycle;
float global_servo_angle;
uint16_t global_servo_period = 1500;

control_panel_t gControlPanelM1;
control_panel_t gControlPanelM2;

    uint16_t vectors[MAX_VECTORS * 4];
    size_t   num_vectors;
/* DMA0 TCD CH0_TRANSFER0 destination address */
AT_NONCACHEABLE_SECTION_ALIGN_INIT(uint32_t CTIMER0_Timings[4], 16);


int32_t period_channel_a;
int32_t period_channel_b;
/*******************************************************************************
 * Code
 ******************************************************************************/

void idle_mode(){
    AppSwitch = 0;
    setSpeedDutyCycle(0.0);
}

void clearStatusLED(){
    LED_BLUE_OFF();
    LED_GREEN_OFF();
    LED_RED_OFF();
}

void updateStatusLED(){
    if (appStatus == kstatus_ok && AppSwitch == 1){
        LED_GREEN_ON();
        LED_RED_OFF();
    }
    if (appStatus == kstatus_ok && AppSwitch == 0){
        LED_GREEN_OFF();
        LED_RED_ON();
    }

    if (appStatus == kstatus_fault){
        LED_GREEN_OFF();
        LED_RED_TOGGLE();
    }

    if (appMode == kappMode_autonomous){
        LED_BLUE_ON();
    }else{
        LED_BLUE_OFF();
    }
}

void run_safety_checks(){
    if(AppSwitch){

        if(voltDCB < 0 || voltDCB > 13.5){
                idle_mode();
        }
        if(gControlPanelM1.currentMeas < -5.0 || gControlPanelM1.currentMeas > 5.0){
            idle_mode();
        }
        if(gControlPanelM1.currentMeas < -5.0 || gControlPanelM1.currentMeas > 5.0){
            idle_mode();
        }
    }
}

 void SysTick_Handler(void)
{
    /* Toggle pin connected to LED */
    //GPIO_PortToggle(BOARD_LED_GPIO, 1u << BOARD_LED_GPIO_PIN);
}

/* LPTMR0_IRQn interrupt handler */
void LPTMR0_IRQHANDLER(void) {
    uint32_t intStatus;
    /* Reading all interrupt flags of status register */
    intStatus = LPTMR_GetStatusFlags(LPTMR0_PERIPHERAL);
    LPTMR_ClearStatusFlags(LPTMR0_PERIPHERAL, intStatus);

    //run_safety_checks();    

    updateStatusLED();
}


/* ADC0_IRQn interrupt handler */
void ADC0_IRQHANDLER(void) {
  uint32_t trigger_status_flag;
  uint32_t status_flag;
  /* Trigger interrupt flags */
  trigger_status_flag = LPADC_GetTriggerStatusFlags(ADC0_PERIPHERAL);
  /* Interrupt flags */
  status_flag = LPADC_GetStatusFlags(ADC0_PERIPHERAL);

  float V_currSens1 =  (float)(((uint16_t) (ADC0->RESFIFO[0] & 0xFFFF))/((float) (1<<16))*3.3);
  float V_currSens2 =  (float)(((uint16_t) (ADC0->RESFIFO[1] & 0xFFFF))/((float) (1<<16))*3.3);
  gControlPanelM1.currentMeas = (float)(36.7 * V_currSens1/3.3 -18.3); 
  gControlPanelM2.currentMeas = (float)(36.7 * V_currSens2/3.3 -18.3); 
  
  if(AppSwitch == 1){

  }else{
  }

  /* Clears trigger interrupt flags */
  LPADC_ClearTriggerStatusFlags(ADC0_PERIPHERAL, trigger_status_flag);
  /* Clears interrupt flags */
  LPADC_ClearStatusFlags(ADC0_PERIPHERAL, status_flag);

}

/* ADC1_IRQn interrupt handler */
void ADC1_IRQHANDLER(void) {
  uint32_t trigger_status_flag;
  uint32_t status_flag;
  /* Trigger interrupt flags */
  trigger_status_flag = LPADC_GetTriggerStatusFlags(ADC1_PERIPHERAL);
  /* Interrupt flags */
  status_flag = LPADC_GetStatusFlags(ADC1_PERIPHERAL);
  /* Clears trigger interrupt flags */
  LPADC_ClearTriggerStatusFlags(ADC1_PERIPHERAL, trigger_status_flag);
  /* Clears interrupt flags */
  LPADC_ClearStatusFlags(ADC1_PERIPHERAL, status_flag);

  float VDC_adc =  (float)(((uint16_t) (ADC1->RESFIFO[0] & 0xFFFF))/((float) (1<<16))*3.3);
  voltDCB = (VDC_adc * 10)/(47+10); // Voltage divider
}


 /* UTICK0_IRQHandler interrupt handler , 1 kHz frequency*/
/*void UTICK0_Callback(void) {
    GPIO_PortToggle(BOARD_LED_GPIO, 1u << BOARD_LED_GPIO_PIN);
    if(AppSwitch){
        uint32_t channel1_duty = (period_channel_a-500)/1000;
        setDutycycleBothMotors(channel1_duty);
        CTIMER_UpdatePwmPulsePeriod (CTIMER2, kCTIMER_Match_0, period_channel_b);
    }
} */

void CTIMER1_IRQHandler(void){
    
    if(AppSwitch){
        
        if (appMode == kappMode_remotecontrolled){
            float channel1_duty = (period_channel_a-1000)/10.0;
            global_dutycycle = channel1_duty;
            global_servo_period = period_channel_b;
        }
        if (appMode == kappMode_autonomous){
            global_dutycycle = MAX_AUTO_DUTYCYCLE;
            global_servo_period = (global_servo_angle/45)*500+1500;
        }

        setSpeedDutyCycle(global_dutycycle);
        CTIMER_SetupPwmPeriod(CTIMER2, kCTIMER_Match_0, kCTIMER_Match_1, CTIMER2_PWM_PERIOD, CTIMER2_PWM_PERIOD - global_servo_period, false);
    }

    CTIMER_ClearStatusFlags(CTIMER1, kCTIMER_Match0Flag);
}

void CTIMER0_IRQHandler(void){
    uint32_t diff_us; 

    diff_us = CTIMER0->CR[2]-CTIMER0->CR[0];
    diff_us = (diff_us > 2550) ? 0 : diff_us;
    period_channel_a = SERVO_LIMITS_US ( diff_us );

    diff_us = CTIMER0->CR[3]-CTIMER0->CR[1];
    diff_us = (diff_us > 2550) ? 1500 : diff_us;
    period_channel_b = SERVO_LIMITS_US ( diff_us );

    CTIMER_ClearStatusFlags(CTIMER0, kCTIMER_Match0Flag);
}

/* GPIO00_IRQn interrupt handler */
void GPIO0_INT_0_IRQHANDLER(void) {
  /* Get pin flags 0 */
  uint32_t pin_flags0 = GPIO_GpioGetInterruptChannelFlags(GPIO0, 0U);

  if(pin_flags0 & (1 << BOARD_INITBUTTONSPINS_SW2_PIN)){
    if(AppSwitch){
        AppSwitch = 0;
        setSpeedDutyCycle(0.0);
        //PWM_OutputDisable(PWM1,kPWM_PwmA,kPWM_Module_0);
        //PWM_OutputDisable(PWM1,kPWM_PwmA,kPWM_Module_1);
    }else{
        AppSwitch = 1;
        setSpeedDutyCycle(global_dutycycle);
        //PWM_OutputEnable(PWM1,kPWM_PwmA,kPWM_Module_0);
        //PWM_OutputEnable(PWM1,kPWM_PwmA,kPWM_Module_1);
    }
  }
  /* Clear pin flags 0 */
  GPIO_GpioClearInterruptChannelFlags(GPIO0, pin_flags0, 0U); 
}

/* GPIO01_IRQn interrupt handler */
void GPIO0_INT_1_IRQHANDLER(void) {
    /* Get pin flags 1 */
    uint32_t pin_flags1 = GPIO_GpioGetInterruptChannelFlags(GPIO0, 1U);

    if(pin_flags1 & (1 << BOARD_INITBUTTONSPINS_BUTTON_CLICK_PIN)){
        if(appMode == kappMode_remotecontrolled){
            appMode = kappMode_autonomous;
        }else{
            appMode = kappMode_remotecontrolled;
        }
    }

    /* Clear pin flags 1 */
    GPIO_GpioClearInterruptChannelFlags(GPIO0, pin_flags1, 1U); 
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Board pin init */
    BOARD_InitHardware();
    BOARD_InitDebugConsole();
    EnableIRQ(GPIO00_IRQn);
    EnableIRQ(GPIO01_IRQn);
    EnableIRQ(ADC0_IRQn);
    EnableIRQ(ADC1_IRQn);
    //EnableIRQ(UTICK0_IRQn);
    EnableIRQ(CTIMER0_IRQn);
    EnableIRQ(CTIMER1_IRQn);
    EnableIRQ(LPTMR0_IRQn);
    LED_GREEN_INIT(1);
    LED_RED_INIT(1);
    LED_BLUE_INIT(1);

    pixy_t cam1;
    pixy_init(&cam1, LPI2C2, 0x54U, &LP_FLEXCOMM2_RX_Handle, &LP_FLEXCOMM2_TX_Handle);
    //pixy_set_led(&cam1, 255, 0, 0);

    while (1)
    {
        if (pixy_get_vectors(&cam1, vectors, MAX_VECTORS, &num_vectors) == kStatus_Success) {
            float angle = 0.0;
            for (size_t i = 0; i < num_vectors; i++) {
                uint16_t x0 = vectors[4*i + 0];
                uint16_t y0 = vectors[4*i + 1];
                uint16_t x1 = vectors[4*i + 2];
                uint16_t y1 = vectors[4*i + 3];
                //PRINTF("  [%2u] (%u,%u)->(%u,%u)\r\n", (unsigned)i, x0, y0, x1, y1);
                float m = ((float)x0-(float)x1) / ((float)y0-(float)y1);
                angle += m;
            }
            angle *= -1;
            //PRINTF("Angle: %u" , angle);
            if(angle > 0)
                angle *= STEERING_P_RIGHT;
            else{
                angle *= STEERING_P_LEFT;
            }
            if (angle > STEERING_LIMIT_RIGHT){
                angle = STEERING_LIMIT_RIGHT;
            }
            if (angle < STEERING_LIMIT_LEFT){
                angle = STEERING_LIMIT_LEFT;
            }
            if(num_vectors !=0)
                global_servo_angle = (angle + STEERING_OFFSET);
        }
        //UTICK0_Callback();
        ADC1->SWTRIG = (uint32_t) 0b0010;
        //PRINTF("VDC: %d \n\r", &voltDCB);
        //PRINTF("M1 I: %d \n\r", &gControlPanelM1.currentMeas);
        //PRINTF("M2 I: %d \n\r", &gControlPanelM2.currentMeas);
    }
}
