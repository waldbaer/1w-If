from dataclasses import dataclass, field
from pathlib import Path
from typing import List

import yaml

from tests.env.one_wire_address import OneWireAddress
from tests.env.one_wire_device_def import OneWireDeviceDefinition


@dataclass
class MqttConfig:
    broker: str = "localhost"
    port: int = 1883
    username: str | None = None
    password: str | None = None
    cmd_topic: str = "1wIf/cmd"
    status_topic: str = "1wIf/stat"


@dataclass
class DeviceConfig:
    device_id: OneWireAddress


@dataclass
class ConfigModel:
    mqtt: MqttConfig
    devices: List[DeviceConfig] = field(default_factory=list)

    @staticmethod
    def load_from_yaml(
        file_path: str = Path(__file__).parent.parent.parent / "test_env_config.yaml",
    ) -> "ConfigModel":
        path = Path(file_path)
        if not path.exists():
            raise FileNotFoundError(f"TestEnvConfig config YAML not found: {file_path}")

        with open(path, encoding="utf-8") as f:
            data = yaml.safe_load(f)

        # MqttConfig
        mqtt_config = MqttConfig(
            broker=data.get("broker", "localhost"),
            port=data.get("port", 1883),
            username=data.get("username"),
            password=data.get("password"),
            cmd_topic=data.get("cmd_topic"),
            status_topic=data.get("status_topic"),
        )

        # Devices List
        devices_config = []
        for device_data in data.get("devices", []):
            device_id = OneWireAddress(device_data["device_id"])
            devices_config.append(DeviceConfig(device_id))

        return ConfigModel(mqtt=mqtt_config, devices=devices_config)

    def get_family_codes_from_devices(self) -> List[int]:
        family_codes = set()
        for device in self.devices:
            family_codes.add(device.device_id.get_family_code())
        return sorted(family_codes)

    def get_by_family_code(self, family_code: int) -> List[OneWireAddress]:
        return [device for device in self.devices if device.device_id.get_family_code() == family_code]

    def get_by_attribute(self, attribute: str) -> List[OneWireAddress]:
        filtered_devices = []
        for device in self.devices:
            device_attributes = OneWireDeviceDefinition.get_attributes(device.device_id)
            if attribute in device_attributes:
                filtered_devices.append(device)

        return filtered_devices
