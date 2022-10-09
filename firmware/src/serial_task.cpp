#include "serial_task.h"
#include "knob_data.h"

constexpr int MAX_STRING_LEN = 100;

SerialTask::SerialTask(const uint8_t task_core, MotorTask &motor_task) : Task("Serial", 4048, 1, task_core), motor_task_(motor_task)
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
        // Write angle
        if (xQueueReceive(knob_state_queue_, &state, 1) == pdTRUE)
        {
            if (millis() - last_knob_state_msg_ > 100)
            {
                Serial.printf("ANGLE||%f\r\n", state.sub_position_unit + state.current_position);
                last_knob_state_msg_ = millis();
            }
        }
        // Write button
        if (xQueueReceive(button_press_queue_, &press, 1) == pdTRUE)
        {
            Serial.println("BUTTON");
        }
        // Read config
        if (Serial.available() > 0)
        {
            String receivedString = Serial.readStringUntil('\0');
            char charArr[MAX_STRING_LEN];
            receivedString.toCharArray(charArr, MAX_STRING_LEN);
            char *ptr = strtok(charArr, "||");
            // Expects message formats of:
            // CONFIG||<num_positions> <position> <position_width_degrees>
            // <detent_strength_unit> <endstop_strength_unit> <snap_point>
            // <description>
            if (strncmp(ptr, "CONFIG", MAX_STRING_LEN) == 0)
            {
                ptr = strtok(nullptr, " ");
                KnobConfig new_config;
                new_config.num_positions = strtod(ptr, nullptr);
                ptr = strtok(nullptr, " ");
                new_config.position = strtod(ptr, nullptr);
                ptr = strtok(nullptr, " ");
                new_config.position_width_radians = strtof(ptr, nullptr) * PI / 180;
                ptr = strtok(nullptr, " ");
                new_config.detent_strength_unit = strtof(ptr, nullptr);
                ptr = strtok(nullptr, " ");
                new_config.endstop_strength_unit = strtof(ptr, nullptr);
                ptr = strtok(nullptr, " ");
                new_config.snap_point = strtof(ptr, nullptr);
                ptr = strtok(nullptr, " ");
                strncpy(new_config.descriptor, ptr, 50);
                motor_task_.setConfig(new_config);
            }
        }
    }
}