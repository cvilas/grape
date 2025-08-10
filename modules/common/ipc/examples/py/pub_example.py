# =================================================================================================
# Copyright (C) 2025 GRAPE Contributors
# =================================================================================================

import time
from grape_ipc_py import init, Config, RawPublisher, ok


def to_bytes(msg: str) -> bytes:
    """Convert a string to bytes."""
    return msg.encode("utf-8")


def match_callback(match):
    """Callback for match/unmatch events."""
    print(f"Match status: {match.status}")


def main():
    try:
        # Initialize the IPC session
        config = Config()
        config.name = "Python IPC publisher example"
        init(config)

        # Define the topic
        topic = "hello_world"

        # Create the publisher
        publisher = RawPublisher(topic, match_callback)

        # Publish messages in a loop
        counter = 0
        SLEEP_TIME = 0.5  # 500 milliseconds
        while ok():
            counter += 1
            message = f"Hello World {counter}"
            print(f"Sending message: '{message}'")
            publisher.publish(to_bytes(message))
            time.sleep(SLEEP_TIME)

    except Exception as e:
        print(f"An error occurred: {e}")


if __name__ == "__main__":
    main()
