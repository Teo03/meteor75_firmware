# Meteor75 Firmware — Custom Betaflight with Motor Passthrough

## Overview

Custom Betaflight 4.5.2 fork for the BetaFPV Meteor75 Pro. Adds an `NN_CONTROL` flight mode that bypasses PID control and reads motor commands directly from RC channels 6-9. Based on Eric Pedley's rl-betaflight fork (github.com/EricPedley/rl-betaflight).

The key modification versus Eric's original: instead of running a neural network on the flight controller itself, this firmware reads motor values from RC channels sent by the ground station over CRSF, allowing the NN to run on a more powerful computer.

## Target Hardware

- **Flight Controller**: BetaFPV Meteor75 Pro
- **MCU**: STM32G47X
- **Build target**: `BETAFPVG473`
- **Flash capacity**: 496 KB (current build uses ~353 KB / 70%)

## Build

```bash
make CONFIG=BETAFPVG473 -j$(nproc)
```

Output: `obj/betaflight_4.5.2_STM32G47X_BETAFPVG473.hex`

## Flash

1. Enter DFU mode: type `bl` in Betaflight CLI (or hold boot button during power-on)
2. Convert hex to bin:
   ```bash
   arm-none-eabi-objcopy -I ihex -O binary \
     obj/betaflight_4.5.2_STM32G47X_BETAFPVG473.hex \
     obj/betaflight_4.5.2_STM32G47X_BETAFPVG473.bin
   ```
3. Flash:
   ```bash
   dfu-util -a 0 --dfuse-address 0x08000000:force:mass-erase:leave \
     -D obj/betaflight_4.5.2_STM32G47X_BETAFPVG473.bin
   ```

## Post-Flash Configuration (Betaflight CLI)

```
set expresslrs_uid = 192,111,9,248,93,9
feature OSD
feature TELEMETRY
save
```

Then in the Betaflight Modes tab:
- **NN_CONTROL**: AUX2 range 1650-2100
- **OSD DISABLE**: AUX2 range 1650-2100 (same channel, disables OSD during NN control)

## Key Source Files

| File | Purpose |
|---|---|
| `src_passthrough/rl_tools/policy.cpp` | Motor passthrough logic: reads `rcData[6-9]`, maps to `motor[]` outputs |
| `oot.mk` | Out-of-tree build config (at repo root) |
| `oot_pre.mk` | Out-of-tree pre-build config, includes `-DUSE_OSD_SD` flag |
| `mk/local.mk` | Patched to use `-include $(ROOT)/oot.mk` and `-include $(ROOT)/oot_pre.mk` (was relative path `../../oot.mk`) |

## NN_CONTROL Flight Mode

- **Box name**: `BOXNNCONTROL`
- **Permanent ID**: 55
- When active, PID controller is completely bypassed
- Motor values come directly from RC channels 6-9 (set by ground station via CRSF)
- Channel mapping in the deployment code: channels 0-3 = sticks, 4 = ARM, 5 = NN_CONTROL, 6-9 = motors

## Makefile Fix

The original rl-betaflight expected the out-of-tree sources at `../../oot.mk` (relative). This was changed to `-include $(ROOT)/oot.mk` and `-include $(ROOT)/oot_pre.mk` in `mk/local.mk` so the build works from the repo root.

## OSD

OSD is enabled via `-DUSE_OSD_SD` in `oot_pre.mk`. OSD is disabled during NN_CONTROL via the AUX2 mode binding (same channel as NN_CONTROL activation).

## Backup & Recovery

The `backup/` directory contains:
- **Stock firmware hex**: `betaflight_4.5.0_STM32G47X_STOCK.hex`
- **Original config dump**: `meteor75_diff_all.txt`
- **Current NN_CONTROL config dump**: saved after NN_CONTROL modifications

### Recovery procedure
1. Flash stock firmware: `betaflight_4.5.0_STM32G47X_STOCK.hex`
2. Open Betaflight CLI
3. Paste contents of `meteor75_diff_all.txt`
4. Type `save`

## TX Module DIP Switches (BetaFPV Micro TX V2)

| Switches | Mode |
|---|---|
| 1 & 2 ON | USB serial mode (for CRSF over USB) |
| 3 & 4 ON | Radio mode (normal flying) |

## Related Projects

- **RL Training**: `/home/teo/DRONE/meteor75_rl/` (Isaac Lab extension, trains the hover policy)
- **Deployment**: `/home/teo/DRONE/drone_control/` (CRSF controller, sends motor commands from policy)
- **Eric Pedley's original**: `github.com/EricPedley/rl-betaflight`
