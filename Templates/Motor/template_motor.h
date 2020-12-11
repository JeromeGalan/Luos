/******************************************************************************
 * @file Template motor
 * @brief motor container template
 * WARING : This h file should be only included by user code or class_*.h codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TEMPLATE_TEMPLATE_MOTOR_H_
#define TEMPLATE_TEMPLATE_MOTOR_H_

#include "luos.h"
#include "struct_motor.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * motor object structure
 */
typedef struct
{
    CONT_CB self;
    class_motor_t class;
} template_motor_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

container_t *TemplateMotor_CreateContainer(CONT_CB cont_cb, template_motor_t *var, const char *alias, revision_t revision);

#endif /* TEMPLATE_TEMPLATE_MOTOR_H_ */
