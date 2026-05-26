# Meteor75 Firmware — Custom Betaflight with Motor Passthrough

## Overview

Custom Betaflight 4.5.2 fork for the BetaFPV Meteor75 Pro. Adds an `NN_CONTROL` flight mode that bypasses PID control and reads motor commands directly from RC channels 6-9. Based on Eric Pedley's rl-betaflight fork (github.com/EricPedley/rl-betaflight).

The key modification versus Eric's original: instead of running a neural network on the flight controller itself, this firmware reads motor values from RC channels sent by the ground station over CRSF, allowing the NN to run on a more powerful computer.

## Target Hardware

- **Flight Controller**: BetaFPV Meteor75 Pro
- **MCU**: STM32G47X
- **Build target**: `BETAFPVG473`
- **Flash capacity**: 496 KB (current build uses ~389 KB / 73.6%)

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

After flashing, restore the full config by pasting the latest `diff_all.txt` from the appropriate backup folder. At minimum, these must be set:

```
set expresslrs_uid = 192,111,9,248,93,9
feature OSD
feature TELEMETRY
save
```

Then in the Betaflight Modes tab:
- **NN_CONTROL**: AUX2 range 1650-2100
- **OSD DISABLE**: AUX2 range 1650-2100 (same channel, disables OSD during NN control)

## UART / Serial Port Mapping

| Port ID | UART  | Function               |
|---------|-------|------------------------|
| 20      | USB   | MSP (Betaflight config)|
| 0       | UART1 | Unused                 |
| 1       | UART2 | VTX SmartAudio (2048)  |
| 2       | UART3 | CRSF receiver (64)     |
| 3       | UART4 | Unused                 |

VTX SmartAudio on UART2 is defined in `src/config/configs/BETAFPVG473/config.h` line 91.

## Key Source Files

| File | Purpose |
|---|---|
| `src_passthrough/rl_tools/policy.cpp` | Motor passthrough logic: reads `rcData[6-9]`, maps to `motor[]` outputs |
| `oot.mk` | Out-of-tree build config (at repo root) |
| `oot_pre.mk` | Out-of-tree pre-build config: `-DUSE_OSD_SD -DUSE_VTX` flags |
| `mk/local.mk` | Patched to use `-include $(ROOT)/oot.mk` and `-include $(ROOT)/oot_pre.mk` (was relative path `../../oot.mk`) |
| `src/main/target/common_pre.h` | Eric's fork commented out `USE_VTX` and `USE_OSD` here; we re-enable them via `oot_pre.mk` |
| `src/config/configs/BETAFPVG473/config.h` | Board pin mappings, VTX_SMARTAUDIO_UART, SERIALRX_UART |

## NN_CONTROL Flight Mode

- **Box name**: `BOXNNCONTROL`
- **Permanent ID**: 55
- When active, PID controller is completely bypassed
- Motor values come directly from RC channels 6-9 (set by ground station via CRSF)
- Channel mapping in the deployment code: channels 0-3 = sticks, 4 = ARM, 5 = NN_CONTROL, 6-9 = motors

## Makefile Fix

The original rl-betaflight expected the out-of-tree sources at `../../oot.mk` (relative). This was changed to `-include $(ROOT)/oot.mk` and `-include $(ROOT)/oot_pre.mk` in `mk/local.mk` so the build works from the repo root.

## OSD & VTX

- OSD is enabled via `-DUSE_OSD_SD` in `oot_pre.mk`
- OSD is disabled during NN_CONTROL via the AUX2 mode binding (same channel as NN_CONTROL activation)
- VTX SmartAudio is enabled via `-DUSE_VTX` in `oot_pre.mk` (added 2026-05-25, was missing from Eric's fork)

## Backup & Recovery

All backups are in `backup/` organized by firmware version:

```
backup/
├── stock_4.5.0/                     # Factory firmware
│   ├── firmware.hex                 # Stock Betaflight 4.5.0 binary (flash this to fully revert)
│   ├── diff_all.txt                 # Config diff from stock 4.5.0
│   └── dump_all.txt                 # Full config dump from stock 4.5.0
│
├── nn_control_4.5.2_no_vtx/         # First custom build (Mar 22 2026), no VTX support
│   ├── diff_all_initial.txt         # Config right after first flash (Mar 22)
│   ├── dump_all_initial.txt         # Full dump right after first flash
│   ├── diff_all.txt                 # Config as of Mar 23 (OSD tweaked, rates tuned)
│   └── dump_all.txt                 # Full dump as of Mar 23
│
└── nn_control_4.5.2_with_vtx/       # Current build (May 25 2026), VTX enabled
    ├── diff_all_pre_rebuild.txt     # Config dumped from drone right before reflashing
    └── dump_all_pre_rebuild.txt     # Full dump right before reflashing
```

### Recovery to stock firmware
1. Flash `backup/stock_4.5.0/firmware.hex`
2. Open Betaflight CLI, paste contents of `backup/stock_4.5.0/diff_all.txt`
3. Type `save`

### Recovery to NN_CONTROL (current)
1. Build and flash: `make CONFIG=BETAFPVG473 -j$(nproc)`, then DFU flash
2. Open Betaflight CLI, paste contents of the latest `diff_all.txt` from `nn_control_4.5.2_with_vtx/`
3. Type `save`

### Taking a new backup
```python
# Connect drone via USB, then:
import serial, time
ser = serial.Serial('/dev/ttyACM0', 115200, timeout=2)
time.sleep(1)
ser.write(b'#')       # enter CLI
time.sleep(0.5)
ser.read(ser.in_waiting)
ser.write(b'diff all\n')  # or 'dump all\n'
time.sleep(3)
data = b''
while True:
    chunk = ser.read(ser.in_waiting)
    if not chunk: break
    data += chunk
    time.sleep(0.5)
# Save data to file
ser.write(b'exit\n')
ser.close()
```

## TX Module DIP Switches (BetaFPV Micro TX V2)

| Switches | Mode |
|---|---|
| 1 & 2 ON | USB serial mode (for CRSF over USB) |
| 3 & 4 ON | Radio mode (normal flying) |

## Related Projects

- **RL Training**: `/home/teo/DRONE/meteor75_rl/` (Isaac Lab extension, trains the hover policy)
- **Deployment**: `/home/teo/DRONE/drone_control/` (CRSF controller, sends motor commands from policy)
- **Eric Pedley's original**: `github.com/EricPedley/rl-betaflight`
