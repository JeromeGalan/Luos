#include "template_state.h"
#include "class_state.h"

/******************************************************************************
 * @brief Msg Handler call backed by Luos when a msg receive for this container
 * @param Container destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void TemplateState_MsgHandler(container_t *container, msg_t *msg)
{
    template_state_t *state_template = (template_state_t *)container->template_context;
    ClassState_Handler(container, msg, &state_template->class);
    if (state_template->self != 0)
    {
        state_template->self(container, msg);
    }
}
/******************************************************************************
 * @brief Container creation following the template
 * @param cont_cb is an optional user callback called on every massage for this container
 * @param state_struct template object pointer
 * @param alias for the container string (15 caracters max).
 * @param revision FW for the container (tab[MajorVersion,MinorVersion,Patch])
 * @return None
 ******************************************************************************/
container_t *TemplateState_CreateContainer(CONT_CB cont_cb, template_state_t *state_template, const char *alias, revision_t revision)
{
    state_template->self        = cont_cb;
    container_t *container      = Luos_CreateContainer(TemplateState_MsgHandler, STATE_TYPE, alias, revision);
    container->template_context = (void *)state_template;
    container->access           = state_template->class.access;
    return container;
}
/******************************************************************************
 * @brief function converting Luos messages innto data and reverse.
 * @param container the target container
 * @param msg the received message
 * @param state_struct the data struct to update
 * @return None
 ******************************************************************************/
void ClassState_Handler(container_t *container, msg_t *msg, class_state_t *state_class)
{
    if ((msg->header.cmd == ASK_PUB_CMD) && ((state_class->access == READ_WRITE_ACCESS) || (state_class->access == READ_ONLY_ACCESS)))
    {
        // fill the message infos
        msg_t pub_msg;
        pub_msg.header.cmd         = IO_STATE;
        pub_msg.header.target_mode = ID;
        pub_msg.header.target      = msg->header.source;
        pub_msg.header.size        = sizeof(char);
        pub_msg.data[0]            = state_class->state;
        Luos_SendMsg(container, &pub_msg);
    }
    if ((msg->header.cmd == IO_STATE) && ((state_class->access == READ_WRITE_ACCESS) || (state_class->access == WRITE_ONLY_ACCESS)))
    {
        state_class->state = msg->data[0];
    }
}
