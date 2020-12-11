/******************************************************************************
 * @file Class state
 * @brief state object managing a true false API
 * WARING : This h file should be only included by class_*.c codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TEMPLATE_CLASS_STATE_H_
#define TEMPLATE_CLASS_STATE_H_

#include "struct_state.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

void ClassState_Handler(container_t *container, msg_t *msg, class_state_t *state_class);

#endif /* TEMPLATE_CLASS_STATE_H_ */
