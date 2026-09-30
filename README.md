# Smart Home MQTT access

The ESP8266 publishes to MQTT, so it can be controlled from a dashboard outside
the home without exposing its HTTP dashboard or ports on the router.

Configure the MQTT values in `lib/Config/secrets.h` using
`lib/Config/secrets.example.h` as the template, then build and upload.

Topics (with the default prefix) are:

| Topic | Direction | Payload |
| --- | --- | --- |
| `smart-home/home/state` | device -> dashboard | Retained JSON state |
| `smart-home/home/availability` | device -> dashboard | Retained `online` / `offline` |
| `smart-home/home/command` | dashboard -> device | `l1_on`, `l1_off`, `stair_manual_on`, `stair_manual_off`, `stair_auto_on`, or `stair_auto_off` |

Use an authenticated broker. For remote access, use a VPN to a home broker, or
configure TLS on a managed broker before using it over the public Internet.
