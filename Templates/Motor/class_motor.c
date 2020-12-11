#include "template_motor.h"
#include "class_motor.h"
#include "math.h"
/******************************************************************************
 * @brief function converting Luos messages innto data and reverse.
 * @param container the target container
 * @param msg the received message
 * @param motor the data struct to update
 * @return None
 ******************************************************************************/
static void MotorConfig_Handler(container_t *container, msg_t *msg, class_motor_t *motor_class)
{
    if (msg->header.cmd == PARAMETERS)
    {
        // check the message size
        // fill the message infos
        memcpy((void *)&motor_class->mode, msg->data, sizeof(motor_mode_t));
        return;
    }
}
/******************************************************************************
 * @brief Msg Handler call backed by Luos when a msg receive for this container
 * @param Container destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void TemplateMotor_MsgHandler(container_t *container, msg_t *msg)
{
    template_motor_t *motor_template = (template_motor_t *)container->template_context;
    ClassMotor_Handler(container, msg, &motor_template->class);
    MotorConfig_Handler(container, msg, &motor_template->class);
    if (motor_template->self != 0)
    {
        motor_template->self(container, msg);
    }
}
/******************************************************************************
 * @brief Container creation following the template
 * @param cont_cb is an optional user callback called on every massage for this container
 * @param motor_struct template object pointer
 * @param alias for the container string (15 caracters max).
 * @param revision FW for the container (tab[MajorVersion,MinorVersion,Patch])
 * @return None
 ******************************************************************************/
container_t *TemplateMotor_CreateContainer(CONT_CB cont_cb, template_motor_t *motor_template, const char *alias, revision_t revision)
{
    motor_template->self        = cont_cb;
    container_t *container      = Luos_CreateContainer(TemplateMotor_MsgHandler, MOTOR_TYPE, alias, revision);
    container->template_context = (void *)motor_template;
    return container;
}
/******************************************************************************
 * @brief function converting Luos messages innto data and reverse.
 * @param container the target container
 * @param msg the received message
 * @param motor_struct the data struct to update
 * @return None
 ******************************************************************************/
void ClassMotor_Handler(container_t *container, msg_t *msg, class_motor_t *class_motor)
{
    switch (msg->header.cmd)
    {
        case ASK_PUB_CMD:
        {
            msg_t pub_msg;
            pub_msg.header.target_mode = msg->header.target_mode;
            pub_msg.header.target      = msg->header.source;
            if (class_motor->mode.current)
            {
                ElectricOD_CurrentToMsg((current_t *)&class_motor->current, &pub_msg);
                Luos_SendMsg(container, &pub_msg);
            }
            if (class_motor->mode.temperature)
            {
                TemperatureOD_TemperatureToMsg((current_t *)&class_motor->temperature, &pub_msg);
                Luos_SendMsg(container, &pub_msg);
            }
        }
        break;
        case RATIO:
        {
            // get the motor power ratio
            RatioOD_RatioFromMsg(&class_motor->power, msg);
            if (class_motor->power > 100.0)
                class_motor->power = 100.0;
            if (class_motor->power < -100.0)
                class_motor->power = -100.0;
        }
        break;
        case RATIO_LIMIT:
        {
            // set the motor power ratio limit
            RatioOD_RatioFromMsg(&class_motor->limit_ratio, msg);
            class_motor->limit_ratio = fabs(class_motor->limit_ratio);
            if (class_motor->limit_ratio > 100.0)
                class_motor->limit_ratio = 100.0;
        }
        break;
        case CURRENT_LIMIT:
        {
            // set the motor current limit
            ElectricOD_CurrentFromMsg((current_t *)&class_motor->limit_current, msg);
            class_motor->limit_current = fabs(class_motor->limit_current);
        }
        break;
        case TEMPERATURE_LIMIT:
        {
            // set the motor power ratio limit
            TemperatureOD_TemperatureFromMsg(&class_motor->limit_temperature, msg);
            class_motor->limit_temperature = fabs(class_motor->limit_temperature);
        }
        break;
    }
}
