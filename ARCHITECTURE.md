# Smart Home Architecture

## Overview

This PlatformIO project implements an ESP-based smart-home controller. It coordinates local sensors and actuators, provides a network-facing user interface, and exposes state and configuration through the firmware's communication channels.

## Goals

- Reliable local control even when internet connectivity is unavailable
- Low-power operation for battery-powered or constrained devices
- Clear separation between hardware I/O, application logic, and connectivity
- Safe handling of invalid sensor readings and configuration changes
- Straightforward maintenance and extension to additional rooms or devices

## System Context

```text
                         ┌─────────────────────┐
                         │ Mobile/desktop app  │
                         └──────────┬──────────┘
                                    │ Wi-Fi/API/UI
                                    ▼
┌──────────────┐  GPIO/I2C/SPI   ┌──────────────────────┐
│ Sensors      │◄────────────────│ ESP smart-home MCU   │
└──────────────┘                 │                      │
┌──────────────┐  GPIO/I2C/SPI   │ - state management   │
│ Actuators    │◄────────────────│ - device rules       │
└──────────────┘                 │ - network services   │
                                 │ - persistence        │
                                 └──────────────────────┘
```

## Firmware Layout

Suggested source layout:

```text
include/
  app_config.h
  constants.h
lib/
  sensors/
  actuators/
  networking/
src/
  main.cpp
platformio.ini
```

- `main.cpp` initializes modules, starts the network stack, and runs the cooperative event loop.
- Hardware drivers isolate MCU pins and bus details.
- Application services own the smart-home state and automation rules.
- Persistence stores only validated configuration and calibration data.

## Major Components

### Hardware Abstraction

Encapsulate GPIO, ADC, I²C, SPI, timers, and relay-control operations behind interfaces. Application code should not manipulate raw pins or bus registers directly.

### Sensor Manager

Polls or receives interrupts from sensors and normalizes readings into engineering units. Each reading should include:

- A validated value
- Timestamp or monotonic age
- Sensor or room identifier
- Health status

Outliers and stale readings must not trigger unsafe actuator changes.

### Actuator Manager

Controls relays, switches, lights, motors, and similar outputs. Commands are validated and translated to hardware-safe operations. Interlocks and debounce or timeout rules prevent conflicting or repeated actions.

### Application and Automation Logic

Maintains the current system state and evaluates rules such as schedules, thresholds, and sensor combinations. Changes should be event-driven rather than implemented as blocking delays in the main loop.

### Networking and UI

Provides configuration and monitoring over the device's network interface. Network updates must be processed asynchronously and applied only after validation. Status responses should distinguish connected, unavailable, stale, and offline states.

### Configuration and Persistence

Configuration should include Wi-Fi settings, room and device identifiers, thresholds, schedules, calibration values, and feature flags. Defaults must work when no persistent configuration exists, and writes must be atomic or otherwise resilient to power loss.

## Runtime Flow

1. Initialize hardware and load validated configuration.
2. Initialize persisted state and apply safe actuator defaults.
3. Start the sensor, networking, scheduling, and watchdog subsystems.
4. Collect readings and publish state-change events.
5. Evaluate automation rules and dispatch validated actuator commands.
6. Process remote commands, configuration updates, and status requests.
7. Persist configuration only when it changes, and periodically persist critical runtime state if required.

## State Model

A device state should contain, at minimum:

- Stable identifier
- Desired state
- Observed state
- Health
- Last update time
- Configuration revision

State transitions should be idempotent. A repeated command must not invert or duplicate an action.

## Concurrency and Timing

Use an event loop or cooperative task model appropriate for the selected framework. Avoid blocking operations in `loop()` or in shared callbacks. Reserve interrupts for short operations such as capturing timestamps or flags; perform work in normal context.

Recommended timing behavior:

- Sensor sampling: configured interval with overflow-safe monotonic timestamps
- Network processing: continuous, but bounded per cycle
- Watchdog: reset only after a demonstrated subsystem failure or sustained deadlock
- Actuator transitions: enforce command cooldowns and safety delays

## Error Handling and Fail-Safe Behavior

- Treat missing or malformed configuration as a default or safe configuration.
- Mark disconnected or stale sensors as unavailable rather than using old values silently.
- Fail actuators to a documented safe state during startup or critical errors.
- Use bounded retries with backoff for network operations.
- Log useful context without exposing passwords or private network keys.
- Provide reset/recovery behavior for invalid state or repeated boot failures.

## Security

- Store network credentials outside source control.
- Keep administrative operations behind an authenticated interface when remote access is enabled.
- Validate lengths and value ranges on all externally supplied configuration.
- Disable unused services and minimize exposed HTTP endpoints.
- Prefer local network-only operation unless internet access is explicitly required.

## Testing Strategy

- Unit-test normalization, threshold logic, scheduling, and state transitions.
- Use mocks for GPIO, buses, timers, and network interfaces.
- Test configuration migration and corrupted or partially written storage.
- Exercise sensor timeout, out-of-range readings, command loss, and power-loss recovery.
- Run hardware-in-the-loop tests for relay, motor, and sensor behavior.

## Build and Deployment

Use one PlatformIO environment per target board. Environment configuration should define serial speed, board settings, partitions, build flags, and release metadata. Firmware updates must validate an image and preserve configuration where compatible.

Before deployment:

1. Build with warnings enabled.
2. Verify safe defaults and actuator startup behavior.
3. Confirm the board can recover after a network or storage failure.
4. Test all external inputs and outputs under expected load.
5. Record board, firmware, and configuration revisions.

## Future Extensions

- MQTT or another broker integration
- Home Assistant discovery and integration
- OTA update and signed firmware packages
- Centralized logging and metrics
- Multiple-node synchronization
- Battery-aware sleep and mesh networking
