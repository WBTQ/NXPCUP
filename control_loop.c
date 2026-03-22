#include "control_loop.h"
#include "AMCLIB_TrackObsrv_FLT.h"
#include "fsl_pwm.h"
#include "fsl_gpio.h"
#include "pin_mux.h"


void initControlLoop(control_panel_t* Motor){
    //Init inner PI
    GFLIB_CtrlPIpAWInit_FLT_Ci(0,&Motor->innerController);

    
    switch (Motor->controlMode)
    {
    case kctrlMode_dutycyle:

        break;

    case kctrlMode_current:
        //Init inner PI
        GFLIB_CtrlPIpAWInit_FLT_Ci(0,&Motor->innerController);
        break;

    case kctrlMode_speed:
        //Init inner PI
        GFLIB_CtrlPIpAWInit_FLT_Ci(0,&Motor->innerController);
        //Init outer PI
        GFLIB_CtrlPIpAWInit_FLT_Ci(0,&Motor->outerController);
        break;

    default:
        break;
    }
}


void updateFastLoop(control_panel_t* Motor){
    switch (Motor->controlMode)
    {
    case kctrlMode_dutycyle:
        /* code */
        break;

    case kctrlMode_current:
        /* code */
        break;

    case kctrlMode_speed:
        /* code */
        break;    

    default:
        break;
    }
}


void updateSlowLoop(control_panel_t* Motor){
    switch (Motor->controlMode)
    {
    case kctrlMode_dutycyle:
        /* code */
        break;

    case kctrlMode_current:
        /* code */
        break;

    case kctrlMode_speed:
        /* code */
        break;    

    default:
        break;
    }
}

void setDutycycleMotor1(float dutycycle){
    PWM_UpdatePwmDutycycleHighAccuracy(PWM1, kPWM_Module_0, kPWM_PwmA, kPWM_SignedCenterAligned, (uint16_t)(dutycycle/100.0*65535));
    PWM_SetPwmLdok(PWM1, kPWM_Control_Module_0,1);
}

void setDutycycleMotor2(float dutycycle){
    PWM_UpdatePwmDutycycleHighAccuracy(PWM1, kPWM_Module_1, kPWM_PwmA, kPWM_SignedCenterAligned, (uint16_t)(dutycycle/100.0*65535));
    PWM_SetPwmLdok(PWM1, kPWM_Control_Module_1,1);
}


void setDutycycleBothMotors(float dutycycle){
    setDutycycleMotor1(dutycycle);
    setDutycycleMotor2(dutycycle);
    PWM_SetPwmLdok(PWM1, kPWM_Control_Module_0 | kPWM_Control_Module_1 , 1);
}

void setSpeedDutyCycle(float dutycycle){
    
    uint8_t ina;
    uint8_t inb;
    if (dutycycle == 0){
        ina=0;
        inb=0;
    }else{
        ina=1;
        inb=0;
    }

    GPIO_PinWrite(BOARD_M1_INA_GPIO,BOARD_M1_INA_GPIO_PIN,ina);
    GPIO_PinWrite(BOARD_M1_INB_GPIO,BOARD_M1_INB_GPIO_PIN,inb);
    GPIO_PinWrite(BOARD_M2_INA_GPIO,BOARD_M2_INA_GPIO_PIN,ina);
    GPIO_PinWrite(BOARD_M2_INB_GPIO,BOARD_M2_INB_GPIO_PIN,inb);
    setDutycycleBothMotors(max(dutycycle , 0));
    
}