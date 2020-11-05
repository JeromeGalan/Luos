/******************************************************************************
 * @file luos
 * @brief User functionalities of the Luos library
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "luos.h"
#include <stdio.h>
#include <stdbool.h>
#include "msgAlloc.h"
#include "robus.h"
#include "LuosHAL.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define STRINGIFY(s) STRINGIFY1(s)
#define STRINGIFY1(s) #s

/*******************************************************************************
 * Variables
 ******************************************************************************/
container_t container_table[MAX_VM_NUMBER];
uint16_t container_number;
volatile routing_table_t *routing_table_pt;

luos_stats_t luos_stats;
/*******************************************************************************
 * Function
 ******************************************************************************/
static error_return_t Luos_MsgHandler(container_t *container, msg_t *input);
static container_t *Luos_GetContainer(vm_t *vm);
static uint16_t Luos_GetContainerIndex(container_t *container);
static void Luos_TransmitLocalRoutingTable(container_t *container, msg_t *routeTB_msg);
static void Luos_AutoUpdateManager(void);
static error_return_t Luos_SaveAlias(container_t *container, uint8_t *alias);
static void Luos_WriteAlias(uint16_t local_id, uint8_t *alias);
static error_return_t Luos_ReadAlias(uint16_t local_id, uint8_t *alias);
static error_return_t Luos_IsALuosCmd(uint8_t cmd, uint16_t size);
static error_return_t Luos_ReceiveData(container_t *container, msg_t *msg, void *bin_data);

/******************************************************************************
 * @brief Luos init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Luos_Init(void)
{
    container_number = 0;
    memset(&luos_stats.unmap[0], 0, sizeof(luos_stats_t));
    Robus_Init(&luos_stats.memory);
}
/******************************************************************************
 * @brief Luos Loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Luos_Loop(void)
{
    static unsigned long last_loop_date;
    uint16_t remaining_msg_number = 0;
    vm_t *oldest_vm = NULL;
    msg_t *returned_msg = NULL;

    // check loop call time stat
    if ((LuosHAL_GetSystick() - last_loop_date) > luos_stats.max_loop_time_ms)
    {
        luos_stats.max_loop_time_ms = LuosHAL_GetSystick() - last_loop_date;
    }
    Robus_Loop();
    // look at all received messages
    while (MsgAlloc_LookAtLuosTask(remaining_msg_number, &oldest_vm) != FAIL)
    {
        // There is a message available find the container linked to it
        container_t *container = Luos_GetContainer(oldest_vm);
        // check if this is a Luos Command
        uint8_t cmd = 0;
        uint16_t size = 0;
        if (MsgAlloc_GetLuosTaskCmd(remaining_msg_number, &cmd) == FAIL)
        {
            // this is a critical failure we should never go here
            while (1)
                ;
        }
        if (MsgAlloc_GetLuosTaskSize(remaining_msg_number, &size) == FAIL)
        {
            // this is a critical failure we should never go here
            while (1)
                ;
        }
        //check if this msg cmd should be consumed by Luos_MsgHandler
        if (Luos_IsALuosCmd(cmd, size) == SUCESS)
        {
            if (MsgAlloc_PullMsgFromLuosTask(remaining_msg_number, &returned_msg) == SUCESS)
            {
                // be sure the content of this message need to be managed by Luos and do it if it is.
                if (Luos_MsgHandler((container_t *)container, returned_msg)==FAIL)
                {
                    // we should not go there there is a mistake on Luos_IsALuosCmd or Luos_MsgHandler
                    while (1)
                        ;
                }
                else
                {
                    // Luos CMD are generic for all continers and have to be executed only once
                    // Clear all luos tasks related to this message (in case of multicast message)
                    MsgAlloc_ClearMsgFromLuosTasks(returned_msg);
                }
            }
        }
        else
        {
            // This message is for a container
            // check if this continer have a callback?
            if (container->cont_cb != 0)
            {
                // This container have a callback pull the message
                if (MsgAlloc_PullMsgFromLuosTask(remaining_msg_number, &returned_msg) == SUCESS)
                {
                    // This message is for the user, pass it to the user.
                    container->cont_cb(container, returned_msg);
                }
            }
            else
            {
                remaining_msg_number++;
            }
        }
    }
    // manage timed auto update
    Luos_AutoUpdateManager();
    // save loop date
    last_loop_date = LuosHAL_GetSystick();
}
/******************************************************************************
 * @brief Check if this command concern luos
 * @param cmd The command value
 * @return Success if the command if for Luos else Fail 
 ******************************************************************************/
static error_return_t Luos_IsALuosCmd(uint8_t cmd, uint16_t size)
{
    switch (cmd)
    {
    case WRITE_NODE_ID:
    case RESET_DETECTION:
    case SET_BAUDRATE:
        // ERROR
        while (1)
            ;
        break;
    case RTB_CMD:
    case WRITE_ALIAS:
    case UPDATE_PUB:
        return SUCESS;
        break;

    case REVISION:
    case LUOS_REVISION:
    case NODE_UUID:
    case LUOS_STATISTICS:
        if (size == 0)
        {
            return SUCESS;
        }
        break;
    default:
        return FAIL;
        break;
    }
    return FAIL;
}
/******************************************************************************
 * @brief handling msg for Luos library
 * @param container
 * @param input msg
 * @param output msg
 * @return None
 ******************************************************************************/
static error_return_t Luos_MsgHandler(container_t *container, msg_t *input)
{
    error_return_t error = FAIL;
    msg_t output_msg;
    routing_table_t *route_tab = &RoutingTB_Get()[RoutingTB_GetLastEntry()];
    time_luos_t time;
    uint16_t base_id = 0;

    switch (input->header.cmd)
    {
    case RTB_CMD:
        // Depending on the size of this message we have to make different operations
        // If size is 0 someone ask to get local_route table back
        // If size is 2 someone ask us to generate a local route table based on the given container ID then send local route table back.
        // If size is bigger than 2 this is a complete routing table comming. We have to save it.
        switch (input->header.size)
        {
        case 2:
            // generate local ID
            RoutingTB_Erase();
            memcpy(&base_id, &input->data[0], sizeof(uint16_t));
            if (base_id == 1)
            {
                // set container Id based on received data except for the detector one.
                base_id = 2;
                int index = 0;
                for (int i = 0; i < container_number; i++)
                {
                    if (container_table[i].vm->id != 1)
                    {
                        container_table[i].vm->id = base_id + index;
                        index++;
                    }
                }
            }
            else
            {
                // set container Id based on received data
                for (int i = 0; i < container_number; i++)
                {
                    container_table[i].vm->id = base_id + i;
                }
            }
        case 0:
            // send back a local routing table
            output_msg.header.cmd = RTB_CMD;
            output_msg.header.target_mode = IDACK;
            output_msg.header.target = input->header.source;
            Luos_TransmitLocalRoutingTable(container, &output_msg);
            break;
        default:
            if (Luos_ReceiveData(container, input, (void *)route_tab) == SUCESS)
            {
                // route table section reception complete
                RoutingTB_ComputeRoutingTableEntryNB();
            }
            break;
        }
        error = SUCESS;
        break;
    case REVISION:
        if (input->header.size == 0)
        {
            msg_t output;
            output.header.cmd = REVISION;
            output.header.target_mode = ID;
            sprintf((char *)output.data, "%s", container->firm_version);
            memcpy(output.data, container->firm_version, sizeof(output.data));
            output.header.size = strlen((char *)output.data);
            output.header.target = input->header.source;
            Luos_SendMsg(container, &output);
            error = SUCESS;
        }
        break;
    case LUOS_REVISION:
        if (input->header.size == 0)
        {
            msg_t output;
            output.header.cmd = LUOS_REVISION;
            output.header.target_mode = ID;
            const char *luos_version = STRINGIFY(VERSION);
            sprintf((char *)output.data, "%s", luos_version);
            memcpy(output.data, luos_version, sizeof(output.data));
            output.header.size = strlen((char *)output.data);
            output.header.target = input->header.source;
            Luos_SendMsg(container, &output);
            error = SUCESS;
        }
        break;
    case NODE_UUID:
        if (input->header.size == 0)
        {
            msg_t output;
            output.header.cmd = NODE_UUID;
            output.header.target_mode = ID;
            output.header.size = sizeof(luos_uuid_t);
            output.header.target = input->header.source;
            luos_uuid_t uuid;
            uuid.uuid[0] = LUOS_UUID[0];
            uuid.uuid[1] = LUOS_UUID[1];
            uuid.uuid[2] = LUOS_UUID[2];
            memcpy(output.data, &uuid.unmap, sizeof(luos_uuid_t));
            Luos_SendMsg(container, &output);
            error = SUCESS;
        }
        break;
    case LUOS_STATISTICS:
        if (input->header.size == 0)
        {
            msg_t output;
            output.header.cmd = LUOS_STATISTICS;
            output.header.target_mode = ID;
            output.header.size = sizeof(luos_stats_t);
            output.header.target = input->header.source;
            memcpy(output.data, &luos_stats.unmap, sizeof(luos_stats_t));
            Luos_SendMsg(container, &output);
            error = SUCESS;
        }
        break;
    case WRITE_ALIAS:
        // Make a clean copy with full \0 at the end.
        memset(container->alias, '\0', MAX_ALIAS_SIZE);
        if (input->header.size > 16)
        {
            input->header.size = 16;
        }
        // check if there is no forbiden character
        uint8_t wrong = false;
        for (uint8_t i = 0; i < MAX_ALIAS_SIZE; i++)
        {
            if (input->data[i] == '\r')
            {
                wrong = true;
                break;
            }
        }
        if ((((input->data[0] >= 'A') & (input->data[0] <= 'Z')) | ((input->data[0] >= 'a') & (input->data[0] <= 'z'))) & (input->header.size != 0) & (wrong == false))
        {
            memcpy(container->alias, input->data, input->header.size);
            Luos_SaveAlias(container, container->alias);
        }
        else
        {
            // This is a wrong alias or an erase instruction, get back to default one
            Luos_SaveAlias(container, '\0');
            memcpy(container->alias, container->default_alias, MAX_ALIAS_SIZE);
        }
        error = SUCESS;
        break;
    case UPDATE_PUB:
        // this container need to be auto updated
        TimeOD_TimeFromMsg(&time, input);
        container->auto_refresh.target = input->header.source;
        container->auto_refresh.time_ms = (uint16_t)TimeOD_TimeTo_ms(time);
        container->auto_refresh.last_update = LuosHAL_GetSystick();
        error = SUCESS;
        break;
    default:
        break;
    }
    return error;
}
/******************************************************************************
 * @brief get pointer to a container in route table
 * @param vm
 * @return container from list
 ******************************************************************************/
static container_t *Luos_GetContainer(vm_t *vm)
{
    for (uint16_t i = 0; i < container_number; i++)
    {
        if (vm == container_table[i].vm)
        {
            return &container_table[i];
        }
    }
    return 0;
}
/******************************************************************************
 * @brief get this index of the container
 * @param container
 * @return container from list
 ******************************************************************************/
static uint16_t Luos_GetContainerIndex(container_t *container)
{
    for (uint16_t i = 0; i < container_number; i++)
    {
        if (container == &container_table[i])
        {
            return i;
        }
    }
    return 0xFFFF;
}
/******************************************************************************
 * @brief transmit local to network
 * @param none
 * @return none
 ******************************************************************************/
static void Luos_TransmitLocalRoutingTable(container_t *container, msg_t *routeTB_msg)
{
    uint16_t entry_nb = 0;
    routing_table_t local_routing_table[container_number + 1];

    //start by saving node entry
    RoutingTB_ConvertNodeToRoutingTable(&local_routing_table[entry_nb], Robus_GetNode());
    entry_nb++;
    // save containers entry
    for (uint16_t i = 0; i < container_number; i++)
    {
        RoutingTB_ConvertContainerToRoutingTable((routing_table_t *)&local_routing_table[entry_nb++], &container_table[i]);
    }
    Luos_SendData(container, routeTB_msg, (void *)local_routing_table, (entry_nb * sizeof(routing_table_t)));
}
/******************************************************************************
 * @brief auto update publication for container
 * @param none
 * @return none
 ******************************************************************************/
static void Luos_AutoUpdateManager(void)
{
    // check all containers timed_update_t contexts
    for (uint16_t i = 0; i < container_number; i++)
    {
        // check if containers have an actual ID. If not, we are in detection mode and should reset the auto refresh
        if (container_table[i].vm->id == DEFAULTID)
        {
            // this container have not been detected or is in detection mode. remove auto_refresh parameters
            container_table[i].auto_refresh.target = 0;
            container_table[i].auto_refresh.time_ms = 0;
            container_table[i].auto_refresh.last_update = 0;
        }
        else
        {
            // check if there is a timed update setted and if it's time to update it.
            if (container_table[i].auto_refresh.time_ms)
            {
                if ((LuosHAL_GetSystick() - container_table[i].auto_refresh.last_update) >= container_table[i].auto_refresh.time_ms)
                {
                    // This container need to send an update
                    // Create a fake message for it from the container asking for update
                    msg_t updt_msg;
                    updt_msg.header.target = container_table[i].vm->id;
                    updt_msg.header.source = container_table[i].auto_refresh.target;
                    updt_msg.header.target_mode = IDACK;
                    updt_msg.header.cmd = ASK_PUB_CMD;
                    updt_msg.header.size = 0;
                    if ((container_table[i].cont_cb != 0))
                    {
                        container_table[i].cont_cb(&container_table[i], &updt_msg);
                    }
                    else
                    {
                        //store container and msg pointer
                        // todo this can't work for now because this message is not permanent.
                        //mngr_set(&container_table[i], &updt_msg);
                    }
                    container_table[i].auto_refresh.last_update = LuosHAL_GetSystick();
                }
            }
        }
    }
}
/******************************************************************************
 * @brief clear list of container
 * @param none
 * @return none
 ******************************************************************************/
void Luos_ContainersClear(void)
{
    container_number = 0;
    Robus_ContainersClear();
}
/******************************************************************************
 * @brief API to Create a container
 * @param callback msg handler for the container
 * @param type of container corresponding to object dictionnary
 * @param alias for the container string (15 caracters max).
 * @param version FW for the container
 * @return container object pointer.
 ******************************************************************************/
container_t *Luos_CreateContainer(CONT_CB cont_cb, uint8_t type, const char *alias, char *firm_revision)
{
    uint8_t i = 0;
    container_t *container = &container_table[container_number];
    container->vm = Robus_ContainerCreate(type);

    // Link the container to his callback
    container->cont_cb = cont_cb;
    // Save default alias
    for (i = 0; i < MAX_ALIAS_SIZE - 1; i++)
    {
        container->default_alias[i] = alias[i];
        if (container->default_alias[i] == '\0')
            break;
    }
    container->default_alias[i] = '\0';
    // Initialise the container alias to 0
    memset((void *)container->alias, 0, sizeof(container->alias));
    if (Luos_ReadAlias(container_number, (uint8_t *)container->alias) == FAIL)
    {
        // if no alias saved keep the default one
        for (i = 0; i < MAX_ALIAS_SIZE - 1; i++)
        {
            container->alias[i] = alias[i];
            if (container->alias[i] == '\0')
                break;
        }
        container->alias[i] = '\0';
    }

    //Initialise the container firm_version to 0
    memset((void *)container->firm_version, 0, sizeof(container->firm_version));
    // Save firmware version
    for (i = 0; i < 20; i++)
    {
        container->firm_version[i] = firm_revision[i];
        if (container->firm_version[i] == '\0')
            break;
    }

    container_number++;
    return container;
}
/******************************************************************************
 * @brief Send msg through network
 * @param Container who send
 * @param Message to send
 * @return error
 ******************************************************************************/
error_return_t Luos_SendMsg(container_t *container, msg_t *msg)
{
    return Robus_SendMsg(container->vm, msg);
}
/******************************************************************************
 * @brief read last msg from buffer for a container
 * @param container who receive the message we are looking for
 * @param returned_msg oldest message of the container
 * @return FAIL if no message available
 ******************************************************************************/
error_return_t Luos_ReadMsg(container_t *container, msg_t **returned_msg)
{
    error_return_t error = SUCESS;
    while (error == SUCESS)
    {
        error = MsgAlloc_PullMsg(container->vm, returned_msg);
        // check if the content of this message need to be managed by Luos and do it if it is.
        if ((Luos_MsgHandler(container, *returned_msg)==FAIL) & (error == SUCESS))
        {
            // This message is for the user, pass it to the user.
            return SUCESS;
        }
    }
    return FAIL;
}
/******************************************************************************
 * @brief read last msg from buffer from a special id container
 * @param container who receive the message we are looking for
 * @param id who sent the message we are looking for
 * @param returned_msg oldest message of the container
 * @return FAIL if no message available
 ******************************************************************************/
error_return_t Luos_ReadFromContainer(container_t *container, short id, msg_t **returned_msg)
{
    uint16_t remaining_msg_number = 0;
    vm_t *oldest_vm = NULL;
    error_return_t error = SUCESS;
    while (MsgAlloc_LookAtLuosTask(remaining_msg_number, &oldest_vm) != FAIL)
    {
        // Check the source id
        if (oldest_vm == container->vm)
        {
            uint16_t source = 0;
            MsgAlloc_GetLuosTaskSourceId(remaining_msg_number, &source);
            if (source == id)
            {
                // Source id of this message match, get it and treat it.
                error = MsgAlloc_PullMsg(container->vm, returned_msg);
                // check if the content of this message need to be managed by Luos and do it if it is.
                if ((Luos_MsgHandler(container, *returned_msg)==FAIL) & (error == SUCESS))
                {
                    // This message is for the user, pass it to the user.
                    return SUCESS;
                }
            }
            else
            {
                remaining_msg_number++;
            }
        }
        else
        {
            remaining_msg_number++;
        }
    }
    return FAIL;
}
/******************************************************************************
 * @brief Send large among of data and formating to send into multiple msg
 * @param Container who send
 * @param Message to send
 * @param Pointer to the message data table
 * @param Size of the data to transmit
 * @return error
 ******************************************************************************/
error_return_t Luos_SendData(container_t *container, msg_t *msg, void *bin_data, unsigned short size)
{
    // Compute number of message needed to send this data
    uint16_t msg_number = 1;
    uint16_t sent_size = 0;
    if (size > MAX_DATA_MSG_SIZE)
    {
        msg_number = (size / MAX_DATA_MSG_SIZE);
        msg_number += (msg_number * MAX_DATA_MSG_SIZE < size);
    }

    // Send messages one by one
    for (volatile uint16_t chunk = 0; chunk < msg_number; chunk++)
    {
        // Compute chunk size
        int chunk_size = 0;
        if ((size - sent_size) > MAX_DATA_MSG_SIZE)
            chunk_size = MAX_DATA_MSG_SIZE;
        else
            chunk_size = size - sent_size;

        // Copy data into message
        memcpy(msg->data, (uint8_t *)bin_data + sent_size, chunk_size);
        msg->header.size = size - sent_size;

        // Send message
        if (Luos_SendMsg(container, msg) == FAIL)
        {
            // This message fail stop transmission and return an error
            return SUCESS;
        }

        // Save current state
        sent_size = sent_size + chunk_size;
    }
    return FAIL;
}
/******************************************************************************
 * @brief receive a multi msg data
 * @param Container who receive
 * @param Message chunk received
 * @param pointer to data
 * @return error
 ******************************************************************************/
static error_return_t Luos_ReceiveData(container_t *container, msg_t *msg, void *bin_data)
{
    // Manage buffer session (one per container)
    static uint32_t data_size[MAX_VM_NUMBER] = {0};
    static uint16_t last_msg_size = 0;
    uint16_t id = Luos_GetContainerIndex(container);

    // check good container index
    if(id == 0xFFFF)
    {
        return FAIL;
    }

    // check message integrity
    if ((last_msg_size > 0) && (last_msg_size - MAX_DATA_MSG_SIZE > msg->header.size))
    {
        // we miss a message (a part of the data),
        // reset session and return an error.
        data_size[id] = 0;
        last_msg_size = 0;
        return FAIL;
    }

    // Get chunk size
    unsigned short chunk_size = 0;
    if (msg->header.size > MAX_DATA_MSG_SIZE)
        chunk_size = MAX_DATA_MSG_SIZE;
    else
        chunk_size = msg->header.size;

    // Copy data into buffer
    memcpy(bin_data + data_size[id], msg->data, chunk_size);

    // Save buffer session
    data_size[id] = data_size[id] + chunk_size;
    last_msg_size = msg->header.size;

    // Check end of data
    if (!(msg->header.size > MAX_DATA_MSG_SIZE))
    {
        // Data collection finished, reset buffer session state
        data_size[id] = 0;
        last_msg_size = 0;
        return SUCESS;
    }
    return FAIL;
}
/******************************************************************************
 * @brief Send datas of a streaming channel
 * @param Container who send
 * @param Message to send
 * @param streaming channel pointer
 * @return error
 ******************************************************************************/
error_return_t Luos_SendStreaming(container_t *container, msg_t *msg, streaming_channel_t *stream)
{
    // Compute number of message needed to send available datas on ring buffer
    int msg_number = 1;
    int data_size = Stream_GetAvailableSampleNB(stream);
    const int max_data_msg_size = (MAX_DATA_MSG_SIZE / stream->data_size);
    if (data_size > max_data_msg_size)
    {
        msg_number = (data_size / max_data_msg_size);
        msg_number += ((msg_number * max_data_msg_size) < data_size);
    }

    // Send messages one by one
    for (volatile uint16_t chunk = 0; chunk < msg_number; chunk++)
    {
        // compute chunk size
        uint16_t chunk_size = 0;
        if (data_size > max_data_msg_size)
        {
            chunk_size = max_data_msg_size;
        }
        else
        {
            chunk_size = data_size;
        }

        // Copy data into message
        Stream_GetSample(stream, msg->data, chunk_size);

        // Send message
        if (Luos_SendMsg(container, msg) == FAIL)
        {
            // this message fail stop transmission, retrieve datas and return an error
            stream->sample_ptr = stream->sample_ptr - (chunk_size * stream->data_size);
            if (stream->sample_ptr < stream->ring_buffer)
            {
                stream->sample_ptr = stream->end_ring_buffer - (stream->ring_buffer - stream->sample_ptr);
            }
            return FAIL;
        }

        // check end of data
        if (data_size > max_data_msg_size)
        {
            data_size -= max_data_msg_size;
        }
        else
        {
            data_size = 0;
        }
    }
    return SUCESS;
}
/******************************************************************************
 * @brief Receive a streaming channel datas
 * @param Container who send
 * @param Message to send
 * @param streaming channel pointer
 * @return error
 ******************************************************************************/
error_return_t Luos_ReceiveStreaming(container_t *container, msg_t *msg, streaming_channel_t *stream)
{
    // Get chunk size
    unsigned short chunk_size = 0;
    if (msg->header.size > MAX_DATA_MSG_SIZE)
        chunk_size = MAX_DATA_MSG_SIZE;
    else
        chunk_size = msg->header.size;

    // Copy data into buffer
    Stream_PutSample(stream, msg->data, (chunk_size / stream->data_size));

    // Check end of data
    if ((msg->header.size <= MAX_DATA_MSG_SIZE))
    {
        // Chunk collection finished
        return SUCESS;
    }
    return FAIL;
}
/******************************************************************************
 * @brief store alias name container in flash
 * @param container to store
 * @param alias to store
 * @return error
 ******************************************************************************/
static error_return_t Luos_SaveAlias(container_t *container, uint8_t *alias)
{
    // Get container index
    uint16_t i = (uint16_t)(Luos_GetContainerIndex(container));

    if ((i >= 0)&&(i != 0xFFFF))
    {
        Luos_WriteAlias(i, alias);
        return SUCESS;
    }
    return FAIL;
}
/******************************************************************************
 * @brief write alias name container from flash
 * @param position in the route table
 * @param alias to store
 * @return error
 ******************************************************************************/
static void Luos_WriteAlias(uint16_t local_id, uint8_t *alias)
{
    uint32_t addr = ADDRESS_ALIASES_FLASH + (local_id * (MAX_ALIAS_SIZE + 1));
    LuosHAL_FlashWriteLuosMemoryInfo(addr, 16, (uint8_t *)alias);
}
/******************************************************************************
 * @brief read alias from flash
 * @param position in the route table
 * @param alias to store
 * @return error
 ******************************************************************************/
static error_return_t Luos_ReadAlias(uint16_t local_id, uint8_t *alias)
{
    uint32_t addr = ADDRESS_ALIASES_FLASH + (local_id * (MAX_ALIAS_SIZE + 1));
    LuosHAL_FlashReadLuosMemoryInfo(addr, 16, (uint8_t *)alias);
    // Check name integrity
    if ((((alias[0] < 'A') | (alias[0] > 'Z')) & ((alias[0] < 'a') | (alias[0] > 'z'))) | (alias[0] == '\0'))
    {
        return FAIL;
    }
    else
    {
        return SUCESS;
    }
}
/******************************************************************************
 * @brief set serial baudrate
 * @param baudrate
 * @return None
 ******************************************************************************/
void Luos_SetBaudrate(uint32_t baudrate)
{
    LuosHAL_ComInit(baudrate);
}
/******************************************************************************
 * @brief send network bauderate
 * @param container sending request
 * @param baudrate
 * @return None
 ******************************************************************************/
void Luos_SendBaudrate(container_t *container, uint32_t baudrate)
{
    msg_t msg;
    memcpy(msg.data, &baudrate, sizeof(uint32_t));
    msg.header.target_mode = BROADCAST;
    msg.header.target = BROADCAST_VAL;
    msg.header.cmd = SET_BAUDRATE;
    msg.header.size = sizeof(uint32_t);
    Robus_SendMsg(container->vm, &msg);
}
/******************************************************************************
 * @brief set id of a container trough the network
 * @param container sending request
 * @param target_mode
 * @param target
 * @param newid : The new Id of container(s)
 * @return None
 ******************************************************************************/
error_return_t Luos_SetExternId(container_t *container, target_mode_t target_mode, uint16_t target, uint16_t newid)
{
    msg_t msg;
    msg.header.target = target;
    msg.header.target_mode = target_mode;
    msg.header.cmd = WRITE_NODE_ID;
    msg.header.size = 2;
    msg.data[1] = newid;
    msg.data[0] = (newid << 8);
    if (Robus_SendMsg(container->vm, &msg)==SUCESS)
    {
        return SUCESS;
    }
    return FAIL;
}
/******************************************************************************
 * @brief return the number of messages available
 * @param None
 * @return the number of messages
 ******************************************************************************/
uint16_t Luos_NbrAvailableMsg(void)
{
    return MsgAlloc_LuosTasksNbr();
}
