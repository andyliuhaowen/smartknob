#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

#include <task.h>

#include "motor_task.h"

class SerialTask : public Task<SerialTask>
{
    friend class Task<SerialTask>; // Allow base Task to invoke protected run()
public:
    SerialTask(const uint8_t task_core, MotorTask &motor_task);

    QueueHandle_t getKnobStateQueue();
    QueueHandle_t getButtonPressQueue();

protected:
    void run();

private:
    QueueHandle_t knob_state_queue_;
    uint32_t last_knob_state_msg_ = 0;
    QueueHandle_t button_press_queue_;

    MotorTask &motor_task_;

    float angle_offset_ = 0;
    float last_angle_ = 0;
};
