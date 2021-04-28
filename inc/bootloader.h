/******************************************************************************
 * @file Bootloader
 * @brief Bootloader functionnalities for luos framework
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef BOOTLOADER_H
#define BOOTLOADER_H

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SHARED_MEMORY_ADDRESS   0x0800C000
#define SHARED_FLASH_PAGE       25
#define APP_ADDRESS             (uint32_t)0x0800C800

#define BOOTLOADER_MODE         0x00
#define APPLICATION_MODE        0x01

// find shared memory section defined in linker file
extern int __my_shared_mem_start;
extern int __my_shared_mem_end;

#define BOOTLOADER_FIRST_COMMAND    0x01
typedef enum
{
    BOOTLOADER_START = BOOTLOADER_FIRST_COMMAND,
    BOOTLOADER_STOP,
    BOOTLOADER_READY,
    BOOTLOADER_BIN_HEADER,
    BOOTLOADER_BIN_CHUNK,
    BOOTLOADER_BIN_END,
    BOOTLOADER_CRC_TEST,
} bootloader_cmd_t;

typedef enum
{
    BOOTLOADER_IDLE_STATE,
    BOOTLOADER_STOP_STATE,
    BOOTLOADER_READY_STATE,
    BOOTLOADER_BIN_HEADER_STATE,
    BOOTLOADER_BIN_CHUNK_STATE,
    BOOTLOADER_BIN_END_STATE,
    BOOTLOADER_CRC_TEST_STATE,
} bootloader_state_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Function
 ******************************************************************************/

/******************************************************************************
 * @brief Main function used by the bootloader app
 ******************************************************************************/
void LuosBootloader_Run(void);

/******************************************************************************
 * @brief function used by Luos to send message to the bootloader
 ******************************************************************************/
void LuosBootloader_MsgHandler(uint8_t*);

#endif /* BOOTLOADER_H */