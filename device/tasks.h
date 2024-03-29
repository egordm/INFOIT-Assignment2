//
// Created by egordm on 19-3-2018.
//

#ifndef B3ITASSIGNMENT2_TASKS_H
#define B3ITASSIGNMENT2_TASKS_H


#include <PubSubClient.h>
#include <Adafruit_BME280.h>
#include <SSD1306Wire.h>
#include <OLEDDisplayUi.h>
#include "../lib/scheduler/scheduler.h"
#include "macros.h"
#include "config.h"
#include "../lib/Servo/src/Servo.h"

class MQTTRoutine : public internals::RoutineTask {
public:
    MQTTRoutine(unsigned long interval, PubSubClient &client) : RoutineTask(interval), client(client) {}

protected:
    PubSubClient &client;
};

class MQTTReconnectCycle : public MQTTRoutine {
public:
    explicit MQTTReconnectCycle(PubSubClient &client) : MQTTRoutine(MQTT_CONNECT_POLL_INTERVAL, client) {}

private:
    bool execute() override;

    bool should_run(unsigned long time) override;
};

class SensorRoutine : public MQTTRoutine {
public:
    explicit SensorRoutine(PubSubClient &client) : MQTTRoutine(SENSOR_POLL_INTERVAL, client) {
        if (!bme.begin()) SHOUT("BME280 sensor is not found!");
        else started = true;
    }

    bool execute() override;

private:
    Adafruit_BME280 bme;
    bool started = false;
};

class WaterPlantRoutine : public internals::RoutineTask {
private:
    Servo servo;
public:
    WaterPlantRoutine() : RoutineTask(WATER_PLANT_POLL_INTERVAL), servo() {
        sensor_data.water_ticks = WATER_PLANT_DURATION;
        servo.attach(SERVO_PIN);
    };

    bool execute() override;

    bool should_run(unsigned long time) override;

    void start_watering() {if(sensor_data.water_ticks >= WATER_PLANT_DURATION) sensor_data.water_ticks = 0;}
};

class ButtonToggleRoutine : public MQTTRoutine {
private:
    bool prev_btn_val;
    uint8_t pin;
    bool &val;
    const char *topic;
public:
    explicit ButtonToggleRoutine(PubSubClient &client, bool &val, const uint8_t pin, const char *topic)
            : MQTTRoutine(BUTTON_POLL_INTERVAL, client), val(val), prev_btn_val(val), pin(pin), topic(topic) {};

    virtual bool execute() override {
        auto reading = digitalRead(pin) == LOW;
        if(reading == prev_btn_val) return false;
        prev_btn_val = reading;
        if(reading) {
            val = !val;
            SHOUT(String("Toggling automatic mode manually to ") + config.automatic_mode);
            if(topic != nullptr) {
                const char *payload = val ? "1" : "0";
                client.publish(topic, payload, true);
            }
        }

        return false;
    }
};

class AutomaticMaintenanceRoutine : public internals::RoutineTask {
private:
    WaterPlantRoutine *plant_routine;
public:
    explicit AutomaticMaintenanceRoutine(WaterPlantRoutine *plant_routine)
            : RoutineTask(MAINTENANCE_POLL_INTERVAL), plant_routine(plant_routine) {}

    bool execute() override;

    bool should_run(unsigned long time) override;
};


#endif //B3ITASSIGNMENT2_TASKS_H
