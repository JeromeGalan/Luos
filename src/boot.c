/******************************************************************************
 * @file Boot
 * @brief fonctionnalities for luos bootloader
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/

#include "main.h"
#include "boot.h"
#include "string.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define REV {1,0,0}

/*******************************************************************************
 * Variables
 ******************************************************************************/
typedef enum
{
    BOOT_APP = LUOS_LAST_TYPE
} App_type_t;

uint16_t blinktime = 100;
unsigned long my_time; //Used to keep track of time
uint8_t led_last_state = 0; //Is the LED on or off?
uint8_t count = 0;

/*******************************************************************************
 * Function
 ******************************************************************************/
static void Boot_MsgHandler(container_t *container, msg_t *msg);

/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Boot_Init(void)
{
	revision_t revision = {.unmap = REV};
  Luos_CreateContainer(Boot_MsgHandler, BOOT_APP, "boot_app", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Boot_Loop(void)
{
  //Check to see if we have overshot our counter
	if (my_time < HAL_GetTick())
	{
    //Reset the counter
    my_time = HAL_GetTick() + blinktime;

    if (count < 3)
    {
      count++;
      blinktime = 100;
    }
    else
    {
      count = 0;
      blinktime = 1000;
    }

    //Invert the LED state
    led_last_state = (led_last_state == 1 ? 0: 1);

    if (led_last_state)
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    else
      HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	}
}
/******************************************************************************
 * @brief Msg manager callback when a msg receive for this container
 * @param Container destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Boot_MsgHandler(container_t *container, msg_t *msg)
{
}