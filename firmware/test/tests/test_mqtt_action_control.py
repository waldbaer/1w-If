import json
import time

import pytest
import re
import socket

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


@pytest.mark.mqtt_capture_data(config.mqtt)
def test_mqtt_protocol_sysinfo(mqtt_capture) -> None:
    logger.info("Send sysinfo request.")

    request = json.dumps({p.ATTRIB_ACTION: p.ACTION_SYSINFO})
    mqtt_capture.publish(config.mqtt.cmd_topic, request)

    mqtt_capture.wait_for_messages()
    response = mqtt_capture.messages[0].as_json()

    # Verify response
    assert response.get(p.ATTRIB_ACTION) == p.ACTION_SYSINFO
    assert response.get(p.ATTRIB_STATE) == p.VALUE_STATE_ONLINE
    assert len(response.get(p.ATTRIB_SYSINFO_VERSION)) >= len("1.0.0")
    assert re.compile(r"^(?:(\d+)d )?(\d{2}):(\d{2}):(\d{2})\.(\d{3})$").match(response.get(p.ATTRIB_SYSINFO_UPTIME))
    assert 0.00 <= response.get(p.ATTRIB_SYSINFO_BOARD_TEMP) <= 80.0
    TimeUtil.assert_timestamp(response.get(p.ATTRIB_TIME))

    eth_info = response.get(p.ATTRIB_SYSINFO_ETHERNET)
    socket.inet_aton(eth_info.get(p.ATTRIB_SYSINFO_ETHERNET_IP))
    assert re.compile(r"^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$").match(eth_info.get(p.ATTRIB_SYSINFO_ETHERNET_MAC))
    assert 10 <= eth_info.get(p.ATTRIB_SYSINFO_ETHERNET_LINK_SPEED) <= 1000
    assert eth_info.get(p.ATTRIB_SYSINFO_ETHERNET_LINK_MODE) in {p.VALUE_SYSINFO_ETHERNET_LINK_MODE_FULL_DUPLEX,
                                                                 p.VALUE_SYSINFO_ETHERNET_LINK_MODE_HALF_DUPLEX}

