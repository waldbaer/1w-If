class MqttProtocol:
    """
    Defines constants for MQTT protocol field names and action values
    used in request/response messages.
    """

    # --- Common field names ---
    ATTRIB_ACTION = "action"
    ATTRIB_DEVICE = "device"
    ATTRIB_DEVICE_ID = "device_id"
    ATTRIB_CHANNEL = "channel"
    ATTRIB_FAMILY_CODE = "family_code"
    ATTRIB_ATTRIBUTE = "attribute"
    ATTRIB_ATTRIBUTES = "attributes"
    ATTRIB_INTERVAL = "interval"
    ATTRIB_PRESENCE = "presence"
    ATTRIB_TEMPERATURE = "temperature"
    ATTRIB_VAD = "VAD"
    ATTRIB_VDD = "VDD"
    ATTRIB_ACKNOWLEDGE = "acknowledge"
    ATTRIB_ERROR = "error"
    ATTRIB_MESSAGE = "message"
    ATTRIB_REQUEST = "request"
    ATTRIB_DEVICES = "devices"

    # --- Action types ---
    ACTION_RESTART = "restart"
    ACTION_SCAN = "scan"
    ACTION_READ = "read"
    ACTION_SUBSCRIBE = "subscribe"
    ACTION_UNSUBSCRIBE = "unsubscribe"

    @classmethod
    def is_valid_action(cls, action: str) -> bool:
        return action in cls.VALID_ACTIONS

    @classmethod
    def is_valid_attribute(cls, attribute: str) -> bool:
        return attribute in cls.VALID_ATTRIBUTES
