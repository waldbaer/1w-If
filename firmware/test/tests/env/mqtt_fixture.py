import logging
import threading
import time
import typing as t

import paho.mqtt.client as mqtt
import pytest

from tests.env.mqtt_message import MqttMessage
from tests.env.mqtt_protocol import MqttProtocol as p
from tests.env.time_util import TimeUtil

logger = logging.getLogger(__name__)


class MqttClientAdapter(threading.Thread):
    def __init__(
        self,
        on_message_callback: t.Callable | None = None,
        host: str = "localhost",
        port: int = 1883,
        username: str | None = None,
        password: str | None = None,
        topics: t.List[str] = None,
    ) -> None:
        if topics is None:
            topics = []
        super().__init__()
        self.client: mqtt.Client

        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

        self.on_message_callback = on_message_callback
        self.host = host
        self.port = int(port)
        self.username = username
        self.password = password
        self.topics = topics

        self.setup()

    def setup(self):
        client = self.client
        client.on_socket_open = self.on_socket_open
        client.on_connect = self.on_connect
        client.on_subscribe = self.on_subscribe
        client.on_message = self.on_message
        if self.on_message_callback:
            client.on_message = self.on_message_callback
        if self.username or self.password:
            client.username_pw_set(self.username, self.password)

        logger.debug("[MQTT] Connecting to MQTT broker")
        client.connect(host=self.host, port=self.port)

        # [ (topic_name, qos), ... ]
        client.subscribe([(topic, 0) for topic in self.topics])

    def run(self):
        self.client.loop_start()

    def stop(self):
        logger.debug("[MQTT] Disconnecting from MQTT broker")
        self.client.disconnect()
        self.client.loop_stop()

    def on_socket_open(self, client, userdata, sock):  # noqa: ARG002
        logger.debug("[MQTT] Opened socket to MQTT broker")

    def on_connect(self, client, userdata, flags, reason_code, properties):  # noqa: ARG002
        logger.debug("[MQTT] Connected to MQTT broker")

    def on_subscribe(self, client, userdata, mid, reason_codes, properties):  # noqa: ARG002
        logger.debug("[MQTT] Subscribed to MQTT topic(s)")

    def on_message(self, client, userdata, msg):  # noqa: ARG002
        logger.debug("[MQTT] Message received: %s", msg)

    def publish(self, topic: str, payload: str, **kwargs) -> mqtt.MQTTMessageInfo:  # noqa: ANN003
        logger.debug(f"[MQTT] Publish to topic {topic}: {payload}")

        message_info = self.client.publish(topic, payload, **kwargs)
        message_info.wait_for_publish()
        return message_info


class MqttCaptureFixture:
    """Provides access and control of log capturing."""

    def __init__(
        self,
        decode_utf8: bool = False,
        host: str = "localhost",
        port: int = 1883,
        username: str | None = None,
        password: str | None = None,
        topics: t.List[str] = None,
    ) -> None:
        self._buffer: t.List[MqttMessage] = []
        self._decode_utf8: bool = decode_utf8 or False

        self.mqtt_client = MqttClientAdapter(
            on_message_callback=self.on_message,
            host=host,
            port=port,
            username=username,
            password=password,
            topics=topics,
        )
        self.mqtt_client.start()

    def on_message(self, client, userdata, msg):  # noqa: ARG002
        payload = msg.payload
        if self._decode_utf8:
            payload = payload.decode("utf-8")

        message = MqttMessage(topic=msg.topic, payload=payload, userdata=userdata)
        self._buffer.append(message)
        logger.debug(f"[MQTT] Message received: {message}")

    def finalize(self) -> None:
        """Finalizes the fixture."""
        self.mqtt_client.stop()
        self.mqtt_client.join(timeout=4.2)
        self._buffer = []

    @property
    def messages(self) -> t.List[MqttMessage]:
        return self._buffer

    def publish(self, topic: str, payload: str, **kwargs: str) -> mqtt.MQTTMessageInfo:
        message_info = self.mqtt_client.publish(topic=topic, payload=payload, **kwargs)
        return message_info

    def wait_for_messages(self, expected_number: int = 1, clean_buffer: bool = False, timeout: int = 5.0):
        if clean_buffer:
            self._buffer.clear()

        start = time.time()
        while time.time() - start < timeout:
            if len(self._buffer) >= expected_number:
                return
            else:
                time.sleep(0.05)
        pytest.fail(f"Timeout: timeout expired while waiting for reception of {expected_number} MQTT message(s)")


@pytest.fixture(scope="function")
def mqtt_capture(request: pytest.FixtureRequest) -> MqttCaptureFixture:
    mqtt_config_marker = request.node.get_closest_marker("mqtt_capture_data")
    assert mqtt_config_marker is not None
    mqtt_config = mqtt_config_marker.args[0]

    mqtt_capture = MqttCaptureFixture(
        decode_utf8=False,
        host=mqtt_config.broker,
        port=mqtt_config.port,
        username=mqtt_config.username,
        password=mqtt_config.password,
        topics=[mqtt_config.status_topic],
    )

    time.sleep(0.05)

    # Wait for LWT online sent immediately after subscription to status topic
    mqtt_capture.wait_for_messages()
    assert len(mqtt_capture.messages) == 1
    lwt_online_msg = mqtt_capture.messages[0].as_json()
    TimeUtil.assert_timestamp(lwt_online_msg.get(p.ATTRIB_TIME), TimeUtil.DISABLE_MAX_DELTA_CHECK)
    mqtt_capture.messages.clear()

    yield mqtt_capture
    mqtt_capture.finalize()
