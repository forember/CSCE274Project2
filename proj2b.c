#include <stdint.h>
#include "timer.h"
#include "cmod.h"
#include "oi.h"

#include "sensing.h"
#include "iroblife.h"
#include "driving.h"

// Driving constants
#define SPEED           (500)
#define TURN_ANGLE      (30)

uint8_t irPrevious = 0;

uint8_t cannotRotateOrAdvance(void) {
    return getSensorUint8(SenBumpDrop) & MASK_WHEEL_DROP;
}

uint8_t cannotAdvance(void) {
    return getSensorUint8(SenBumpDrop) || !(getSensorUint16(SenCliffLSig1) &&
            getSensorUint16(SenCliffFLSig1) && getSensorUint16(SenCliffFRSig1)
            && getSensorUint16(SenCliffRSig1));
}

// Called by irobPeriodic
void iroblifePeriodic(void) {
    updateSensors();
    waitForSensors();
    uint8_t ir = getSensorUint8(SenIRChar);
    if (cannotRotateOrAdvance()) {
        driveStop();
    } else {
        switch (ir) {
            case IR_FORWARD:
                if (cannotAdvance()) {
                    driveStop();
                } else {
                    drive(SPEED, RadStraight);
                }
                break;
            case IR_LEFT:
                drive(SPEED, RadCCW);
                break;
            case IR_RIGHT:
                drive(SPEED, RadCW);
                break;
            default:
                switch (irPrevious) {
                    case IR_LEFT:
                        driveAngleOp(SPEED, RadCCW, TURN_ANGLE);
                        break;
                    case IR_RIGHT:
                        driveAngleOp(SPEED, RadCW, TURN_ANGLE);
                        break;
                    default:
                        driveStop();
                }
                break;
        }
    }
    irPrevious = ir;
}


int main(void) {
    // Submit to irobPeriodic
    setIrobPeriodicImpl(&iroblifePeriodic);

    // Initialize the Create
    irobInit();

    // Infinite operation loop
    for(;;) {
        // Periodic execution
        irobPeriodic();

        // Delay
        delayMs(1);
    }
}

