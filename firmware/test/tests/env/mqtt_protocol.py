class MqttProtocol:
    """
    Defines constants for MQTT protocol field names and action values
    used in request/response messages.
    """

    # ---- Common field names ----
    ATTRIB_STATE = "state"
    ATTRIB_TIME = "time"
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

    # ---- SysInfo attributes ----
    ATTRIB_SYSINFO_VERSION = "version"
    ATTRIB_SYSINFO_UPTIME = "uptime"
    ATTRIB_SYSINFO_BOARD_TEMP = "board_temp"
    ATTRIB_SYSINFO_ETHERNET = "ethernet"
    ATTRIB_SYSINFO_ETHERNET_IP = "ip"
    ATTRIB_SYSINFO_ETHERNET_MAC = "mac"
    ATTRIB_SYSINFO_ETHERNET_LINK_SPEED = "link_speed"
    ATTRIB_SYSINFO_ETHERNET_LINK_MODE = "link_mode"

    # ---- Action types ----
    ACTION_RESTART = "restart"
    ACTION_SYSINFO = "sysinfo"
    ACTION_SCAN = "scan"
    ACTION_READ = "read"
    ACTION_SUBSCRIBE = "subscribe"
    ACTION_UNSUBSCRIBE = "unsubscribe"

    # ---- Common attribute values ----
    VALUE_STATE_ONLINE = "online"
    VALUE_STATE_OFFLINE = "offline"

    # ---- SysInfo values ----
    VALUE_SYSINFO_ETHERNET_LINK_MODE_FULL_DUPLEX = "FULL_DUPLEX"
    VALUE_SYSINFO_ETHERNET_LINK_MODE_HALF_DUPLEX = "HALF_DUPLEX"

