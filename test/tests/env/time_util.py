from datetime import datetime


class TimeUtil:
    TIMESTAMP_FORMAT = "%Y-%m-%d %H:%M:%S.%f"
    MAX_DELTA_SECONDS = 30

    @staticmethod
    def assert_timestamp(timestamp: str) -> None:
        try:
            parsed_timestamp = datetime.strptime(timestamp, TimeUtil.TIMESTAMP_FORMAT)

            now = datetime.now()
            delta = abs((now - parsed_timestamp).total_seconds())
            assert delta <= TimeUtil.MAX_DELTA_SECONDS, (
                f"Timestamp outside tolerance: {timestamp} (Δ={delta:.2f}s, allowed ±{TimeUtil.MAX_DELTA_SECONDS}sec)"
            )

        except ValueError as e:
            raise AssertionError(f"Invalid timestamp format: {timestamp}") from e


#
