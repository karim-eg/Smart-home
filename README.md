# Smart Home MQTT access

## Plant irrigation and gas sensor pins

The ESP8266 uses its single analog input (`A0`) for the soil moisture sensor.
The MQ-5 is read from its digital output (`DO`, `D7` / GPIO13); its analog
output is not used. The pump driver input is connected to `D0` / GPIO16.
Connect the pump through a suitable relay or transistor driver, not directly
to the ESP8266 pin. The irrigation controller reads soil moisture every 2 s,
waters for 500 ms when the raw value is above 697, then waits 6 s before it can
water again. Adjust `SOIL_DRY_THRESHOLD` in `lib/Config/Config.h` for the
specific sensor and soil.

The gas alarm currently treats a HIGH level on MQ-5 `DO` as gas detected, as
the previous firmware did. Change the input polarity in `GasSensor.cpp` if the
module's comparator output is configured active-low.

## DNS ad blocker

Web files are kept separately in `data/` and served from LittleFS:

```text
data/
├── index.html
├── style.css
├── app.js
├── adblock.html
├── adblock.css
├── adblock.js
├── update.html
├── update.css
└── update.js
```

Open `http://192.168.100.50/adblock` (or `http://smart-home.local/adblock`)
to view DNS statistics and add or remove blocked domains. The list is stored in
LittleFS and survives a restart. The dashboard home page links to this screen.

After editing files in `data/`, build and upload the filesystem image from the
project directory:

```sh
pio run -e esp8266 -t buildfs
pio run -e esp8266 -t uploadfs
```

In VS Code, the equivalent PlatformIO actions are **Build Filesystem Image**
and **Upload Filesystem Image**. Upload firmware separately with the normal
PlatformIO upload action.

To apply filtering to a phone or computer, set its DNS server to
`192.168.100.50`, or configure that address as the DNS server distributed by
your router's DHCP settings. The ESP8266 forwards allowed lookups to Cloudflare
DNS (`1.1.1.1`) and reserves `smart-home.local` for the local dashboard.
DNS-based blocking does not cover apps that use encrypted DNS or every ad
provider. After enabling LittleFS, upload the filesystem image once so the
persistent domain list can be created.

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
