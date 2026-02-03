import json
import time

import pytest

from tests.env.config_model import ConfigModel
from tests.env.logger import Logger
from tests.env.mqtt_fixture import mqtt_capture  # noqa: F401
from tests.env.mqtt_protocol import MqttProtocol as p

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

    # Wait until device rebooted
    wait_reboot_time = 10.0
    logger.info(f"Wait {wait_reboot_time}sec until device rebooted...")
    time.sleep(wait_reboot_time)

    # Check that device responds again
    request = json.dumps({p.ATTRIB_ACTION: p.ACTION_SCAN})
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages(clean_buffer=True)

    scan_response_msg = mqtt_capture.messages[0].as_json()
    assert scan_response_msg.get(p.ATTRIB_ACTION) == p.ACTION_SCAN
