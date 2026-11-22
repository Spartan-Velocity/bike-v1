#include "motorController.hpp"

#include <FlexCAN_T4.h>
#include <elapsedMillis.h>

#include "canController.hpp"
#include "config.hpp"

static void motor_init() {
    Serial.println("Initalizing Motor Controller");

    CANController::requestMessageTemp(motorConfig::MSGID_RX_PARAM, [](const Message& msg) {
        if ((msg.buf[0] | (msg.buf[1] << 8)) != motor.expectedAddr) return;
        motor.paramOk = (msg.buf[2] == 1);
        motor.paramRecieved = true;
    });

    CANController::requestMessages(motorConfig::MSGID_RT_STATES, motor.on_state);
    CANController::requestMessages(motorConfig::MSGID_RT_FAULTS, motor.on_fault);

    for (const auto& param : motorConfig::PARAMS) {
        MessageData data =
            CAN_DATA(param[0] & 0xFF, param[0] >> 8, 1, 0, param[1] & 0xFF, param[1] >> 8, 0, 0);

        motor.paramRecieved = false;
        motor.paramOk = false;
        motor.expectedAddr = param[0];
        CANController::write(motorConfig::MSGID_TX_PARAM, &data);

        elapsedMillis timeout{};
        while (!motor.paramRecieved && timeout < motorConfig::PARAM_RX_TIMEOUT) CANController::poll();

        if (!motor.paramRecieved)   Serial.printf("param %u timeout\n", param[0]);
        else if (!motor.paramOk)    Serial.printf("param %u failed\n", param[0]);
        else                        Serial.printf("param %u ok\n", param[0]);

    }
    motor.elapsed = 0;
    Serial.println("Initalized Motor Controller");
}

static void motor_send_command(uint8_t torque, uint8_t direction, uint8_t flags, uint8_t torqueLimit) {
    
    MessageData data = CAN_DATA(torque, 0, 0, 0, direction, flags, torqueLimit, 0);
    CANController::write(motorConfig::MSGID_TX_COMMAND, &data);
}

static void motor_throttle_cmd() {
    int32_t P = (motorConfig::TEST_TORQUE * motor.veh_speed) / 9550;
    uint32_t current = (P / 212) * 10;
    Serial.println(current);
    //Serial.println(motor.thrvoltage);
    if (current >= motor.iqCurrent || current >= motor.idCurrent) motor.torque = 90;
    
    motor_send_command(motorConfig::TEST_TORQUE, 1, 2, 1.5 * motorConfig::TEST_TORQUE);
}

static void motor_poll() {
    // if (motor.elapsed <= motorConfig::COMMAND_MSG_RATE) return;
    // motor.elapsed = 0;
   // if (motor.lockout) motor_send_command(0, 0, 1, 1, 0);
    // if (motor.thrvoltage > motorConfig::THROTTLE_THRESHOLD) motor_throttle_cmd();
    // else motor_send_command(0, 0, 1, 3, 0);
    switch (motor.vsmState) {
        case 4: {
            motor_send_command(0, 1, 3, 0);
            motor_throttle_cmd();
        } break;

        case 6:
            Serial.printf("Drive Enabled");
            //drive();
        break;

        case 7: {
            MessageData data = CAN_DATA(0x14, 0x00, 1, 0, 0, 0, 0, 0);
            CANController::write(motorConfig::MSGID_TX_PARAM, &data);
        } break; 
    }
}


static void motor_on_state(const Message& msg) {
    switch (msg.id) {
        case 0xA3: motor.thrvoltage = msg.buf[0] | ((msg.buf[1] & 0x03) << 8); break;
        case 0xAA: motor.vsmState = msg.buf[0]; break;
        case 0xAC: motor.current_torque = msg.buf[0]; break;
        case 0xAB:
            motor.runFault = msg.buf[4] | ((uint32_t)msg.buf[5] << 8)|((uint32_t)msg.buf[6] << 16) | ((uint32_t)msg.buf[7] << 24);
        break;

        case 0xC2: {
            uint16_t addr = msg.buf[0] | (msg.buf[1] << 8);
            uint32_t val  = msg.buf[4] | ((uint32_t)msg.buf[5] << 8) | ((uint32_t)msg.buf[6] << 16) | ((uint32_t)msg.buf[7] << 24);
            Serial.printf("Param 0x%04X = %lu\n", addr, (unsigned long)val);
        } break;
    }
}

static void motor_on_fault(const Message& msg) {
    
    if (!motor.runFault) return;
    if (motor.runFault & 0x00000400) Serial.println("Under Voltage Error");
    if (motor.runFault & 0x00000100) { 
        Serial.println("Hardware Gate Fault"); 
        motor.vsmState = 7; 
        motor.lockout = true; 
        
    }

    if (motor.runFault & 0x40000000) Serial.println("Resolver Not Connected");
    if (motor.runFault & 0x00000002) Serial.println("Over-Current Fault");
    if (motor.runFault & 0x00000800) Serial.println("CAN Command Message Lost");
    if (motor.runFault & 0x00000020) Serial.println("Accelerator Input Open");
}

// The one motor instance: state defaults come from the struct, ops wired here.
MotorDriver motor = {
    .init         = motor_init,
    .poll         = motor_poll,
    .on_state     = motor_on_state,
    .on_fault     = motor_on_fault,
    .send_command = motor_send_command,
    .throttle_cmd = motor_throttle_cmd,
};
