import json

import pytest

from tests.env.config_model import ConfigModel
from tests.env.logger import Logger
from tests.env.mqtt_fixture import mqtt_capture  # noqa: F401
from tests.env.mqtt_protocol import MqttProtocol as p
from tests.env.one_wire_device_def import OneWireDeviceDefinition as ow_dd
from tests.env.time_util import TimeUtil

# ---- Setup Test Environment ------------------------------------------------------------------------------------------
config = ConfigModel.load_from_yaml()
logger = Logger.get(__name__)

# ---- Test Implementation ---------------------------------------------------------------------------------------------


@pytest.mark.parametrize("device", config.devices)
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_subscription_single_device_presence(mqtt_capture, device) -> None:
    logger.info(f"Subscribe / Unsubscribe to single device attribute 'presence' of device {device.device_id}.")

    interval_ms = 1000
    interval_sec = interval_ms / 1000

    subscribe_request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_SUBSCRIBE,
            p.ATTRIB_DEVICE_ID: str(device.device_id),
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_PRESENCE,
            p.ATTRIB_INTERVAL: interval_ms,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, subscribe_request)

    expected_cyclic_reads = 4
    expected_total_messages = expected_cyclic_reads + 2  # +2: subscribe ack + immediate read
    mqtt_capture.wait_for_messages(
        expected_number=expected_total_messages,
        # first read takes between 100-500ms
        timeout=500 / 1000 + ((expected_cyclic_reads + 0.5) * (interval_sec)),
    )

    assert len(mqtt_capture.messages) == expected_total_messages

    subscribe_ack_msg = mqtt_capture.messages[0].as_json()
    cyclic_read_msgs = [mqtt_message.as_json() for mqtt_message in mqtt_capture.messages[-expected_cyclic_reads:]]
    assert len(cyclic_read_msgs) == expected_cyclic_reads

    # Verify subscribe ack and read responses
    TimeUtil.assert_timestamp(subscribe_ack_msg.get(p.ATTRIB_TIME))
    assert subscribe_ack_msg.get(p.ATTRIB_ACTION) == p.ACTION_SUBSCRIBE
    assert subscribe_ack_msg.get(p.ATTRIB_ACKNOWLEDGE) is True

    response_device = subscribe_ack_msg.get(p.ATTRIB_DEVICE)
    assert response_device is not None
    assert response_device.get(p.ATTRIB_CHANNEL) is None
    assert response_device.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)

    for cyclic_read_msg in cyclic_read_msgs:
        TimeUtil.assert_timestamp(cyclic_read_msg.get(p.ATTRIB_TIME))
        assert cyclic_read_msg.get(p.ATTRIB_ACTION) == p.ACTION_READ
        response_device = cyclic_read_msg.get(p.ATTRIB_DEVICE)
        assert response_device is not None
        assert response_device.get(p.ATTRIB_CHANNEL) == device.channel
        assert response_device.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)
        assert response_device.get(p.ATTRIB_PRESENCE) is True

    # Unsubscribe
    unsubscribe_request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_UNSUBSCRIBE,
            p.ATTRIB_DEVICE_ID: str(device.device_id),
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_PRESENCE,
        }
    )

    mqtt_capture.publish(config.mqtt.cmd_topic, unsubscribe_request)

    mqtt_capture.wait_for_messages(clean_buffer=True)
    unsubscribe_ack_msg = mqtt_capture.messages[0].as_json()

    # Verify unsubscribe ack response
    TimeUtil.assert_timestamp(unsubscribe_ack_msg.get(p.ATTRIB_TIME))
    assert unsubscribe_ack_msg.get(p.ATTRIB_ACTION) == p.ACTION_UNSUBSCRIBE
    assert unsubscribe_ack_msg.get(p.ATTRIB_ACKNOWLEDGE) is True
    response_device = unsubscribe_ack_msg.get(p.ATTRIB_DEVICE)
    assert response_device is not None
    assert response_device.get(p.ATTRIB_CHANNEL) is None
    assert response_device.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)


@pytest.mark.parametrize("device", config.get_by_attribute(p.ATTRIB_TEMPERATURE))
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_subscription_single_device_temperature(mqtt_capture, device) -> None:
    logger.info(f"Subscribe / Unsubscribe to single device attribute 'temperature' of device {device.device_id}.")

    interval_ms = 3000
    interval_sec = interval_ms / 1000

    subscribe_request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_SUBSCRIBE,
            p.ATTRIB_DEVICE_ID: str(device.device_id),
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_TEMPERATURE,
            p.ATTRIB_INTERVAL: interval_ms,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, subscribe_request)

    expected_cyclic_reads = 1
    expected_total_messages = expected_cyclic_reads + 2  # +2: subscribe ack + immediate read
    mqtt_capture.wait_for_messages(
        expected_number=expected_total_messages,
        timeout=interval_sec + 1.0,  # +1.0 for temp. sampling time
    )

    assert len(mqtt_capture.messages) == expected_total_messages

    subscribe_ack_msg = mqtt_capture.messages[0].as_json()
    cyclic_read_msgs = [mqtt_message.as_json() for mqtt_message in mqtt_capture.messages[-expected_cyclic_reads:]]
    assert len(cyclic_read_msgs) == expected_cyclic_reads

    # Verify subscribe ack and read responses
    TimeUtil.assert_timestamp(subscribe_ack_msg.get(p.ATTRIB_TIME))
    assert subscribe_ack_msg.get(p.ATTRIB_ACTION) == p.ACTION_SUBSCRIBE
    assert subscribe_ack_msg.get(p.ATTRIB_ACKNOWLEDGE) is True
    response_device = subscribe_ack_msg.get(p.ATTRIB_DEVICE)
    assert response_device is not None
    assert response_device.get(p.ATTRIB_CHANNEL) is None
    assert response_device.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)

    for cyclic_read_msg in cyclic_read_msgs:
        TimeUtil.assert_timestamp(cyclic_read_msg.get(p.ATTRIB_TIME))
        assert cyclic_read_msg.get(p.ATTRIB_ACTION) == p.ACTION_READ
        response_device = cyclic_read_msg.get(p.ATTRIB_DEVICE)
        assert response_device is not None
        assert response_device.get(p.ATTRIB_CHANNEL) == device.channel
        assert response_device.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)
        ow_dd.assert_temperature_range(response_device.get(p.ATTRIB_TEMPERATURE))

    # Unsubscribe
    unsubscribe_request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_UNSUBSCRIBE,
            p.ATTRIB_DEVICE_ID: str(device.device_id),
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_TEMPERATURE,
        }
    )

    mqtt_capture.publish(config.mqtt.cmd_topic, unsubscribe_request)

    mqtt_capture.wait_for_messages(clean_buffer=True)
    unsubscribe_ack_msg = mqtt_capture.messages[0].as_json()

    # Verify unsubscribe ack response
    TimeUtil.assert_timestamp(unsubscribe_ack_msg.get(p.ATTRIB_TIME))
    assert unsubscribe_ack_msg.get(p.ATTRIB_ACTION) == p.ACTION_UNSUBSCRIBE
    assert unsubscribe_ack_msg.get(p.ATTRIB_ACKNOWLEDGE) is True
    response_device = unsubscribe_ack_msg.get(p.ATTRIB_DEVICE)
    assert response_device is not None
    assert response_device.get(p.ATTRIB_CHANNEL) is None
    assert response_device.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)


@pytest.mark.parametrize("device", config.devices)
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_subscription_single_device_already_subscribed(mqtt_capture, device) -> None:
    logger.info(f"Subscribe twice to single device attribute 'presence' of device {device.device_id}.")

    interval_ms = 1000

    subscribe_request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_SUBSCRIBE,
            p.ATTRIB_DEVICE_ID: str(device.device_id),
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_PRESENCE,
            p.ATTRIB_INTERVAL: interval_ms,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, subscribe_request)

    mqtt_capture.wait_for_messages(expected_number=2)

    subscribe_ack_msg = mqtt_capture.messages[0].as_json()
    immediate_read_msg = mqtt_capture.messages[1].as_json()

    # Verify subscribe ack and immediate read response
    TimeUtil.assert_timestamp(subscribe_ack_msg.get(p.ATTRIB_TIME))
    assert subscribe_ack_msg.get(p.ATTRIB_ACTION) == p.ACTION_SUBSCRIBE
    assert subscribe_ack_msg.get(p.ATTRIB_ACKNOWLEDGE) is True
    response_device = subscribe_ack_msg.get(p.ATTRIB_DEVICE)
    assert response_device is not None
    assert response_device.get(p.ATTRIB_CHANNEL) is None
    assert response_device.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)

    TimeUtil.assert_timestamp(immediate_read_msg.get(p.ATTRIB_TIME))
    assert immediate_read_msg.get(p.ATTRIB_ACTION) == p.ACTION_READ
    response_device = immediate_read_msg.get(p.ATTRIB_DEVICE)
    assert response_device is not None
    assert response_device.get(p.ATTRIB_CHANNEL) == device.channel
    assert response_device.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)

    # Repeat subscription
    mqtt_capture.publish(config.mqtt.cmd_topic, subscribe_request)
    mqtt_capture.wait_for_messages(clean_buffer=1)

    # Verify error response
    error_response_msg = mqtt_capture.messages[0].as_json()

    TimeUtil.assert_timestamp(error_response_msg.get(p.ATTRIB_TIME))
    error = error_response_msg.get(p.ATTRIB_ERROR)
    assert error is not None
    assert error.get(p.ATTRIB_MESSAGE) == "WARN: Already subscribed to device / attribute. Updating subscription."
    assert error.get(p.ATTRIB_REQUEST) is None

    # Unsubscribe
    unsubscribe_request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_UNSUBSCRIBE,
            p.ATTRIB_DEVICE_ID: str(device.device_id),
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_PRESENCE,
        }
    )

    mqtt_capture.publish(config.mqtt.cmd_topic, unsubscribe_request)

    mqtt_capture.wait_for_messages(clean_buffer=True)
    unsubscribe_ack_msg = mqtt_capture.messages[0].as_json()

    # Verify unsubscribe ack response
    TimeUtil.assert_timestamp(unsubscribe_ack_msg.get(p.ATTRIB_TIME))
    assert unsubscribe_ack_msg.get(p.ATTRIB_ACTION) == p.ACTION_UNSUBSCRIBE
    assert unsubscribe_ack_msg.get(p.ATTRIB_ACKNOWLEDGE) is True
    response_device = unsubscribe_ack_msg.get(p.ATTRIB_DEVICE)
    assert response_device is not None
    assert response_device.get(p.ATTRIB_CHANNEL) is None
    assert response_device.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)


@pytest.mark.parametrize("device", config.devices)
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_subscription_single_device_not_subscribed(mqtt_capture, device) -> None:
    logger.info(f"Unsubscribe from not subscribed device attribute 'presence' of device {device.device_id}.")

    subscribe_request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_UNSUBSCRIBE,
            p.ATTRIB_DEVICE_ID: str(device.device_id),
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_PRESENCE,
            p.ATTRIB_INTERVAL: 1000,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, subscribe_request)

    mqtt_capture.wait_for_messages()

    # Verify error response
    error_response_msg = mqtt_capture.messages[0].as_json()

    TimeUtil.assert_timestamp(error_response_msg.get(p.ATTRIB_TIME))
    error = error_response_msg.get(p.ATTRIB_ERROR)
    assert error is not None
    assert error.get(p.ATTRIB_MESSAGE) == "WARN: No subscription for requested device / attribute found."
    assert error.get(p.ATTRIB_REQUEST) is None


@pytest.mark.parametrize("device", config.devices)
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_subscription_single_device_missing_interval(mqtt_capture, device) -> None:
    logger.info(f"Subscribe with missing interval to device {device.device_id}.")

    subscribe_request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_SUBSCRIBE,
            p.ATTRIB_DEVICE_ID: str(device.device_id),
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_PRESENCE,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, subscribe_request)

    mqtt_capture.wait_for_messages()

    # Verify error response
    error_response_msg = mqtt_capture.messages[0].as_json()

    TimeUtil.assert_timestamp(error_response_msg.get(p.ATTRIB_TIME))
    error = error_response_msg.get(p.ATTRIB_ERROR)
    assert error is not None
    assert error.get(p.ATTRIB_MESSAGE) == "Missing or invalid JSON attributes 'attribute' or 'interval'."
    error_request = error.get(p.ATTRIB_REQUEST)
    assert error_request is not None
    assert error_request.get(p.ATTRIB_ACTION) == p.ACTION_SUBSCRIBE
    assert error_request.get(p.ATTRIB_CHANNEL) is None
    assert error_request.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)
    assert error_request.get(p.ATTRIB_ATTRIBUTE) == p.ATTRIB_PRESENCE
    assert error_request.get(p.ATTRIB_INTERVAL) is None


@pytest.mark.parametrize("device", config.devices)
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_subscription_single_device_invalid_attribute(mqtt_capture, device) -> None:
    unknown_attribute = "UNKNOWN_ATTRIBUTE"
    interval = 1000

    logger.info(f"Subscribe with invalid attribute to device {device.device_id}.")

    subscribe_request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_SUBSCRIBE,
            p.ATTRIB_DEVICE_ID: str(device.device_id),
            p.ATTRIB_ATTRIBUTE: unknown_attribute,
            p.ATTRIB_INTERVAL: interval,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, subscribe_request)

    mqtt_capture.wait_for_messages()

    # Verify error response
    error_response_msg = mqtt_capture.messages[0].as_json()

    TimeUtil.assert_timestamp(error_response_msg.get(p.ATTRIB_TIME))
    error = error_response_msg.get(p.ATTRIB_ERROR)
    assert error is not None
    assert error.get(p.ATTRIB_MESSAGE) == "Missing or invalid JSON attributes 'attribute' or 'interval'."
    error_request = error.get(p.ATTRIB_REQUEST)
    assert error_request is not None
    assert error_request.get(p.ATTRIB_ACTION) == p.ACTION_SUBSCRIBE
    assert error_request.get(p.ATTRIB_CHANNEL) is None
    assert error_request.get(p.ATTRIB_DEVICE_ID) == str(device.device_id)
    assert error_request.get(p.ATTRIB_ATTRIBUTE) == unknown_attribute
    assert error_request.get(p.ATTRIB_INTERVAL) == interval


@pytest.mark.parametrize("family_code", config.get_family_codes_from_devices())
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_subscription_family_presence(mqtt_capture, family_code) -> None:
    logger.info(f"Subscribe / Unsubscribe to attribute 'presence' of family {family_code}.")

    interval_ms = 1000
    interval_sec = interval_ms / 1000

    subscribe_request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_SUBSCRIBE,
            p.ATTRIB_FAMILY_CODE: family_code,
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_PRESENCE,
            p.ATTRIB_INTERVAL: interval_ms,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, subscribe_request)

    expected_cyclic_reads = 4
    expected_total_messages = expected_cyclic_reads + 2  # +2: subscribe ack + immediate read
    mqtt_capture.wait_for_messages(
        expected_number=expected_total_messages,
        # first read takes between 100-300ms
        timeout=300 / 1000 + ((expected_cyclic_reads + 0.5) * (interval_sec)),
    )

    assert len(mqtt_capture.messages) == expected_total_messages

    subscribe_ack_msg = mqtt_capture.messages[0].as_json()
    cyclic_read_msgs = [mqtt_message.as_json() for mqtt_message in mqtt_capture.messages[-expected_cyclic_reads:]]
    assert len(cyclic_read_msgs) == expected_cyclic_reads

    # Verify subscribe ack and read responses
    TimeUtil.assert_timestamp(subscribe_ack_msg.get(p.ATTRIB_TIME))
    assert subscribe_ack_msg.get(p.ATTRIB_ACTION) == p.ACTION_SUBSCRIBE
    assert subscribe_ack_msg.get(p.ATTRIB_CHANNEL) is None
    assert subscribe_ack_msg.get(p.ATTRIB_ACKNOWLEDGE) is True
    assert subscribe_ack_msg.get(p.ATTRIB_FAMILY_CODE) == family_code

    expected_devices = config.get_by_family_code(family_code)

    for cyclic_read_msg in cyclic_read_msgs:
        TimeUtil.assert_timestamp(cyclic_read_msg.get(p.ATTRIB_TIME))
        assert cyclic_read_msg.get(p.ATTRIB_ACTION) == p.ACTION_READ
        response_devices = cyclic_read_msg.get(p.ATTRIB_DEVICES)
        assert response_devices is not None

        assert len(response_devices) == len(expected_devices)
        for expected_device in expected_devices:
            match = next((d for d in response_devices if d[p.ATTRIB_DEVICE_ID] == str(expected_device.device_id)), None)
            assert match is not None
            assert match.get(p.ATTRIB_CHANNEL) == expected_device.channel
            assert match.get(p.ATTRIB_PRESENCE) is True

    # Unsubscribe
    unsubscribe_request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_UNSUBSCRIBE,
            p.ATTRIB_FAMILY_CODE: family_code,
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_PRESENCE,
        }
    )

    mqtt_capture.publish(config.mqtt.cmd_topic, unsubscribe_request)

    mqtt_capture.wait_for_messages(clean_buffer=True)
    unsubscribe_ack_msg = mqtt_capture.messages[0].as_json()

    # Verify unsubscribe ack response
    TimeUtil.assert_timestamp(unsubscribe_ack_msg.get(p.ATTRIB_TIME))
    assert unsubscribe_ack_msg.get(p.ATTRIB_ACTION) == p.ACTION_UNSUBSCRIBE
    assert unsubscribe_ack_msg.get(p.ATTRIB_CHANNEL) is None
    assert unsubscribe_ack_msg.get(p.ATTRIB_ACKNOWLEDGE) is True
    assert unsubscribe_ack_msg.get(p.ATTRIB_FAMILY_CODE) == family_code


@pytest.mark.parametrize("family_code", config.get_family_codes_from_devices())
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_subscription_family_not_subscribed(mqtt_capture, family_code) -> None:
    logger.info(f"Unsubscribe from not subscribed device attribute 'presence' of family {family_code}.")

    subscribe_request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_UNSUBSCRIBE,
            p.ATTRIB_FAMILY_CODE: family_code,
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_PRESENCE,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, subscribe_request)

    mqtt_capture.wait_for_messages()

    # Verify error response
    error_response_msg = mqtt_capture.messages[0].as_json()

    TimeUtil.assert_timestamp(error_response_msg.get(p.ATTRIB_TIME))
    error = error_response_msg.get(p.ATTRIB_ERROR)
    assert error is not None
    assert error.get(p.ATTRIB_MESSAGE) == "WARN: No subscription for requested device family found."
    assert error.get(p.ATTRIB_REQUEST) is None


@pytest.mark.parametrize("family_code", config.get_family_codes_from_devices())
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_subscription_family_missing_interval(mqtt_capture, family_code) -> None:
    logger.info(f"Subscribe / Unsubscribe with missing interval of family {family_code}.")

    subscribe_request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_SUBSCRIBE,
            p.ATTRIB_FAMILY_CODE: family_code,
            p.ATTRIB_ATTRIBUTE: p.ATTRIB_PRESENCE,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, subscribe_request)

    mqtt_capture.wait_for_messages()

    # Verify error response
    error_response_msg = mqtt_capture.messages[0].as_json()

    TimeUtil.assert_timestamp(error_response_msg.get(p.ATTRIB_TIME))
    error = error_response_msg.get(p.ATTRIB_ERROR)
    assert error is not None
    assert error.get(p.ATTRIB_MESSAGE) == "Missing or invalid JSON attributes 'attribute' or 'interval'."
    error_request = error.get(p.ATTRIB_REQUEST)
    assert error_request is not None
    assert error_request.get(p.ATTRIB_ACTION) == p.ACTION_SUBSCRIBE
    assert error_request.get(p.ATTRIB_ATTRIBUTE) == p.ATTRIB_PRESENCE
    assert error_request.get(p.ATTRIB_CHANNEL) is None
    assert error_request.get(p.ATTRIB_FAMILY_CODE) == family_code
    assert error_request.get(p.ATTRIB_INTERVAL) is None


@pytest.mark.parametrize("family_code", config.get_family_codes_from_devices())
@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_subscription_family_invalid_attribute(mqtt_capture, family_code) -> None:
    unknown_attribute = "UNKNOWN_ATTRIBUTE"
    interval = 1000

    logger.info(f"Subscribe / Unsubscribe to/from unknown attribute of family {family_code}.")

    subscribe_request = json.dumps(
        {
            p.ATTRIB_ACTION: p.ACTION_SUBSCRIBE,
            p.ATTRIB_FAMILY_CODE: family_code,
            p.ATTRIB_ATTRIBUTE: unknown_attribute,
            p.ATTRIB_INTERVAL: interval,
        }
    )
    mqtt_capture.publish(config.mqtt.cmd_topic, subscribe_request)

    mqtt_capture.wait_for_messages()

    # Verify error response
    error_response_msg = mqtt_capture.messages[0].as_json()

    TimeUtil.assert_timestamp(error_response_msg.get(p.ATTRIB_TIME))
    error = error_response_msg.get(p.ATTRIB_ERROR)
    assert error is not None
    assert error.get(p.ATTRIB_MESSAGE) == "Missing or invalid JSON attributes 'attribute' or 'interval'."
    error_request = error.get(p.ATTRIB_REQUEST)
    assert error_request is not None
    assert error_request.get(p.ATTRIB_ACTION) == p.ACTION_SUBSCRIBE
    assert error_request.get(p.ATTRIB_CHANNEL) is None
    assert error_request.get(p.ATTRIB_ATTRIBUTE) == unknown_attribute
    assert error_request.get(p.ATTRIB_FAMILY_CODE) == family_code
    assert error_request.get(p.ATTRIB_INTERVAL) == interval
