/******************************************************************************
 * @file Bootloader
 * @brief Bootloader functionnalities for luos framework
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "bootloader.h"
#include "luos_hal.h"
#include "luos.h"

// TODO : juste pour le debug
#include "boot.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static bootloader_state_t bootloader_state = BOOTLOADER_IDLE_STATE;
static uint8_t* bootloader_data;
static bootloader_cmd_t bootloader_cmd;

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief read the boot mode in flash memory
 * @param None
 * @return boot_mode
 ******************************************************************************/
uint8_t LuosBootloader_GetMode(void)
{
    uint32_t* p_start = (uint32_t*)SHARED_MEMORY_ADDRESS;
    uint8_t boot_mode = 0x00;
    
    if(*p_start==0x00000000)
        boot_mode = BOOTLOADER_MODE;
    else
        boot_mode = APPLICATION_MODE;

    return boot_mode;
}

/******************************************************************************
 * @brief DeInit peripherals in bootloader
 * @param None
 * @return None
 ******************************************************************************/
void LuosBootloader_DeInit(void)
{
    LuosHAL_DeInit();
}

/******************************************************************************
 * @brief launch application from the bootloader
 * @param None
 * @return None
 ******************************************************************************/
void LuosBootloader_JumpToApp(void)
{
    LuosHAL_JumpToApp(APP_ADDRESS);
}

/******************************************************************************
 * @brief set state of the bootloader
 * @param state 
 * @return None
 ******************************************************************************/
void LuosBootloader_SetState(bootloader_state_t state)
{
    bootloader_state = state;
}

/******************************************************************************
 * @brief Bootloader machine state
 * @param None
 * @return None
 ******************************************************************************/
void LuosBootloader_Task(void)
{
    switch(bootloader_state)
    {
        case BOOTLOADER_IDLE_STATE:
            // for debug purpose
            change_blink(3);

            // if STOP_CMD, restart the node
            if(bootloader_cmd == BOOTLOADER_STOP)
            {
                LuosBootloader_SetState(BOOTLOADER_STOP_STATE);
            }
            // if READY_CMD, continue BOOTLOADER process
            if(bootloader_cmd == BOOTLOADER_READY)
            {
                // send READY resp and go to HEADER state

                LuosBootloader_SetState(BOOTLOADER_BIN_HEADER_STATE);
            }
            break;

        case BOOTLOADER_STOP_STATE:
            // save boot_mode in flash
            LuosHAL_SetApplicationMode();
            // reboot the node
            LuosHAL_Reboot();
            break;

        case BOOTLOADER_BIN_HEADER_STATE:
            // for debug purpose
            change_blink(5);

            if(bootloader_cmd == BOOTLOADER_BIN_HEADER)
            {
                // handle header data

                // go to BIN_CHUNK state
                LuosBootloader_SetState(BOOTLOADER_BIN_CHUNK_STATE);
            }
            break;

        case BOOTLOADER_BIN_CHUNK_STATE:
            if(bootloader_cmd == BOOTLOADER_BIN_CHUNK)
            {
                // handle binary data
            }

            if(bootloader_cmd == BOOTLOADER_BIN_END)
            {
                // go to BIN_END state
                LuosBootloader_SetState(BOOTLOADER_BIN_END_STATE);
            }
            break;

        case BOOTLOADER_BIN_END_STATE:
            // go to CRC_TEST state
            LuosBootloader_SetState(BOOTLOADER_CRC_TEST_STATE);
            break;

        case BOOTLOADER_CRC_TEST_STATE:
            // go to IDLE state
            LuosBootloader_SetState(BOOTLOADER_IDLE_STATE);
            break;

        default:
            break;
    }
}

/******************************************************************************
 * @brief Message handler called from luos library
 * @param data received from luos network
 * @return None
 ******************************************************************************/
void LuosBootloader_MsgHandler(uint8_t* data)
{
    bootloader_cmd = data[0];

    if(bootloader_cmd == BOOTLOADER_START)
    {
        // We're in the app, set bootloader mode and reboot
        LuosHAL_SetBootloaderMode();
        LuosHAL_Reboot();
    }
    else
    {
        // we're in the bootloader, process cmd and data
        bootloader_data = &data[1];
    }
}

/******************************************************************************
 * @brief bootloader app
 * @param None
 * @return None
 ******************************************************************************/
void LuosBootloader_Run(void)
{
    switch(LuosBootloader_GetMode())    
    {
        case BOOTLOADER_MODE:
            Luos_Init();
            Boot_Init(); // pour le debug
            while (1)
            {
                Luos_Loop();
                Boot_Loop(); // pour le debug
                LuosBootloader_Task();
            }
            break;

        case APPLICATION_MODE:
            // boot the application programmed in dedicated flash partition
            LuosBootloader_DeInit();
            LuosBootloader_JumpToApp();
            break;

        default:
            break;
    }
}