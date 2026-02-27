import json
import time

import pytest

from tests.env.config_model import ConfigModel
from tests.env.logger import Logger
from tests.env.mqtt_fixture import mqtt_capture  # noqa: F401
from tests.env.mqtt_protocol import MqttProtocol as p
from tests.env.time_util import TimeUtil

# ---- Setup Test Environment ------------------------------------------------------------------------------------------
config = ConfigModel.load_from_yaml()
logger = Logger.get(__name__)

# ---- Test Implementation ---------------------------------------------------------------------------------------------


@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_restart(mqtt_capture) -> None:
    logger.info("Send restart request to 1-Wire Interface.")

    request = json.dumps({p.ATTRIB_ACTION: p.ACTION_RESTART})
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages(clean_buffer=True)
    ack_msg = mqtt_capture.messages[0].as_json()
    assert ack_msg.get(p.ATTRIB_ACTION) == p.ACTION_RESTART
    assert ack_msg.get(p.ATTRIB_ACKNOWLEDGE) is True

    # Waiting now for LWT offline and LWT online messages
    wait_reboot_time = 10.0
    mqtt_capture.wait_for_messages(expected_number=2, clean_buffer=True, timeout=2 * wait_reboot_time)

    # Wait until device rebooted
    logger.info(f"Wait {wait_reboot_time}sec until device rebooted...")
    time.sleep(wait_reboot_time)

    # Check LWT offline + online messages
    assert len(mqtt_capture.messages) == 2  # LWT offline + LWT online
    lwt_offline_msg = mqtt_capture.messages[0].as_json()
    lwt_online_msg = mqtt_capture.messages[1].as_json()

    assert lwt_offline_msg.get(p.ATTRIB_STATE) == p.VALUE_STATE_OFFLINE
    assert lwt_online_msg.get(p.ATTRIB_STATE) == p.VALUE_STATE_ONLINE

    # Timestamps are initial MQTT topic subscription times. Not the current time.
    TimeUtil.assert_timestamp(lwt_online_msg.get(p.ATTRIB_TIME), TimeUtil.DISABLE_MAX_DELTA_CHECK)
    TimeUtil.assert_timestamp(lwt_offline_msg.get(p.ATTRIB_TIME), TimeUtil.DISABLE_MAX_DELTA_CHECK)

    # Check that device responds again
    request = json.dumps({p.ATTRIB_ACTION: p.ACTION_SCAN})
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages(clean_buffer=True)

    scan_response_msg = mqtt_capture.messages[0].as_json()
    assert scan_response_msg.get(p.ATTRIB_ACTION) == p.ACTION_SCAN
