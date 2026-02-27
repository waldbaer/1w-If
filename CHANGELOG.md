# Changelog

All notable changes to this project will be documented in this file.

## [1.1.0] - (not yet released)

### Features
* Support MQTT Last Will and Testament (LWT)

### Fixes / Improvements
* Improve housing

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
