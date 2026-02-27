from datetime import datetime


class TimeUtil:
    TIMESTAMP_FORMAT = "%Y-%m-%d %H:%M:%S.%f"
    DISABLE_MAX_DELTA_CHECK = -1
    MAX_DELTA_SECONDS_DEFAULT = 30

    @staticmethod
    def assert_timestamp(timestamp: str, max_delta_seconds_to_now = MAX_DELTA_SECONDS_DEFAULT) -> None:
        try:
            parsed_timestamp = datetime.strptime(timestamp, TimeUtil.TIMESTAMP_FORMAT)

            if max_delta_seconds_to_now >= 0:
                now = datetime.now()
                delta = abs((now - parsed_timestamp).total_seconds())
                assert delta <= max_delta_seconds_to_now, (
                    f"Timestamp outside tolerance: {timestamp} (Δ={delta:.2f}s, allowed ±{max_delta_seconds_to_now}sec)"
                )

        except ValueError as e:
            raise AssertionError(f"Invalid timestamp format: {timestamp}") from e


#
