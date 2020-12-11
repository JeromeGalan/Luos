/******************************************************************************
 * @file Class voltage
 * @brief voltage object
 * WARING : This h file should be only included by class_*.c codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TEMPLATE_CLASS_VOLTAGE_H_
#define TEMPLATE_CLASS_VOLTAGE_H_

#include "struct_voltage.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

void ClassVoltage_Handler(container_t *container, msg_t *msg, class_voltage_t *voltage_class);

#endif /* TEMPLATE_CLASS_VOLTAGE_H_ */
