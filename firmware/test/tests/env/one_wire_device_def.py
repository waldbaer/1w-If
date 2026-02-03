from typing import List

from tests.env.one_wire_address import OneWireAddress


class OneWireDeviceDefinition:
    # Static mapping: family_code (decimal) → supported attributes
    _FAMILY_ATTRIBUTES = {
        0x01: ["presence"],
        0x26: ["presence", "temperature", "VAD", "VDD"],
        0x28: ["presence", "temperature"],
    }

    @staticmethod
    def get_attributes(device_id: OneWireAddress) -> List[str]:
        family_code = device_id.get_family_code()

        return OneWireDeviceDefinition._FAMILY_ATTRIBUTES[family_code]

    @staticmethod
    def get_supported_family_codes() -> List[int]:
        """Return a list of all known family codes."""
        return list(OneWireDeviceDefinition._FAMILY_ATTRIBUTES.keys())

    @staticmethod
    def get_supported_family_codes_by_attribute(attribute: str) -> List[int]:
        """Return a list of family codes supporting the requested attribute."""
        return [
            family_id for family_id, attrs in OneWireDeviceDefinition._FAMILY_ATTRIBUTES.items() if attribute in attrs
        ]

    @staticmethod
    def assert_temperature_range(temperature: float) -> None:
        assert 0.0 <= temperature <= 50.0

    @staticmethod
    def assert_vad_range(vad: float) -> None:
        assert 0.0 <= vad <= 5.2

    @staticmethod
    def assert_vdd_range(vdd: float) -> None:
        assert 0.0 <= vdd <= 5.2
