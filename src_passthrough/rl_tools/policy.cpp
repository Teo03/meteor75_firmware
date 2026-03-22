/**
 * Motor passthrough for Meteor75 — receives motor commands from RC channels.
 *
 * Instead of running a neural network on the flight controller,
 * this reads motor commands sent from a PC via CRSF/ELRS.
 * The RL policy runs on the PC and sends motor values on channels 6-9.
 */

#include <cmath>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
extern "C" {
    #include "rx/rx.h"
    #include "flight/mixer.h"
    #include "sensors/gyro.h"
    #include "drivers/time.h"
    #include "build/debug.h"
}
#pragma GCC diagnostic pop

using T = float;

/**
 * Convert RC channel value (PWM 988-2012) to [-1, 1] range.
 * CRSF channels 172-1811 map to PWM 988-2012 in Betaflight.
 */
static T from_channel(T value) {
    return (value - 1500.0f) / 512.0f;
}

/**
 * Motor passthrough control function.
 * Called every loop iteration when NN_CONTROL mode is active.
 *
 * Reads motor commands from RC channels 6-9 and applies directly to motors.
 * Channels 0-5 remain for normal stick input + arming + mode selection.
 */
extern "C" void rl_tools_control(bool armed) {
    // Log channel values to blackbox for debugging
    DEBUG_SET(DEBUG_RL_TOOLS, 0, (int16_t)(rcData[6]));   // Motor 1 RC value
    DEBUG_SET(DEBUG_RL_TOOLS, 1, (int16_t)(rcData[7]));   // Motor 2 RC value
    DEBUG_SET(DEBUG_RL_TOOLS, 2, (int16_t)(rcData[8]));   // Motor 3 RC value
    DEBUG_SET(DEBUG_RL_TOOLS, 3, (int16_t)(rcData[9]));   // Motor 4 RC value
    DEBUG_SET(DEBUG_RL_TOOLS, 4, (int16_t)(armed));        // Armed status

    // Log gyro data for telemetry (PC can use this)
    constexpr float GYRO_FACTOR = 3.14159265f / 180.0f;
    DEBUG_SET(DEBUG_RL_TOOLS, 5, (int16_t)(gyro.gyroADCf[0] * 10));  // Roll rate (0.1 deg/s)
    DEBUG_SET(DEBUG_RL_TOOLS, 6, (int16_t)(gyro.gyroADCf[1] * 10));  // Pitch rate
    DEBUG_SET(DEBUG_RL_TOOLS, 7, (int16_t)(gyro.gyroADCf[2] * 10));  // Yaw rate

    if (!armed) {
        return;
    }

    // Read motor commands from RC channels 6-9
    // Each channel: PWM [988, 2012] → [-1, 1] → [0, 1] → [motorLow, motorHigh]
    for (int i = 0; i < 4; i++) {
        T cmd = from_channel(rcData[6 + i]);  // [-1, 1]

        // Clamp to valid range
        if (cmd > 1.0f) cmd = 1.0f;
        if (cmd < -1.0f) cmd = -1.0f;

        // Map to motor output range
        T normalized = (cmd + 1.0f) * 0.5f;  // [0, 1]
        motor[i] = getMotorOutputLow() + normalized * (getMotorOutputHigh() - getMotorOutputLow());
    }
}
