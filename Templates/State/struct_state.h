/******************************************************************************
 * @file Struct state
 * @brief state data structure definition
 * WARING : This h file should be only included by class_*.h or template_*.h codes
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef TEMPLATE_STRUCT_STATE_H_
#define TEMPLATE_STRUCT_STATE_H_

#include <stdbool.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*
 * State data
 */
typedef struct
{
    access_t access;
    bool state;
} class_state_t;

#endif /* TEMPLATE_STRUCT_STATE_H_ */
