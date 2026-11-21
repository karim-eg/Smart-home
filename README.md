# Smart Home - IOT Project

## NodeMCU wiring

Connect each module to the NodeMCU ESP8266 pins below. `D#` is the NodeMCU
silkscreen label; the GPIO number is the ESP8266 pin used by the firmware.

| NodeMCU pin | ESP8266 GPIO | Connect to | Module terminal / purpose |
| --- | ---: | --- | --- |
| `D1` | GPIO5 | Staircase lamp relay module | `IN` |
| `D2` | GPIO4 | Living room lamp relay module | `IN` |
| `D8` | GPIO15 | PIR motion sensor | `OUT` / signal; must be LOW during boot |
| `RX` | GPIO3 | DHT11 temperature/humidity sensor | `DATA` / signal |
| `A0` | ADC0 | MQ-5 gas sensor module | `AO` / analog output |
| `D3` | GPIO0 | SD card module | `CS` / chip select |
| `D5` | GPIO14 | SD card module | `SCK` / clock |
| `D6` | GPIO12 | SD card module | `MISO` / card to NodeMCU |
| `D7` | GPIO13 | SD card module | `MOSI` / NodeMCU to card |
| `TX` | GPIO1 | IR LED driver for the fan | Transistor driver input |

### Power and load connections

- Connect each module's `GND` to NodeMCU `GND` so the signal has a common
  reference. Power each module from a supply appropriate to that module; do
  not power motors, pumps, or lamps from the NodeMCU.
- Connect the MQ-5 module's `AO` output to `A0`. Its voltage must stay within
  the A0 input range of the exact NodeMCU board being used.
- Add a 10 kΩ pull-up from `D3` / GPIO0 (`CS`) to 3.3 V so the SD card stays
  deselected while the ESP8266 boots. This pin affects ESP8266 startup.
- Check the SD adapter before powering it: a bare microSD socket needs 3.3 V;
  use `VIN`/5 V only when the adapter has a 3.3 V regulator and suitable logic
  level shifting. Connect all module grounds together and format the card as
  FAT32.
- Connect relay contacts (`COM` and `NO` or `NC`) in series with the lamp's
  external supply as appropriate. Keep mains wiring isolated and enclosed;
  never connect a mains load directly to the NodeMCU.
- The relay modules use active-low inputs: LOW turns the relay ON and HIGH
  turns it OFF. The firmware initializes both relay outputs OFF. A brief relay-module LED
  flash at power-up can happen while ESP8266 pins are resetting; if the relay
  clicks or the lamp flashes, use relay modules with suitable input circuitry
  and add a 10 kΩ pull-down from each active-HIGH relay `IN` to `GND`.
- `D8` / GPIO15 is a boot-configuration pin that must be LOW while the ESP8266
  starts. Confirm the PIR output stays LOW during reset/boot; a HIGH level can
  prevent the ESP8266 from starting. Keep the PIR output at or below 3.3 V.
- Connect an infrared LED through a transistor driver to `TX` (`GPIO1`) so the
  controller can send the fan's remote commands. Do not drive a high-power IR
  LED directly from the GPIO. `TX` is reassigned to the IR output, so firmware
  serial logging and the serial monitor are disabled. The dashboard's **Fan
  Setting** is stored on the SD card; when enabled, the saved power/speed and
  swing sequence is sent after the controller starts.
- For a bare DHT11, connect `VCC`, `GND`, and `DATA`; add a pull-up from
  `DATA` to 3.3 V if the sensor board does not already include one. Connect its
  `DATA` wire to `RX` / GPIO3. PIR signal
  output must also stay at or below 3.3 V.

The firmware reads the MQ-5 analog output (`A0`) and shows its raw value on the
dashboard. It treats readings of 400 or higher as a gas alert. Adjust
`GAS_ALERT_THRESHOLD` in `lib/Config/Config.h` after observing normal readings
in the dashboard.

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
`http://smart-home.local/adblock`) to view DNS statistics, add or remove blocked
domains, and see recent blocked-domain history. The domain list, fan setting,
DNS counters, and blocked-domain history are stored on the SD card. Existing
`/block.txt` and `/fan.json` files are copied from LittleFS on the first boot
with an SD card, then the LittleFS copies are removed. LittleFS remains in use
for the dashboard files under `data/`.

Only blocked-domain requests are written to the history log, avoiding an SD
write for every DNS lookup. The log rotates at about 32 KiB and retains the
previous segment; DNS counters are checkpointed once per minute, so a sudden
power loss can lose up to the last minute of counter changes. The domain list
is kept in RAM while filtering for fast DNS responses; SD moves persistent
storage off LittleFS but does not eliminate that small runtime cache.

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
provider. Keep the SD card inserted during operation. If it is missing, the
dashboard and built-in DNS blocklist can still start, but SD-backed changes and
history cannot be saved.

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
