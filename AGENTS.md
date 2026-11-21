# AGENTS.md

## Project scope

This is a Smart Home firmware project managed with PlatformIO.

## Working conventions

- Make focused changes and do not modify unrelated files.
- Follow the existing source structure and naming style.
- Prefer non-blocking firmware code for Arduino/ESP targets.
- Protect resources that are shared by tasks or interrupts.
- Use clear, descriptive names and add comments only where behavior is not obvious.
- Keep credentials, Wi-Fi data, tokens, and private keys out of source control.

## Build and validation

- Use PlatformIO commands for builds and uploads.
- Build all relevant environments before completing changes.
- For hardware-dependent changes, note what could not be tested locally.
- Check compiler warnings and fix issues introduced by the change.

## Testing

- Add or update tests when existing test infrastructure supports the change.
- Test normal operation and relevant failure paths, such as device disconnects and sensor failures.

## Documentation

- Update configuration examples and comments when interfaces or settings change.
- Do not commit generated build output or local environment files.
