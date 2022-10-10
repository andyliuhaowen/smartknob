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
                Serial.printf("ANGLE||%d\r\n", static_cast<int>(state.current_angle - angle_offset_));
                last_angle_ = state.current_angle;
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
                new_config.num_positions = strtol(ptr, nullptr, 10);
                ptr = strtok(nullptr, " ");
                new_config.position = strtol(ptr, nullptr, 10);
                ptr = strtok(nullptr, " ");
                new_config.position_width_radians = strtof(ptr, nullptr) * PI / 180;
                ptr = strtok(nullptr, " ");
                new_config.detent_strength_unit = strtof(ptr, nullptr);
                ptr = strtok(nullptr, " ");
                new_config.endstop_strength_unit = strtof(ptr, nullptr);
                ptr = strtok(nullptr, " ");
                new_config.snap_point = strtof(ptr, nullptr);
                strncpy(new_config.descriptor, "Custom", 50);
                motor_task_.setConfig(new_config);
                Serial.printf("Read new config, config is: %d %d %f %f %f %f\r\n", new_config.num_positions, new_config.position, new_config.position_width_radians, new_config.detent_strength_unit, new_config.endstop_strength_unit, new_config.snap_point);
            }
            else if (strncmp(ptr, "SET", MAX_STRING_LEN) == 0)
            {
                ptr = strtok(nullptr, "||");
                float set_angle = strtof(ptr, nullptr);
                angle_offset_ = last_angle_ - set_angle;
                Serial.printf("Read set, set target is: %f\r\n", set_angle);
            }
        }
    }
}