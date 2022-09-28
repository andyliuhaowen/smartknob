#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

#include <task.h>

class SerialTask : public Task<SerialTask>
{
    friend class Task<SerialTask>; // Allow base Task to invoke protected run()
public:
    SerialTask(const uint8_t task_core);

    QueueHandle_t getKnobStateQueue();
    QueueHandle_t getButtonPressQueue();

protected:
    void run();

private:
    QueueHandle_t knob_state_queue_;
    uint32_t last_knob_state_msg_ = 0;
    QueueHandle_t button_press_queue_;
};
