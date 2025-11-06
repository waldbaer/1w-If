class OneWireAddress:
    """
    Utility class for handling 1-Wire device IDs.
    Example DeviceID: "28.FF45161301"
    """

    def __init__(self, device_id: str) -> None:
        if not device_id or "." not in device_id:
            raise ValueError(f"Invalid device_id format: {device_id}")
        self.device_id = device_id.upper()

    def get_family_code(self) -> int:
        hex_code = self.device_id.split(".")[0]
        return int(hex_code, 16)

    def __str__(self) -> str:
        return self.device_id

    def __repr__(self) -> str:
        return f"OneWireAddress('{self.device_id}')"
