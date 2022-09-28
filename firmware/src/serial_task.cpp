#include "serial_task.h"
#include "knob_data.h"

SerialTask::SerialTask(const uint8_t task_core) : Task{"Serial", 4048, 1, task_core}
{
    knob_state_queue_ = xQueueCreate(1, sizeof(KnobState));
    assert(knob_state_queue_ != NULL);
    button_press_queue_ = xQueueCreate(1, sizeof(char));
    assert(button_press_queue_ != NULL);
}

QueueHandle_t SerialTask::getKnobStateQueue()
{
    return knob_state_queue_;
}

QueueHandle_t SerialTask::getButtonPressQueue()
{
    return button_press_queue_;
}

void SerialTask::run()
{
    KnobState state;
    char press;
    while (1)
    {
        if (xQueueReceive(knob_state_queue_, &state, portMAX_DELAY) != pdFALSE)
        {
            if (millis() - last_knob_state_msg_ > 50)
            {
                Serial.printf("ANGLE||%d\r\n", state.current_position);
                last_knob_state_msg_ = millis();
            }
        }
        if (xQueueReceive(button_press_queue_, &press, portMAX_DELAY) != pdFALSE)
        {
            Serial.println("BUTTON");
        }
    }
}