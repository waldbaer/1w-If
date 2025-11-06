import json

import pytest

from tests.env.config_model import ConfigModel
from tests.env.logger import Logger
from tests.env.mqtt_fixture import mqtt_capture  # noqa: F401
from tests.env.mqtt_protocol import MqttProtocol as p
from tests.env.one_wire_address import OneWireAddress
from tests.env.one_wire_device_def import OneWireDeviceDefinition as ow_dd

# ---- Setup Test Environment ------------------------------------------------------------------------------------------
config = ConfigModel.load_from_yaml()
logger = Logger.get(__name__)

# ---- Test Implementation ---------------------------------------------------------------------------------------------


@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_scan_all(mqtt_capture) -> None:
    logger.info("Sending scan all request")

    request = json.dumps({p.ATTRIB_ACTION: p.ACTION_SCAN})
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    response_devices = response.get(p.ATTRIB_DEVICES)
    assert response_devices is not None
    assert len(response_devices) == len(config.devices)

    for expected_device in config.devices:
        match = next((d for d in response_devices if d[p.ATTRIB_DEVICE_ID] == str(expected_device.device_id)), None)
        assert match is not None
        assert match.get(p.ATTRIB_PRESENCE) is True
        assert match.get(p.ATTRIB_ATTRIBUTES) == ow_dd.get_attributes(expected_device.device_id)


@pytest.mark.parametrize("device", config.devices)
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_scan_single_device(mqtt_capture, device) -> None:
    logger.info(f"Sending single-device scan request for device {device.device_id}")

    request = json.dumps({p.ATTRIB_ACTION: p.ACTION_SCAN, p.ATTRIB_DEVICE_ID: str(device.device_id)})
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    assert response.get(p.ATTRIB_ACTION) == p.ACTION_SCAN
    response_device = response.get(p.ATTRIB_DEVICE)
    assert response_device is not None
    assert response_device.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)
    assert response_device.get(p.ATTRIB_PRESENCE) is True
    assert response_device.get(p.ATTRIB_ATTRIBUTES) == ow_dd.get_attributes(device.device_id)


@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_scan_single_device_not_available(mqtt_capture) -> None:
    unknown_device_id = OneWireAddress("26.FFFFFFFFFFFF")
    logger.info(f"Sending single-device scan request for not available device {unknown_device_id}")

    request = json.dumps({p.ATTRIB_ACTION: p.ACTION_SCAN, p.ATTRIB_DEVICE_ID: str(unknown_device_id)})
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    assert response.get(p.ATTRIB_ACTION) == p.ACTION_SCAN
    response_device = response.get(p.ATTRIB_DEVICE)
    assert response_device is not None
    assert response_device.get(p.ATTRIB_DEVICE_ID) == str(unknown_device_id)
    assert response_device.get(p.ATTRIB_PRESENCE) is False
    assert response_device.get(p.ATTRIB_ATTRIBUTES) == ow_dd.get_attributes(unknown_device_id)


@pytest.mark.parametrize("family_code", config.get_family_codes_from_devices())
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_scan_family(mqtt_capture, family_code) -> None:
    logger.info(f"Sending single-device scan request for family code {family_code}")

    request = json.dumps({p.ATTRIB_ACTION: p.ACTION_SCAN, p.ATTRIB_FAMILY_CODE: family_code})
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    assert response.get(p.ATTRIB_ACTION) == p.ACTION_SCAN
    assert response.get(p.ATTRIB_FAMILY_CODE) == family_code

    response_devices = response.get(p.ATTRIB_DEVICES)
    assert response_devices is not None

    expected_devices = config.get_by_family_code(family_code)
    assert len(response_devices) == len(expected_devices)

    for expected_device in expected_devices:
        match = next((d for d in response_devices if d[p.ATTRIB_DEVICE_ID] == str(expected_device.device_id)), None)
        assert match is not None
        assert match.get(p.ATTRIB_PRESENCE) is True
        assert match.get(p.ATTRIB_ATTRIBUTES) == ow_dd.get_attributes(expected_device.device_id)


@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_scan_device_and_family(mqtt_capture) -> None:
    logger.info("Sending invalid single-device scan request for device ID and family code")

    device_id = OneWireAddress("26.FFFFFFFFFFFF")
    family_code = device_id.get_family_code()

    request = json.dumps(
        {p.ATTRIB_ACTION: p.ACTION_SCAN, p.ATTRIB_DEVICE_ID: str(device_id), p.ATTRIB_FAMILY_CODE: family_code}
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    error = response.get(p.ATTRIB_ERROR)
    assert error is not None
    assert error.get(p.ATTRIB_MESSAGE) == "Missing or invalid JSON attributes 'device_id' or 'family_code'."
    response_request = error.get(p.ATTRIB_REQUEST)
    assert response_request is not None
    assert response_request.get(p.ATTRIB_ACTION) == p.ACTION_SCAN
    assert response_request.get(p.ATTRIB_DEVICE_ID) == str(device_id)
    assert response_request.get(p.ATTRIB_FAMILY_CODE) == family_code


@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_scan_invalid_device_ID(mqtt_capture) -> None:
    invalid_device_id = "abc.INVALID"

    logger.info(f"Sending invalid single-device ID scan request: {invalid_device_id}")

    request = json.dumps({p.ATTRIB_ACTION: p.ACTION_SCAN, p.ATTRIB_DEVICE_ID: invalid_device_id})
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    error = response.get(p.ATTRIB_ERROR)
    assert error is not None
    assert error.get(p.ATTRIB_MESSAGE) == "Missing or invalid JSON attributes 'device_id' or 'family_code'."
    response_request = error.get(p.ATTRIB_REQUEST)
    assert response_request is not None
    assert response_request.get(p.ATTRIB_ACTION) == p.ACTION_SCAN
    assert response_request.get(p.ATTRIB_DEVICE_ID) == invalid_device_id


@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_scan_invalid_JSON(mqtt_capture) -> None:
    logger.info("Sending invalid JSON request")

    request = '{ "action": "scan", INVALID_JSON'
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    error = response.get(p.ATTRIB_ERROR)
    assert error is not None
    assert error.get(p.ATTRIB_MESSAGE) == "Failed to deserialize MQTT message."
    assert error.get(p.ATTRIB_REQUEST) is None
