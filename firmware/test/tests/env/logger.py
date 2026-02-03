import logging


class Logger(logging.LoggerAdapter):
    def process(self, msg, kwargs):  # noqa: ANN201
        return f"[TEST] {msg}", kwargs

    @staticmethod
    def get(name: str):  # noqa: ANN205
        return Logger(logging.getLogger(name))
