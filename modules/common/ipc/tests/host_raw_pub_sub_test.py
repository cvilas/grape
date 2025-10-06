#!/usr/bin/env python3
#=================================================================================================
# Copyright (C) 2025 GRAPE Contributors
#=================================================================================================

"""
Python equivalent of host_raw_pub_sub_test.cpp using the grape_ipc_py bindings.
Tests basic pub-sub functionality with large messages in host-only scope.
"""

import time
import random
import threading
import sys
from typing import List, Optional

try:
    import grape_ipc_py as ipc
except ImportError:
    print("ERROR: Failed to import grape_ipc_py module. Make sure it's installed or in your PYTHONPATH.")
    sys.exit(1)

def test_basic_pub_sub_large_message():
    """Test that basic pub-sub on large message works in host-only scope."""
    
    # Initialize the IPC session
    ipc.init(ipc.Config())
    
    # Topic name
    topic = "pub_sub_py_test"
    
    # Create a large payload (e.g., 1080p RGB image)
    PAYLOAD_SIZE = 1920 * 1080 * 3
    # Generate random bytes directly
    payload = bytes(random.randint(0, 255) for _ in range(PAYLOAD_SIZE))
    
    # Create synchronization event for test
    data_received = threading.Event()
    
    # Storage for received data
    received_msg = bytes()
    pub_id = 0
    
    # Define subscriber callback
    def recv_callback(sample):
        nonlocal received_msg, pub_id
        received_msg = sample.data()  
        pub_id = sample.info.publisher.id
        data_received.set()
    
    qos = ipc.QoS.BestEffort
    
    # Define match callbacks
    matched_sub_id = 0
    def pub_match_cb(match):
        nonlocal matched_sub_id
        matched_sub_id = match.remote_entity.id
    
    matched_pub_id = 0
    def sub_match_cb(match):
        nonlocal matched_pub_id
        matched_pub_id = match.remote_entity.id
    
    # Create publisher and subscriber
    publisher = ipc.RawPublisher(topic, pub_match_cb)
    subscriber = ipc.RawSubscriber(topic, qos, recv_callback, sub_match_cb)
    
    # Wait for pub/sub registration
    RETRY_COUNT = 10
    count_down = RETRY_COUNT
    while subscriber.get_publisher_count() == 0 and count_down > 0:
        time.sleep(0.2)  # 200 milliseconds
        count_down -= 1
    
    # Assert matched publishers/subscribers
    assert subscriber.get_publisher_count() == 1, "Expected 1 publisher to be matched"
    assert matched_sub_id != 0, "Expected non-zero subscriber ID"
    assert matched_pub_id != 0, "Expected non-zero publisher ID"
    
    # Publish payload
    pub_result = publisher.publish(bytes(payload))
    assert pub_result, "Failed to publish message"
    
    # Wait a reasonable time for subscriber to receive message
    success = data_received.wait(1.0)  # Wait up to 1 second
    assert success, "Timeout waiting for data to be received"
    
    # Verify message
    assert len(received_msg) == PAYLOAD_SIZE, f"Expected {PAYLOAD_SIZE} bytes, got {len(received_msg)}"
    assert received_msg == payload, "Received payload does not match sent payload"
    assert pub_id != 0, "Expected non-zero publisher ID in received sample"
    
def main():
    """Entry point for the test script."""
    try:
        test_basic_pub_sub_large_message()
        return 0
    except AssertionError as e:
        print(f"FAILURE: Assertion failed: {e}")
        return 1
    except Exception as e:
        print(f"FAILURE: Unexpected error: {e}")
        return 1

if __name__ == "__main__":
    sys.exit(main())