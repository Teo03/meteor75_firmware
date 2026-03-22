/**
 * Motor passthrough policy for Meteor75.
 *
 * When NN_CONTROL mode is active:
 *   - Reads motor commands from RC channels 4-7
 *   - Maps [-1, 1] range to motor output [low, high]
 *   - Bypasses Betaflight PID entirely
 *
 * This allows a PC-side RL policy to send direct motor commands
 * via CRSF through the ELRS TX module.
 *
 * RC Channel mapping:
 *   Ch 0-3: Normal sticks (roll, pitch, throttle, yaw) — used for arming
 *   Ch 4:   AUX1 — arm switch
 *   Ch 5:   AUX2 — NN_CONTROL mode switch
 *   Ch 6:   Motor 1 (front-right) command
 *   Ch 7:   Motor 2 (back-right) command
 *   Ch 8:   Motor 3 (back-left) command
 *   Ch 9:   Motor 4 (front-left) command
 */

#ifdef __cplusplus
extern "C" {
#endif

void rl_tools_control(bool armed);

#ifdef __cplusplus
}
#endif
