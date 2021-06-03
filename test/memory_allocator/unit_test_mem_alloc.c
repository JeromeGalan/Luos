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


void unittest_BufferAvailableSpaceComputation(void)
{
    TEST_ASSERT_EQUAL(1,1);

    /*printf("\n\n\n*******Debug*******\n");
    printf("used_msg : %d\n", used_msg);
    printf("oldest_msg : %d\n", oldest_msg);
    printf("\n*******END Debug*******\n\n\n\n");*/
    
    //ICI !!!!!!!!!!!!
    //MsgAlloc_BufferAvailableSpaceComputation();

    /******************************************************************************
    // * brief compute remaing space on msg_buffer.
    // * param None
    // * return Available space in bytes
    // *****************************************************************************
    static inline uint32_t MsgAlloc_BufferAvailableSpaceComputation(void)
    {
        uint32_t stack_free_space = 0;

        LuosHAL_SetIrqState(false);
        if ((uint32_t)oldest_msg != 0xFFFFFFFF)
        {
            LUOS_ASSERT(((uint32_t)oldest_msg >= (uint32_t)&msg_buffer[0]) && ((uint32_t)oldest_msg < (uint32_t)&msg_buffer[MSG_BUFFER_SIZE]));
            // There is some tasks
            if ((uint32_t)oldest_msg > (uint32_t)data_end_estimation)
            {
                // The oldest task is between `data_end_estimation` and the end of the buffer
                stack_free_space = (uint32_t)oldest_msg - (uint32_t)data_end_estimation;
                LuosHAL_SetIrqState(true);
            }
            else
            {
                // The oldest task is between the begin of the buffer and `current_msg`
                stack_free_space = ((uint32_t)oldest_msg - (uint32_t)&msg_buffer[0]) + ((uint32_t)&msg_buffer[MSG_BUFFER_SIZE] - (uint32_t)data_end_estimation);
                LuosHAL_SetIrqState(true);
            }
        }
        else
        {
            // There is no task yet just compute the actual reception
            stack_free_space = MSG_BUFFER_SIZE - ((uint32_t)data_end_estimation - (uint32_t)current_msg);
            LuosHAL_SetIrqState(true);
        }
        return stack_free_space;
    }*/

}

void unittest_OldestMsgCandidate(void)
{
    TEST_ASSERT_EQUAL(1,1);

    /******************************************************************************
    // * brief save the given msg as oldest if it is
    // * param oldest_stack_msg_pt : the oldest message of a stack
    // * return None
     ******************************************************************************
    static inline void MsgAlloc_OldestMsgCandidate(msg_t *oldest_stack_msg_pt)
    {
        if ((uint32_t)oldest_stack_msg_pt > 0)
        {
            LUOS_ASSERT(((uint32_t)oldest_stack_msg_pt >= (uint32_t)&msg_buffer[0]) && ((uint32_t)oldest_stack_msg_pt < (uint32_t)&msg_buffer[MSG_BUFFER_SIZE]));
            // recompute oldest_stack_msg_pt into delta byte from current message
            uint32_t stack_delta_space;
            if ((uint32_t)oldest_stack_msg_pt > (uint32_t)current_msg)
            {
                // The oldest task is between `data_end_estimation` and the end of the buffer
                stack_delta_space = (uint32_t)oldest_stack_msg_pt - (uint32_t)current_msg;
            }
            else
            {
                // The oldest task is between the begin of the buffer and `data_end_estimation`
                // we have to decay it to be able to define delta
                stack_delta_space = ((uint32_t)oldest_stack_msg_pt - (uint32_t)&msg_buffer[0]) + ((uint32_t)&msg_buffer[MSG_BUFFER_SIZE] - (uint32_t)current_msg);
            }
            // recompute oldest_msg into delta byte from current message
            uint32_t oldest_msg_delta_space;
            if ((uint32_t)oldest_msg > (uint32_t)current_msg)
            {
                // The oldest msg is between `data_end_estimation` and the end of the buffer
                oldest_msg_delta_space = (uint32_t)oldest_msg - (uint32_t)current_msg;
            }
            else
            {
                // The oldest msg is between the begin of the buffer and `data_end_estimation`
                // we have to decay it to be able to define delta
                oldest_msg_delta_space = ((uint32_t)oldest_msg - (uint32_t)&msg_buffer[0]) + ((uint32_t)&msg_buffer[MSG_BUFFER_SIZE] - (uint32_t)current_msg);
            }
            // Compare deltas
            if (stack_delta_space < oldest_msg_delta_space)
            {
                // This one is the new oldest message
                oldest_msg = oldest_stack_msg_pt;
            }
        }
    }*/
}


int main(int argc, char **argv)
{
    UNITY_BEGIN();	
    RUN_TEST(unittest_DoWeHaveSpace);
    RUN_TEST(unittest_CheckMsgSpace);
    RUN_TEST(unittest_BufferAvailableSpaceComputation);
    RUN_TEST(unittest_OldestMsgCandidate);    
    UNITY_END();
}


/*void MsgAlloc_Init(memory_stats_t *memory_stats)
{
    //******** Init global vars pointers **********
    current_msg = (msg_t *)&msg_buffer[0];
    data_ptr = (uint8_t *)&msg_buffer[0];
    data_end_estimation = (uint8_t *)&current_msg->data[2];
    msg_tasks_stack_id = 0;
    memset((void *)msg_tasks, 0, sizeof(msg_tasks));
    luos_tasks_stack_id = 0;
    memset((void *)luos_tasks, 0, sizeof(luos_tasks));
    tx_tasks_stack_id = 0;
    memset((void *)tx_tasks, 0, sizeof(tx_tasks));
    copy_task_pointer = NULL;
    used_msg = NULL;
    oldest_msg = (msg_t *)0xFFFFFFFF;
    mem_clear_needed = false;
    if (memory_stats != NULL)
    {
        mem_stat = memory_stats;
    }
}*/


/*
23 fcts :

void MsgAlloc_Init(memory_stats_t *memory_stats);
void MsgAlloc_loop(void);
void MsgAlloc_ValidHeader(uint8_t valid, uint16_t data_size);
void MsgAlloc_InvalidMsg(void);
void MsgAlloc_EndMsg(void);
void MsgAlloc_SetData(uint8_t data);
error_return_t MsgAlloc_IsEmpty(void);
void MsgAlloc_UsedMsgEnd(void);
error_return_t MsgAlloc_PullMsgToInterpret(msg_t **returned_msg);
void MsgAlloc_LuosTaskAlloc(ll_container_t *container_concerned_by_current_msg, msg_t *concerned_msg);
error_return_t MsgAlloc_PullMsg(ll_container_t *target_container, msg_t **returned_msg);
error_return_t MsgAlloc_PullMsgFromLuosTask(uint16_t luos_task_id, msg_t **returned_msg);
error_return_t MsgAlloc_LookAtLuosTask(uint16_t luos_task_id, ll_container_t **allocated_container);
error_return_t MsgAlloc_GetLuosTaskSourceId(uint16_t luos_task_id, uint16_t *source_id);
error_return_t MsgAlloc_GetLuosTaskCmd(uint16_t luos_task_id, uint8_t *cmd);
error_return_t MsgAlloc_GetLuosTaskSize(uint16_t luos_task_id, uint16_t *size);
uint16_t MsgAlloc_LuosTasksNbr(void);
void MsgAlloc_ClearMsgFromLuosTasks(msg_t *msg);
error_return_t MsgAlloc_SetTxTask(ll_container_t *ll_container_pt, uint8_t *data, uint16_t crc, uint16_t size, uint8_t locahost, uint8_t ack);
void MsgAlloc_PullMsgFromTxTask(void);
void MsgAlloc_PullContainerFromTxTask(uint16_t container_id);
error_return_t MsgAlloc_GetTxTask(ll_container_t **ll_container_pt, uint8_t **data, uint16_t *size, uint8_t *locahost);
error_return_t MsgAlloc_TxAllComplete(void);
*/