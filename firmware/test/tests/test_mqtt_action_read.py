import json

import pytest

from tests.env.config_model import ConfigModel
from tests.env.logger import Logger
from tests.env.mqtt_fixture import mqtt_capture  # noqa: F401
from tests.env.mqtt_protocol import MqttProtocol as p
from tests.env.one_wire_address import OneWireAddress
from tests.env.one_wire_device_def import OneWireDeviceDefinition as ow_dd
from tests.env.time_util import TimeUtil

# ---- Setup Test Environment ------------------------------------------------------------------------------------------
config = ConfigModel.load_from_yaml()
logger = Logger.get(__name__)

# ---- Test Implementation ---------------------------------------------------------------------------------------------


@pytest.mark.parametrize("device", config.devices)
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_read_single_device_presence(mqtt_capture, device) -> None:
    logger.info(f"Sending read request for attribute 'presence' to single device {device.device_id}.")

    request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_READ,
            p.ATTRIB_DEVICE_ID: str(device.device_id),
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_PRESENCE,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    TimeUtil.assert_timestamp(response.get(p.ATTRIB_TIME))
    assert response.get(p.ATTRIB_ACTION) == p.ACTION_READ
    response_device = response.get(p.ATTRIB_DEVICE)
    assert response_device is not None
    assert response_device.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)
    assert response_device.get(p.ATTRIB_PRESENCE) is True


@pytest.mark.parametrize("device", config.get_by_attribute(p.ATTRIB_TEMPERATURE))
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_read_single_device_temperature(mqtt_capture, device) -> None:
    logger.info(f"Sending read request for attribute 'temperature' to single device {device.device_id}.")

    request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_READ,
            p.ATTRIB_DEVICE_ID: str(device.device_id),
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_TEMPERATURE,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    TimeUtil.assert_timestamp(response.get(p.ATTRIB_TIME))
    assert response.get(p.ATTRIB_ACTION) == p.ACTION_READ
    response_device = response.get(p.ATTRIB_DEVICE)
    assert response_device is not None
    assert response_device.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)
    ow_dd.assert_temperature_range(response_device.get(p.ATTRIB_TEMPERATURE))


@pytest.mark.parametrize("device", config.get_by_attribute(p.ATTRIB_VAD))
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_read_single_device_vad(mqtt_capture, device) -> None:
    logger.info(f"Sending read request for attribute 'VAD' to single device {device.device_id}.")

    request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_READ,
            p.ATTRIB_DEVICE_ID: str(device.device_id),
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_VAD,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    TimeUtil.assert_timestamp(response.get(p.ATTRIB_TIME))
    assert response.get(p.ATTRIB_ACTION) == p.ACTION_READ
    response_device = response.get(p.ATTRIB_DEVICE)
    assert response_device is not None
    assert response_device.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)
    ow_dd.assert_vad_range(response_device.get(p.ATTRIB_VAD))


@pytest.mark.parametrize("device", config.get_by_attribute(p.ATTRIB_VDD))
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_read_single_device_vdd(mqtt_capture, device) -> None:
    logger.info(f"Sending read request for attribute 'VDD' to single device {device.device_id}.")

    request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_READ,
            p.ATTRIB_DEVICE_ID: str(device.device_id),
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_VDD,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    TimeUtil.assert_timestamp(response.get(p.ATTRIB_TIME))
    assert response.get(p.ATTRIB_ACTION) == p.ACTION_READ
    response_device = response.get(p.ATTRIB_DEVICE)
    assert response_device is not None
    assert response_device.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)
    ow_dd.assert_vdd_range(response_device.get(p.ATTRIB_VDD))


@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_read_single_device_not_available(mqtt_capture) -> None:
    unknown_device_id = OneWireAddress("26.FFFFFFFFFFFF")
    logger.info(f"Sending read request to unknown single device {unknown_device_id}.")

    request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_READ,
            p.ATTRIB_DEVICE_ID: str(unknown_device_id),
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_PRESENCE,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    TimeUtil.assert_timestamp(response.get(p.ATTRIB_TIME))
    assert response.get(p.ATTRIB_ACTION) == p.ACTION_READ
    response_device = response.get(p.ATTRIB_DEVICE)
    assert response_device is not None
    assert response_device.get(p.ATTRIB_DEVICE_ID) == str(unknown_device_id)
    assert response_device.get(p.ATTRIB_PRESENCE) is False


@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_read_single_device_device_id_missing(mqtt_capture) -> None:
    logger.info("Sending read request for 'presence' with missing device ID.")

    request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_READ,
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_PRESENCE,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    TimeUtil.assert_timestamp(response.get(p.ATTRIB_TIME))
    error = response.get(p.ATTRIB_ERROR)
    assert error is not None
    assert error.get(p.ATTRIB_MESSAGE) == "Missing or invalid JSON attributes 'device_id' or 'family_code'."
    response_request = error.get(p.ATTRIB_REQUEST)
    assert response_request is not None
    assert response_request.get(p.ATTRIB_ACTION) == p.ACTION_READ
    assert response_request.get(p.ATTRIB_ATTRIBUTE) == p.ATTRIB_PRESENCE


@pytest.mark.parametrize("device", config.devices)
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_read_single_device_attribute_missing(mqtt_capture, device) -> None:
    logger.info(f"Sending read request with missing attribute to single device {device.device_id}.")

    request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_READ,
            p.ATTRIB_DEVICE_ID: str(device.device_id),
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    TimeUtil.assert_timestamp(response.get(p.ATTRIB_TIME))
    error = response.get(p.ATTRIB_ERROR)
    assert error is not None
    assert error.get(p.ATTRIB_MESSAGE) == "Missing or invalid JSON attribute 'attribute'."
    response_request = error.get(p.ATTRIB_REQUEST)
    assert response_request is not None
    assert response_request.get(p.ATTRIB_ACTION) == p.ACTION_READ
    assert response_request.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)


@pytest.mark.parametrize("family_code", config.get_family_codes_from_devices())
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_read_family_presence(mqtt_capture, family_code) -> None:
    logger.info(f"Sending read request for attribute 'presence' for device family {family_code}.")

    request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_READ,
            p.ATTRIB_FAMILY_CODE: family_code,
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_PRESENCE,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    TimeUtil.assert_timestamp(response.get(p.ATTRIB_TIME))
    assert response.get(p.ATTRIB_ACTION) == p.ACTION_READ
    response_devices = response.get(p.ATTRIB_DEVICES)
    assert response_devices is not None

    expected_devices = config.get_by_family_code(family_code)
    assert len(response_devices) == len(expected_devices)

    for expected_device in expected_devices:
        match = next((d for d in response_devices if d[p.ATTRIB_DEVICE_ID] == str(expected_device.device_id)), None)
        assert match is not None
        assert match.get(p.ATTRIB_PRESENCE) is True


@pytest.mark.parametrize("family_code", ow_dd.get_supported_family_codes_by_attribute(p.ATTRIB_TEMPERATURE))
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_read_family_temperature(mqtt_capture, family_code) -> None:
    logger.info(f"Sending read request for attribute 'temperature' for device family {family_code}.")

    request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_READ,
            p.ATTRIB_FAMILY_CODE: family_code,
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_TEMPERATURE,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    TimeUtil.assert_timestamp(response.get(p.ATTRIB_TIME))
    assert response.get(p.ATTRIB_ACTION) == p.ACTION_READ
    response_devices = response.get(p.ATTRIB_DEVICES)
    assert response_devices is not None

    expected_devices = config.get_by_family_code(family_code)
    assert len(response_devices) == len(expected_devices)

    for expected_device in expected_devices:
        match = next((d for d in response_devices if d[p.ATTRIB_DEVICE_ID] == str(expected_device.device_id)), None)
        assert match is not None
        ow_dd.assert_temperature_range(match.get(p.ATTRIB_TEMPERATURE))


@pytest.mark.parametrize("family_code", ow_dd.get_supported_family_codes_by_attribute(p.ATTRIB_VAD))
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_read_family_vad(mqtt_capture, family_code) -> None:
    logger.info(f"Sending read request for attribute 'VAD' for device family {family_code}.")

    request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_READ,
            p.ATTRIB_FAMILY_CODE: family_code,
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_VAD,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    TimeUtil.assert_timestamp(response.get(p.ATTRIB_TIME))
    assert response.get(p.ATTRIB_ACTION) == p.ACTION_READ
    response_devices = response.get(p.ATTRIB_DEVICES)
    assert response_devices is not None

    expected_devices = config.get_by_family_code(family_code)
    assert len(response_devices) == len(expected_devices)

    for expected_device in expected_devices:
        match = next((d for d in response_devices if d[p.ATTRIB_DEVICE_ID] == str(expected_device.device_id)), None)
        assert match is not None
        ow_dd.assert_vad_range(match.get(p.ATTRIB_VAD))


@pytest.mark.parametrize("family_code", ow_dd.get_supported_family_codes_by_attribute(p.ATTRIB_VDD))
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_read_family_vdd(mqtt_capture, family_code) -> None:
    logger.info(f"Sending read request for attribute 'VDD' for device family {family_code}.")

    request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_READ,
            p.ATTRIB_FAMILY_CODE: family_code,
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_VDD,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    TimeUtil.assert_timestamp(response.get(p.ATTRIB_TIME))
    assert response.get(p.ATTRIB_ACTION) == p.ACTION_READ
    response_devices = response.get(p.ATTRIB_DEVICES)
    assert response_devices is not None

    expected_devices = config.get_by_family_code(family_code)
    assert len(response_devices) == len(expected_devices)

    for expected_device in expected_devices:
        match = next((d for d in response_devices if d[p.ATTRIB_DEVICE_ID] == str(expected_device.device_id)), None)
        assert match is not None
        ow_dd.assert_vad_range(match.get(p.ATTRIB_VDD))


@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_read_family_not_available(mqtt_capture) -> None:
    family_code = 99
    logger.info(f"Sending read request for device family {family_code}.")

    request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_READ,
            p.ATTRIB_FAMILY_CODE: family_code,
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_PRESENCE,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    TimeUtil.assert_timestamp(response.get(p.ATTRIB_TIME))
    assert response.get(p.ATTRIB_ACTION) == p.ACTION_READ
    response_devices = response.get(p.ATTRIB_DEVICES)
    assert response_devices is not None
    assert len(response_devices) == 0


@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_read_family_family_code_missing(mqtt_capture) -> None:
    logger.info("Sending read request for device family with missing family code.")

    request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_READ,
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_PRESENCE,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    TimeUtil.assert_timestamp(response.get(p.ATTRIB_TIME))
    error = response.get(p.ATTRIB_ERROR)
    assert error is not None
    assert error.get(p.ATTRIB_MESSAGE) == "Missing or invalid JSON attributes 'device_id' or 'family_code'."
    response_request = error.get(p.ATTRIB_REQUEST)
    assert response_request is not None
    assert response_request.get(p.ATTRIB_ACTION) == p.ACTION_READ
    assert response_request.get(p.ATTRIB_ATTRIBUTE) == p.ATTRIB_PRESENCE


@pytest.mark.parametrize("family_code", config.get_family_codes_from_devices())
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_read_family_attribute_missing(mqtt_capture, family_code) -> None:
    logger.info(f"Sending read request with missing attribute for device family {family_code}.")

    request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_READ,
            p.ATTRIB_FAMILY_CODE: family_code,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    TimeUtil.assert_timestamp(response.get(p.ATTRIB_TIME))
    error = response.get(p.ATTRIB_ERROR)
    assert error is not None
    assert error.get(p.ATTRIB_MESSAGE) == "Missing or invalid JSON attribute 'attribute'."
    response_request = error.get(p.ATTRIB_REQUEST)
    assert response_request is not None
    assert response_request.get(p.ATTRIB_ACTION) == p.ACTION_READ
    assert response_request.get(p.ATTRIB_FAMILY_CODE) == family_code
