#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "unity.h"
#include "../src/msg_alloc.c"


/******************************************************************************
 * @brief check if there is enough space to store this data in buffer
 *****************************************************************************/
void unittest_DoWeHaveSpace(void)
{
    // Case 1 : there is enough space
    for(int i=0; i<MSG_BUFFER_SIZE; i++)
    {
        TEST_ASSERT_EQUAL(MsgAlloc_DoWeHaveSpace((void *)&msg_buffer[i]), SUCCEED);
    }

    // Case 2 : there is not enough space
    for(int i=MSG_BUFFER_SIZE; i<MSG_BUFFER_SIZE+10; i++)
    {
        TEST_ASSERT_EQUAL(MsgAlloc_DoWeHaveSpace((void *)&msg_buffer[i]), FAILED);
    }
}


/******************************************************************************
 * @brief Check if there is space in buffer
 *****************************************************************************/
void unittest_CheckMsgSpace(void)
{
    // Declaration of dummy message start and end pointer 
    uint32_t *mem_start;
    uint32_t *mem_end;

    // Initialize pointer to buffer beginning    
    used_msg = (msg_t *)&msg_buffer[0];

    // Test function considering "used_msg"
    // ("oldest_message" unit testing is useless as it's the same behaviour)
    mem_start = (uint32_t*)used_msg - 2;
    mem_end  = mem_start + 1;
    TEST_ASSERT_EQUAL(MsgAlloc_CheckMsgSpace((void *)mem_start, (void *)mem_end), SUCCEED);

    mem_start = (uint32_t*)used_msg - 1;
    mem_end  = mem_start + 1;
    TEST_ASSERT_EQUAL(MsgAlloc_CheckMsgSpace((void *)mem_start, (void *)mem_end), FAILED);

    mem_start = (uint32_t*)used_msg;
    mem_end  = mem_start + 1;
    TEST_ASSERT_EQUAL(MsgAlloc_CheckMsgSpace((void *)mem_start, (void *)mem_end), FAILED);

    mem_start = (uint32_t*)used_msg + 1;
    mem_end  = mem_start + 1;
    TEST_ASSERT_EQUAL(MsgAlloc_CheckMsgSpace((void *)mem_start, (void *)mem_end), SUCCEED);
}


int main(int argc, char **argv)
{
    UNITY_BEGIN();	
    RUN_TEST(unittest_DoWeHaveSpace);
    RUN_TEST(unittest_CheckMsgSpace); 
    UNITY_END();
}
