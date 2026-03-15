# Changelog

All notable changes to this project will be documented in this file.

## [1.1.0] - 2026-03-15

### Features
* Support MQTT Last Will and Testament (LWT) ([#21](https://github.com/waldbaer/1w-If/pull/21))
* Support action `sysinfo` ([#24](https://github.com/waldbaer/1w-If/pull/24))

### Fixes / Improvements
* Improve housing WAGO connector cuts

### Dependencies
* Bump ESPAsyncWebServer from 3.9.5 to 3.10.1 ([#22](https://github.com/waldbaer/1w-If/pull/22), [#23](https://github.com/waldbaer/1w-If/pull/23))


## [1.0.0] - 2026-02-06

**Initial release**

### Features
* Access 1-wire devices via ethernet / MQTT protocol
* 4 independent 1-Wire channels with hardware bus masters
* Over-the-air (OTA) updates (web interface and espota protocol)
* Responsive web interface for configuration
* NTP for clock synchronization
* 3D printed housing

### Supported 1-Wire devices

* DS2401 / DS2411 (Silicon Serial Number)
  * Unique Unique 64-Bit Serial Code

* DS18B20 (Temperature sensor)
  * Configurable resolution temperature

* DS2438  (Smart Battery Monitor)
  * Temperature, VAD, VDD
