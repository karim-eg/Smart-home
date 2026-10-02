# Smart Home MQTT access

## NodeMCU wiring

Connect each module to the NodeMCU ESP8266 pins below. `D#` is the NodeMCU
silkscreen label; the GPIO number is the ESP8266 pin used by the firmware.

| NodeMCU pin | ESP8266 GPIO | Connect to | Module terminal / purpose |
| --- | ---: | --- | --- |
| `D1` | GPIO5 | Staircase lamp relay module | `IN` |
| `D2` | GPIO4 | Living room lamp relay module | `IN` |
| `D5` | GPIO14 | PIR motion sensor | `OUT` / signal |
| `D6` | GPIO12 | DHT11 temperature/humidity sensor | `DATA` / signal |
| `D7` | GPIO13 | MQ-5 gas sensor module | `DO` / digital output |
| `A0` | ADC0 | Soil moisture sensor | `AO` / analog output |
| `D8` | GPIO15 | Pump relay or transistor driver | `IN` / control signal |

### Power and load connections

- Connect each module's `GND` to NodeMCU `GND` so the signal has a common
  reference. Power each module from a supply appropriate to that module; do
  not power motors, pumps, or lamps from the NodeMCU.
- The NodeMCU GPIOs use 3.3 V logic and are not 5 V tolerant. If the MQ-5
  module's `DO` can output 5 V, use a level shifter or voltage divider before
  connecting it to `D7`. Check the sensor module's output voltage first.
- Connect relay contacts (`COM` and `NO` or `NC`) in series with the lamp's
  external supply as appropriate. Keep mains wiring isolated and enclosed;
  never connect a mains load directly to the NodeMCU.
- Connect the pump through a suitably rated relay or transistor/MOSFET driver,
  with a separate pump supply. Add a flyback diode when using a bare DC motor or
  a driver that does not already include one. Do not connect the pump directly
  to `D8`.
- `D8` / GPIO15 is a boot-configuration pin that must be LOW while the ESP8266
  starts. Use a driver that does not pull this pin HIGH during reset, and
  ensure the pump stays off during boot. The firmware drives `D8` HIGH only
  when watering.
- Connect the soil sensor's analog output to `A0` and keep its voltage within
  the board's A0 input range (check the exact NodeMCU board revision; do not
  assume every ESP8266 board has the same input divider).
- For a bare DHT11, connect `VCC`, `GND`, and `DATA`; add a pull-up from
  `DATA` to 3.3 V if the sensor board does not already include one. PIR signal
  output must also stay at or below 3.3 V.

The firmware reads the MQ-5 digital output (`D7` / GPIO13); its analog output
is not used. The gas alarm treats HIGH on `DO` as gas detected. Change the
input polarity in `GasSensor.cpp` if the module's comparator is active-low.
Soil is sampled every 2 s; watering starts when the raw reading is above 697,
runs for 500 ms, then waits 6 s before another watering cycle. Adjust
`SOIL_DRY_THRESHOLD` in `lib/Config/Config.h` for the sensor and soil.

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

Open `http://smart-home.local` for the dashboard. The ESP8266 answers this name
over mDNS, which is normally available automatically on the same Wi-Fi/LAN.
Open `http://192.168.100.50` if the client network or browser does not support
mDNS. The ESP8266 must be online, and the client must be on the same local
network; guest Wi-Fi or client isolation can prevent discovery. The mDNS event
loop is polled continuously by the firmware.

Open `http://192.168.100.50/adblock` (or
`http://smart-home.local/adblock`) to view DNS statistics and add or remove blocked domains. The list is stored in
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
your router's DHCP settings. This DNS setting is only needed for DNS filtering
and the custom DNS fallback; mDNS hostname access does not require changing the
client's DNS server. The ESP8266 forwards allowed lookups to Cloudflare
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
