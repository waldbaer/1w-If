import json


class MqttMessage:
    """
    Represents a single MQTT message with topic and payload.
    Provides helper functions to validate and parse JSON payloads.
    """

    def __init__(self, topic: str, payload: str, userdata: str = None) -> None:
        self.topic = topic
        self.payload = payload
        self.userdata = userdata

    def is_json(self) -> bool:
        try:
            json.loads(self.payload)
            return True
        except (json.JSONDecodeError, TypeError):
            return False

    def as_json(self) -> dict:
        if not self.is_json():
            raise ValueError(f"Payload is not valid JSON: {self.payload}")
        return json.loads(self.payload)

    def get_userdata(self) -> str:
        return self.userdata

    def __repr__(self) -> str:
        return f"[{self.topic}] {self.payload}"
