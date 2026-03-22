# Meteor75 RL Firmware — Motor Passthrough

Custom Betaflight firmware for BetaFPV Meteor75 with motor passthrough mode.

Based on Eric Pedley's rl-betaflight fork which adds NN_CONTROL flight mode.

## What this does

When NN_CONTROL mode is activated via AUX switch, Betaflight's PID is bypassed.
Motor commands are read directly from RC channels 6-9 sent by the PC.

## Channel mapping

```
Ch 0: Roll (stick)          Ch 6: Motor 1 (FR) — from PC
Ch 1: Pitch (stick)         Ch 7: Motor 2 (BR) — from PC
Ch 2: Throttle (stick)      Ch 8: Motor 3 (BL) — from PC
Ch 3: Yaw (stick)           Ch 9: Motor 4 (FL) — from PC
Ch 4: AUX1 — Arm
Ch 5: AUX2 — NN_CONTROL mode
```

## Safety

- Normal flight (ACRO/ANGLE/HORIZON) works as before
- NN_CONTROL only activates when AUX switch is flipped
- Flip switch back = normal PID takes over immediately
- ELRS failsafe works as normal

## Build

```bash
make arm_sdk_install
make configs
make CONFIG=BETAFPVG473 OOT_DIR=src_passthrough -j$(nproc)
```
