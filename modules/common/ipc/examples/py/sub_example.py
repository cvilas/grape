# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

import time
from datetime import datetime
from grape_ipc_py import init, Config, RawSubscriber, ok


def from_bytes(data: bytes) -> str:
    """Convert bytes to a string."""
    return data.decode("utf-8")


def data_callback(sample):
    """Callback for received data."""
    message = from_bytes(sample.data())
    publish_time = sample.info.publish_time
    print(f"Received message: '{message}' at {publish_time}")


def match_callback(match):
    """Callback for match/unmatch events."""
    print(f"Match status: {match.status}")


def main():
    try:
        # Initialize the IPC session
        config = Config()
        config.name = "Python IPC subscriber example"
        init(config)

        # Define the topic
        topic = "hello_world"

        # Create the subscriber
        subscriber = RawSubscriber(topic, data_callback, match_callback)

        # Sleep loop to keep the subscriber running
        SLEEP_TIME = 0.5  # 500 milliseconds
        while ok():
            time.sleep(SLEEP_TIME)

    except Exception as e:
        print(f"An error occurred: {e}")


if __name__ == "__main__":
    main()
