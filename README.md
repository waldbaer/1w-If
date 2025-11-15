
# <img src="doc/logo/logo-300dpi.png" title="1-Wire Interface Logo" width="400" />

An ESP32-based 1-Wire interface supporting access with physical ethernet and MQTT protocol.

![Overview Diagram](doc/overview.excalidraw.png)



## Key Features

* Access 1-wire devices via ethernet / MQTT protocol
* 1-Wire bus access via hardware IC
* Over-the-air (OTA) updates (web interface and espota protocol)
* Responsive web interface
  <br/>
  <img src="doc/screenshots/web-dashboard.png" title="Dashboard"  width="350"/> <img src="doc/screenshots/web-console.png" title="Dashboard" width="350"/>

## Supported 1-Wire devices

* DS2401 / DS2411 (Silicon Serial Number)
  * Unique Unique 64-Bit Serial Code

* DS18B20 (Temperature sensor)
  * Configurable resolution temperature

* DS2438  (Smart Battery Monitor)
  * Temperature, VAD, VDD

## Web-Interface

Default login:
- Username: `admin`
- Password: `1w-If`


## MQTT API
`%topic%` is a configurable prefix. Default: `1wIf`

### Commands

General MQTT topic for commands: `%topic%/cmd`

General MQTT topic for responses: `%topic%/stat`

Payload Format:

```
{
  "action": "<command>",
  ... action specific parameters / attributes ...
}
```

#### Command 'Restart'

Restart the entire system.
```
{
  "action": "restart",
}
```

Response:
```
{
  "action": "restart",
  "acknowledge": true
}
```


#### Command 'Scan'

Scan 1-wire buses for all available devices:

```
{
  "action": "scan",
}
```

Scan 1-wire buses for availability of a specific device:

```
{
  "action": "scan",
  "device_id": "01.D2C79A1A0000"
}
```

Scan 1-wire buses for availability of a specific device family:

```
{
  "action": "scan",
  "family_code": 40
}
```

The response lists all available devices and the supported attributes (for `Read` / `Subscribe` / `Unsubscribe` commands):

Example Response:
```
{
  "action": "scan",
  "devices": [
    {
      "device_id": "01.D2C79A1A0000",
      "presence": true,
      "attributes": ["presence"]
    },
    {
      "device_id": "28.8F0945161301",
      "presence": true,
      "attributes": ["presence", "temperature"]
    },
    {
      "device_id": "26.563743020000",
      "presence": true,
      "attributes": ["presence", "VAD", "VDD"]
    },
    ...
  ]
}
```

#### Command 'Read'

Read value(s) of a single 1-wire device:

```
{
  "action": "read",
  "device_id": "28.8F0945161301",
  "attribute": "temperature"
}
```

Example Response:
```
{
  "action": "read",
  "device": {
     "device_id": "28.8F0945161301",
     "temperature": 24.75
  }
}
```

Read value(s) of a specific device family:

```
{
  "action": "read",
  "family_code": 40,
  "attribute": "temperature"
}
```

Example Response:
```
{
  "action": "read",
  "family_code": 40,
  "devices": [
    {
      "device_id": "28.8F0945161301",
      "temperature": 24.75
    },
    ...
  ]
}
```

Also the presence of a device can be accessed via the read command using the attribute `presence`.
This is similar to the scan command.

```
{
  "action": "read",
  "device_id": "28.8F0945161301",
  "attribute": "presence"
}
```

Example Response:
```
{
  "action": "read",
  "device": {
     "device_id": "28.8F0945161301",
     "presence": true
  }
}
```


#### Command 'Subscribe / Unsubscribe'

Subscribe to cyclic updates for 1-wire device attributes.
After successful subscription this triggers internally a `Read` command with the configured internal time.

Unit of 'interval': _milliseconds_

```
{
  "action": "subscribe",
  "device_id": "28.8F0945161301",
  "attribute": "temperature",
  "interval": 1000
}
```

Example response acknowledging the successful subscription:
```
{
  "action": "subscribe",
  "acknowledge": true,
  "device": {
    "device_id": "28.8F0945161301"
  }
}
```


Unsubscribe from the attribute:
```
{
  "action": "unsubscribe",
  "device_id": "28.8F0945161301",
  "attribute": "temperature"
}
```

It is also possible to subscribe to the attribute `presence` to get cyclic updates about the availability of dedicated
devices:
```
{
    "action": "subscribe",
    "device_id": "01.D2C79A1A0000",
    "attribute": "presence",
    "interval": 2000
}
```

Example of the `Read` command for attribute `presence` cyclically triggered by the subscription:
```
{
  "action": "read",
  "device": {
     "device_id": "01.D2C79A1A0000",
     "presence": false
  }
}
```

## Development

```
# Build firmware
./build.sh

# Flash firmware + LittleFS filesystem
./flash-fs.sh

# Optional: Monitor via serial outputs
./monitor.sh
```

Flash / upload with dedicated method:
```
# via espota
pio run --target upload -e wt32-eth01_espota

# via esptool (serial)
pio run --target upload -e wt32-eth01_esptool
```

Run tests
```
cd test
pdm install --dev

# Create a test_env_config.yaml in the test directory
cp example_test_env_config.yaml test_env_config.yaml
# Adapt all MQTT / device settings test_env_config.yaml
vim test_env_config.yaml

# Execute tests
pdm run tests
# Alternative: with filter
pdm run tests -k "<test name filter>"

# Lint/Format of test implementations
pdm run lint
pdm run format
```
