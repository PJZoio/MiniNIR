/*
 *
 * vim: sta:et:sw=4:ts=4:sts=4
 * https://arduinojson.org/v6/how-to/use-arduinojson-with-arduinomqttclient/
 * https://github.com/arduino-libraries/ArduinoMqttClient/
 *
 * https://github.com/maxgerhardt/platform-raspberrypi
 * https://github.com/earlephilhower/arduino-pico
 * https://components101.com/development-boards/raspberry-pi-pico-w
 *******************************************************************************/

#include <Arduino.h>
//#include <EEPROM.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
//#include <Vrekrer_scpi_arrays_code.h>
//#include <Vrekrer_scpi_parser.h>
#include <SerialCommands.h>

// First we create the constants that we will use throughout our code
#define MOTOR_STEPS 200
#define MICROSTEPS 32
#define TOTAL_STEPS (MOTOR_STEPS * MICROSTEPS) // One full motor turn

#define X_DIR     0       // X axis, direction pin
#define Y_DIR     4       // Y
#define Z_DIR     12       // Z

#define X_STP     1       // X axis, step pin
#define Y_STP     5       // Y
#define Z_STP     13       // Z

#define X_EN        3       // stepper motor enable, low level effective (note put jumper so automatic)
#define Y_EN        10
#define Z_EN        15

#define X_SPEED 2000 // X steps per second
#define Y_SPEED 2000 // Y
#define Z_SPEED 2000 // Z

#define X_ACCEL 1000 // X steps per second per second
#define Y_ACCEL 1000 // Y
#define Z_ACCEL 1000 // Z


#define BAUD_RATE 230400  // the rate at which data is read
//#define BAUD_RATE 115200  // the rate at which data is read

// AccelStepper is the class we use to run all of the motors in a parallel fashion
// Documentation can be found here: http://www.airspayce.com/mikem/arduino/AccelStepper/classAccelStepper.html
#define NUM_MOTORS 3
AccelStepper stepperX(AccelStepper::DRIVER, X_STP, X_DIR);
AccelStepper stepperY(AccelStepper::DRIVER, Y_STP, Y_DIR);
AccelStepper stepperZ(AccelStepper::DRIVER, Z_STP, Z_DIR);

AccelStepper steppers[3] = {stepperX, stepperY, stepperZ};

// Up to 10 steppers can be handled as a group by MultiStepper
MultiStepper multiSteppers;

const unsigned int msgPeriod = 10 * 1000U;
const int enablePins[NUM_MOTORS] = {X_EN, Y_EN, Z_EN};
bool ledState = false;
//bool stepperXon = false;
bool stepperOn[NUM_MOTORS] = {false, false, false};
unsigned long StepperMaxTime = 15 * 1000UL;
unsigned long stepperTimeOut[NUM_MOTORS] = {0UL,0UL,0UL};

char serial_command_buffer_[32];
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\r\n", " ");

//This is the default handler, and gets called when no other command matches.
void cmd_unrecognized(SerialCommands* sender, const char* cmd) {
    sender->GetSerial()->print("Unrecognized command [");
    sender->GetSerial()->print(cmd);
    sender->GetSerial()->println("]");
}
void stepper_start(unsigned int s) {
    if(s < NUM_MOTORS) {
        stepperTimeOut[s]  = millis() + StepperMaxTime;
        digitalWrite(enablePins[s], LOW);
        stepperOn[s] = true;
    }
}
void stepper_stop(unsigned int s) {
    if(s < NUM_MOTORS) {
        digitalWrite(enablePins[s], HIGH);
        steppers[s].stop();
        stepperOn[s] = false;
    }
}
//expects one single parameter
void cmd_move_x(SerialCommands* sender) {
    //Note: Every call to Next moves the pointer to next parameter
    char* disp_str = sender->Next();
    if (disp_str == NULL) {
        sender->GetSerial()->println("ERROR NO_XDISP");
        return;
    }

    signed long move2 = strtol(disp_str, NULL, 0); //atoi(ts_str);
    sender->GetSerial()->print(F("Move motor X: "));
    sender->GetSerial()->print(move2);
    steppers[0].moveTo(move2);
    stepper_start(0);
    //stepperTimeOut[0]  = millis() + StepperMaxTime;
    //digitalWrite(enablePins[0], LOW);
    //stepperOn[0] = true;
    //stepperX.runToPosition(); // Blocking
}

void cmd_move_y(SerialCommands* sender) {
    char* disp_str = sender->Next();
    if (disp_str == NULL) {
        sender->GetSerial()->println("ERROR NO_YDISP");
        return;
    }
    signed long move2 = strtol(disp_str, NULL, 0); //atoi(ts_str);
    sender->GetSerial()->print(F("Move motor Y: "));
    sender->GetSerial()->print(move2);
    //digitalWrite(Y_EN, LOW);
    digitalWrite(enablePins[1], LOW);
    steppers[1].moveTo(move2);
    stepperTimeOut[1]  = millis() + StepperMaxTime;
    stepperOn[1] = true;
}

void cmd_move_z(SerialCommands* sender) {
    char* disp_str = sender->Next();
    if (disp_str == NULL) {
        sender->GetSerial()->println("ERROR NO_ZDISP");
        return;
    }
    signed long move2 = strtol(disp_str, NULL, 0); //atoi(ts_str);
    sender->GetSerial()->print(F("Move motor Z: "));
    sender->GetSerial()->print(move2);
    steppers[2].moveTo(move2);
    stepper_start(2);
}

void cmd_print(SerialCommands* sender) {
    for (int i = 0; i < NUM_MOTORS; i++) {
        steppers[i].stop();
        digitalWrite(enablePins[i], HIGH); // disable Driver
    }
    sender->GetSerial()->print(F("Stop. now= "));
    sender->GetSerial()->println(millis());
    digitalWrite(Z_EN, HIGH);
}
void cmd_jog_plus_1(SerialCommands* sender) {
    steppers[0].move(TOTAL_STEPS); // move (long relative)
    stepper_start(0);
    sender->GetSerial()->print(F("JOG motor X+: "));
}
void cmd_jog_minus_1(SerialCommands* sender) {
    steppers[0].move(-TOTAL_STEPS); // move (long relative)
    stepper_start(0);
    sender->GetSerial()->print(F("JOG motor X-: "));
}
void cmd_jog_plus_2(SerialCommands* sender) {
    steppers[1].move(TOTAL_STEPS); // move (long relative)
    stepper_start(1);
    sender->GetSerial()->print(F("JOG motor Y+: "));
}
void cmd_jog_minus_2(SerialCommands* sender) {
    steppers[1].move(-TOTAL_STEPS); // move (long relative)
    stepper_start(1);
    sender->GetSerial()->print(F("JOG motor Y-: "));
}
void cmd_jog_plus_3(SerialCommands* sender) {
    steppers[2].move(TOTAL_STEPS);
    stepper_start(2);
    sender->GetSerial()->print(F("JOG motor Z+: "));
}
void cmd_jog_minus_3(SerialCommands* sender) {
    steppers[2].move(-TOTAL_STEPS);
    stepper_start(2);
    sender->GetSerial()->print(F("JOG motor Z-: "));
}
//Note: Commands are case sensitive
SerialCommand cmd_move_x_("M1", cmd_move_x); // requires one argument
SerialCommand cmd_move_y_("M2", cmd_move_y);
SerialCommand cmd_move_z_("M3", cmd_move_z);
/// Add one_key commands
SerialCommand cmd_print_("p", cmd_print, true);
SerialCommand cmd_jog_plus_1_("q", cmd_jog_plus_1, true);
SerialCommand cmd_jog_minus_1_("a", cmd_jog_minus_1, true);
SerialCommand cmd_jog_plus_2_("w", cmd_jog_plus_2, true);
SerialCommand cmd_jog_minus_2_("s", cmd_jog_minus_2, true);
SerialCommand cmd_jog_plus_3_("e", cmd_jog_plus_3, true);
SerialCommand cmd_jog_minus_3_("d", cmd_jog_minus_3, true);

void setup() {
    //my_instrument.RegisterCommand(F("*IDN?"), &Identify);
    pinMode(LED_BUILTIN, OUTPUT);
    //pinMode(Z_EN, OUTPUT);
    //digitalWrite(Z_EN, HIGH);

    Serial.begin(BAUD_RATE);
    serial_commands_.SetDefaultHandler(cmd_unrecognized);
    serial_commands_.AddCommand(&cmd_move_x_);
    serial_commands_.AddCommand(&cmd_move_y_);
    serial_commands_.AddCommand(&cmd_move_z_);
    serial_commands_.AddCommand(&cmd_print_);
    serial_commands_.AddCommand(&cmd_jog_plus_1_);
    serial_commands_.AddCommand(&cmd_jog_minus_1_);
    serial_commands_.AddCommand(&cmd_jog_plus_2_);
    serial_commands_.AddCommand(&cmd_jog_minus_2_);
    serial_commands_.AddCommand(&cmd_jog_plus_3_);
    serial_commands_.AddCommand(&cmd_jog_minus_3_);

    for (int i = 0; i < NUM_MOTORS; i++) {
        steppers[i].setMaxSpeed(X_SPEED);
        steppers[i].setAcceleration(X_ACCEL);
        steppers[i].setEnablePin(enablePins[i]);
        steppers[i].enableOutputs();
        digitalWrite(enablePins[i], HIGH); // disable Driver chip
        steppers[i].setCurrentPosition(0L);
    // Then give them to MultiStepper to manage
        multiSteppers.addStepper(steppers[i]);
    }
    delay(200);
    //while (!Serial);
}

void steppers_loop(unsigned long now) {
    //if (now > stepperXtimeOut) {
    if (now > stepperTimeOut[0]) {
        digitalWrite(enablePins[0], HIGH); // disable Driver
        //digitalWrite(X_EN, HIGH); // disable Driver
        if(stepperOn[0]) {
            stepperOn[0] = false;
            Serial.println("NOK");
        }
    }
    //else if (stepperX.distanceToGo() != 0L) {
    else if (steppers[0].distanceToGo() != 0L) {
        steppers[0].run();
    }
    else if(stepperOn[0]) {
        digitalWrite(enablePins[0], HIGH); // disable Driver
        stepperOn[0] = false;
        //stepperXon = false;
        Serial.println("OK");
    }
    /* Stepper 1 (Y)*/
    if (now > stepperTimeOut[1]) {
        digitalWrite(Y_EN, HIGH); // disable Driver
        if(stepperOn[1]) {
            stepperOn[1] = false;
            Serial.println("NOK");
        }
    }
    else if (steppers[1].distanceToGo() != 0L) {
        steppers[1].run();
    }
    else if(stepperOn[1]) {
        stepper_stop(1);
        Serial.println("OK");
    }
    /* Stepper 2 (Z)*/
    if (now > stepperTimeOut[2]) {
        digitalWrite(Z_EN, HIGH); // disable Driver
        if(stepperOn[2]) {
            stepperOn[2] = false;
            Serial.println("NOK");
        }
    }
    else if (steppers[2].distanceToGo() != 0L) {
        steppers[2].run();
    }
    else if(stepperOn[2]) {
        stepper_stop(2);
        Serial.println("OK");
    }
}
void loop() {
    static unsigned long lastMsg = 0;
    serial_commands_.ReadSerial();
    unsigned long now = millis();
    steppers_loop(now);
    if (now - lastMsg > msgPeriod) {
        lastMsg = now;
        Serial.print("Loop: ");
        Serial.print(now / 1000);
        Serial.print("s. Positions: ");
        ledState = not ledState;
        digitalWrite(LED_BUILTIN, ledState);
        for (int i = 0; i < NUM_MOTORS; i++) {
            Serial.print(" ");
            Serial.print(i);
            Serial.print(":");
            Serial.print(steppers[i].currentPosition());
        }
        Serial.println(".");
        /*
        if (ledState)
            movePos = 5000;
    //print_loop(now_ms);
        else
            movePos = 0;
        */
        //stepperZ.moveTo(movePos);
        //stepperZ.runToPosition();

        //        stepperX.run();

    }
}

//  vim: syntax=cpp ts=4 sw=4 sts=4 sr et
