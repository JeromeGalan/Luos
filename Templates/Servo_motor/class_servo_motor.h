/******************************************************************************
 * @file Class servo motor
 * @brief servo motor class
 * WARING : This h file should be only included by class_*.c codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TEMPLATE_CLASS_SERVO_MOTOR_H_
#define TEMPLATE_CLASS_SERVO_MOTOR_H_

#include "struct_servo_motor.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

void ClassServoMotor_Handler(container_t *container, msg_t *msg, class_servo_motor_t *class_servo_motor);

#endif /* TEMPLATE_CLASS_SERVO_MOTOR_H_ */
