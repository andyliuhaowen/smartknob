#include <string>

#include "serial_task.h"
#include "knob_data.h"

constexpr int MAX_STRING_LEN = 100;

SerialTask::SerialTask(const uint8_t task_core, MotorTask &motor_task) : Task("Serial", 4048, 1, task_core),
                                                                         motor_task_(motor_task) {
    knob_state_queue_ = xQueueCreate(1, sizeof(KnobState));
    assert(knob_state_queue_ != nullptr);
    button_press_queue_ = xQueueCreate(1, sizeof(char));
    assert(button_press_queue_ != nullptr);
}

QueueHandle_t SerialTask::getKnobStateQueue() {
    return knob_state_queue_;
}

QueueHandle_t SerialTask::getButtonPressQueue() {
    return button_press_queue_;
}

[[noreturn]] void SerialTask::run() {
    KnobState state{};
    char press;
    while (true) {
        // Write angle
        if (xQueueReceive(knob_state_queue_, &state, 1) == pdTRUE) {
            if (millis() - last_knob_state_msg_ > 100) {
                Serial.printf("ANGLE||%d\r\n", static_cast<int>(state.current_angle - angle_offset_));
                last_angle_ = state.current_angle;
                last_knob_state_msg_ = millis();
            }
        }
        // Write button
        if (xQueueReceive(button_press_queue_, &press, 1) == pdTRUE) {
            Serial.println("BUTTON");
        }
        // Read config
        if (Serial.available() > 0) {
            String receivedString = Serial.readStringUntil('\0');
            char charArr[MAX_STRING_LEN];
            receivedString.toCharArray(charArr, MAX_STRING_LEN);
            std::string full_msg(charArr);
            std::string::size_type start = 0;
            std::string::size_type end = full_msg.find("||");
            std::string msg_type = full_msg.substr(start, end - start);
            std::string msg_value = full_msg.substr(end + 2);
            Serial.printf("Received message %s\n", msg_value.data());

            // Expects message formats of:
            // CONFIG||<num_positions> <position> <position_width_degrees>
            // <detent_strength_unit> <endstop_strength_unit> <snap_point>
            // <description>
            if (msg_type == "CONFIG") {
                KnobConfig new_config{};
                std::string token_str;
                token_str.reserve(50);

                start = 0;
                end = msg_value.find(' ', start);
                token_str = msg_value.substr(start, end - start);
                new_config.num_positions = std::stoi(token_str, nullptr);

                start = end + 1;
                end = msg_value.find(' ', start);
                token_str = msg_value.substr(start, end - start);
                new_config.position = std::stoi(token_str, nullptr);

                start = end + 1;
                end = msg_value.find(' ', start);
                token_str = msg_value.substr(start, end - start);
                new_config.position_width_radians =
                        std::stof(token_str, nullptr) *
                        static_cast<float>(PI) / 180;

                start = end + 1;
                end = msg_value.find(' ', start);
                token_str = msg_value.substr(start, end - start);
                new_config.detent_strength_unit = std::stof(token_str,
                                                            nullptr);

                start = end + 1;
                end = msg_value.find(' ', start);
                token_str = msg_value.substr(start, end - start);
                new_config.endstop_strength_unit = std::stof(token_str,
                                                             nullptr);

                start = end + 1;
                end = msg_value.find(' ', start);
                token_str = msg_value.substr(start, end - start);
                new_config.snap_point = std::stof(token_str, nullptr);

                strncpy(new_config.descriptor, "Custom", 50);
                motor_task_.setConfig(new_config);
                Serial.printf("Read new config, config is: %d %d %f %f %f %f\r\n", new_config.num_positions,
                              new_config.position, new_config.position_width_radians, new_config.detent_strength_unit,
                              new_config.endstop_strength_unit, new_config.snap_point);
            } else if (msg_type == "SET") {
                float set_angle = std::stof(std::string(msg_value), nullptr);
                angle_offset_ = last_angle_ - set_angle;
                Serial.printf("Read set, set target is: %f\r\n", set_angle);
                Serial.printf("New offset value is %f\n", angle_offset_);
            }
        }
    }
}