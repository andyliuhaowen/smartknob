#include "serial_task.h"
#include "knob_data.h"

SerialTask::SerialTask(const uint8_t task_core) : Task{"Serial", 4048, 1, task_core}
{
    knob_state_queue_ = xQueueCreate(1, sizeof(KnobState));
    assert(knob_state_queue_ != NULL);
}

QueueHandle_t SerialTask::getKnobStateQueue()
{
    return knob_state_queue_;
}

void SerialTask::run()
{
    KnobState state;
    while (1)
    {
        if (xQueueReceive(knob_state_queue_, &state, portMAX_DELAY) == pdFALSE)
        {
            continue;
        }
        if (millis() - last_msg_ > 50)
        {
            Serial.printf("ANGLE||%d\n", state.current_position);
            last_msg_ = millis();
        }
    }
}