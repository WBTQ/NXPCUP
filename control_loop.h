/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _CONTROL_LOOP_H_
#define _CONTROL_LOOP_H_


#include "GFLIB_CtrlPIp_FLT.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
typedef enum _control_mode {
    kctrlMode_dutycyle,
    kctrlMode_current,
    kctrlMode_speed,
} control_mode_t;


typedef struct _control_panel{
    control_mode_t controlMode; 
    GFLIB_CTRL_PI_P_AW_T_FLT innerController;
    GFLIB_CTRL_PI_P_AW_T_FLT outerController;
    float currentRef;
    float currentMeas;
    float speedRef;
    float speedMeas;
    uint32_t outDutycycle;

} control_panel_t;

extern control_panel_t gControlPanelM1;
extern control_panel_t gControlPanelM2;


void initControlLoop(control_panel_t* Motor);
void updateFastLoop(control_panel_t* Motor);
void updateSlowLoop(control_panel_t* Motor);
void setDutycycleMotor1(float dutycycle);
void setDutycycleMotor2(float dutycycle);
void setDutycycleBothMotors(float dutycycle);
void setSpeedDutyCycle(float dutycycle);

#endif /* _CONTROL_LOOP_H_ */