#include "motorController.hpp"

#include <FlexCAN_T4.h>
#include <elapsedMillis.h>

#include "canController.hpp"
#include "config.hpp"


Control ctrl = {0, 0, 0, 0, true, 0, 0};
MotorController mtrCtrl;

// struct Messenger{
//     uint32_t id;
//     uint8_t len = 8;
//     uint16_t addr;
//     uint16_t data;

//     bool extended = 0;
//     bool command_message = 1;
//     bool write = true;

//     Messenger(bool write) : write(write) {
//         sync(write);
//     };

//     Messenger(uint16_t addr, int16_t data, bool write) : addr(addr), data(data), write(write) {
//     // Serial.println("init");
//     Message msg;
//     if (!write) data = 0;
        
//     addr = addr;
//     data = data;
//     write = write;
//     msg.id = 0x0C1;
//     msg.len = len;
//     msg.flags.extended = 0;
//     msg.buf[0] = addr & 0xFF;
//     msg.buf[1] = addr >> 8;
//     msg.buf[2] = write;

//     msg.buf[4] = data & 0xFF;
//     msg.buf[5] = data >> 8;
//     can.write(msg);
//     delay(20);
//     };


//     Messenger(uint16_t torque, uint16_t speed, bool direction, uint8_t enable, uint16_t torque_limit) {

//     Message msg;
//     msg.id = 0x0C0;
//     msg.len = len;
//     msg.flags.extended = 0;
//     msg.buf[0] = torque & 0xFF;
//     msg.buf[1] = torque >> 8;
//     msg.buf[2] = speed & 0xFF;
//     msg.buf[3] = speed >> 8;
//     msg.buf[4] = direction;
//     msg.buf[5] = enable;
//     can.write(msg);
//     //delay(50);
//     };

//     void sync(bool write) {

//         for (int i = 0; i < 16; ++i) {
//             Messenger(PARAMS[i][0], PARAMS[i][1], write);
//         }
//     }
// };



void init() {
    Serial.println("Initalizing Motor Controller");

    CANController::requestMessageTemp(motorConfig::MSGID_RX_PARAM, [](const Message& msg) {
        if ((msg.buf[0] | (msg.buf[1] << 8)) != mtrCtrl.expectedAddr) {
            return;
        }
        mtrCtrl.paramOk = (msg.buf[2] == 1);
        mtrCtrl.paramRecieved = true;
    });

    CANController::requestMessages(motorConfig::MSGID_RT_STATES, onStateMessage);
    CANController::requestMessages(motorConfig::MSGID_RT_FAULTS, onFaultMessage);
    // CANController::requestMessages(motorConfig::MSGID_RT_ANALOG, onAnalogMessage);

    for (const auto& param : motorConfig::PARAMS) {
        Serial.printf("param %u write value %u\n", param[0], param[1]);

        MessageData data =
            CAN_DATA(param[0] & 0xFF, param[0] >> 8, 1, 0, param[1] & 0xFF, param[1] >> 8, 0, 0);

        mtrCtrl.paramRecieved = false;
        mtrCtrl.paramOk = false;
        mtrCtrl.expectedAddr = param[0];

        CANController::write(motorConfig::MSGID_TX_PARAM, &data);

        elapsedMillis timeout{};
        while (!mtrCtrl.paramRecieved && timeout < motorConfig::PARAM_RX_TIMEOUT) {
            CANController::poll();
        }

        if (!mtrCtrl.paramRecieved) {
            Serial.printf("param %u write timeout\n", param[0]);
        } else if (!mtrCtrl.paramOk) {
            Serial.printf("param %u write failed\n", param[0]);
        } else {
            Serial.printf("param %u write success\n", param[0]);
        }
    }

    mtrCtrl.elapsed = 0;
    Serial.println("Initalized Motor Controller");
}

void poll() {
   
    if (mtrCtrl.elapsed <= motorConfig::COMMAND_MSG_RATE) {
        return;
    }
    mtrCtrl.elapsed = 0;

    switch (mtrCtrl.lockout) {
    case true:
        sendCommandMessage(0, 0, 1, 1, 0);
        return;
    
    case false:
        break;
    default:
        break;
    }

    bool throttle_active = ctrl.voltage > motorConfig::THROTTLE_THRESHOLD;
    
    switch (throttle_active) {
    case true:
        throttleCmd();
        break;
    
    default:
        sendCommandMessage(0, 0, 1, 1, 0);
        break;
    }

}

void onStateMessage(const Message& msg) {

    switch (msg.id) {
    case 0xA3:
      ctrl.voltage = msg.buf[0] | ((msg.buf[1] & 0x03) << 8);
      Serial.println(ctrl.voltage);
      break;

    case 0xAA:
      ctrl.vsmState = msg.buf[0];
      break;
      
    case 0x0AB:
      ctrl.runFault = msg.buf[4] | ((uint32_t)msg.buf[5] << 8) |
        ((uint32_t)msg.buf[6] << 16) | ((uint32_t)msg.buf[7] << 24);
      break;
    case 0x0AC:
      ctrl.current_torque = msg.buf[0];
      break;
    // case 0x0A5:
    //   Serial.print(msg.buf[0] | (msg.buf[1] << 8));
    //   Serial.print(" ");
    //   Serial.print(msg.buf[2] | (msg.buf[3] << 8));
    //   ctrl.veh_speed = (msg.buf[2] | (msg.buf[3] << 8));
    //   Serial.print(" ");
    //   Serial.println(msg.buf[6] | (msg.buf[7] << 8));
    //   break;
    // case 0x0A7:
    //   Serial.println(msg.buf[0]);
    //   break;
    case 0x0C2: {
      uint16_t addr = msg.buf[0] | (msg.buf[1] << 8);
      uint32_t value = (uint32_t)msg.buf[4] | ((uint32_t)msg.buf[5] << 8) |
                      ((uint32_t)msg.buf[6] << 16) | ((uint32_t)msg.buf[7] << 24);
      Serial.printf("Param 0x%04X = %lu\n", addr, (unsigned long)value);
    } break;

  }
}

void onFaultMessage(const Message& msg) {
    uint32_t run = msg.buf[4] | (static_cast<uint32_t>(msg.buf[5]) << 8) |
                   (static_cast<uint32_t>(msg.buf[6]) << 16) |
                   (static_cast<uint32_t>(msg.buf[7]) << 24);
    //Serial.printf("Entered Lockout");
    if (run) {
      
      if (run & 0x00000400) Serial.println("Under Voltage Error");
      if (run & 0x00000100) {
        Serial.println("Hardware Gate Fault");
        ctrl.vsmState = 7;
        ctrl.lockout = true;
        
      }
      if (run & 0x40000000) Serial.println("Resolver Not Connected");
      if (run & 0x00000002) Serial.println("Over-Current Fault");
      if (run & 0x00000800) Serial.println("CAN Command Message thrLost");
      if (run & 0x00000020) Serial.println("Accelerator Input Open");
    }
}


void throttleCmd() {

    static const uint8_t iqCurrent = 200;
    static const uint8_t idCurrent = 200;
    int16_t T = motorConfig::TEST_TORQUE;
    int16_t N = ctrl.veh_speed;
    const uint16_t cons =  9550;
    int32_t P = (T * N) / cons;
    const uint8_t voltage = 212;
    uint32_t current = P / voltage;
    current = current * 10;
        
    if (current >= iqCurrent || current >= idCurrent) {
        ctrl.torque = 90;
    }
    Serial.println("Throttle Active");

    sendCommandMessage(motorConfig::TEST_TORQUE, 0, 1, 2, (1.5*motorConfig::TEST_TORQUE));
}

void sendCommandMessage(uint8_t torque, uint8_t speed, uint8_t direction, uint8_t flags, uint8_t torqueLimit) {
    MessageData data = CAN_DATA(torque, 0, speed, 0, direction, flags, torqueLimit, 0);
    CANController::write(motorConfig::MSGID_TX_COMMAND, &data);
}
