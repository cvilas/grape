#!/usr/bin/env python3
"""Publisher test script for IPC communication."""
import sys
import time
from grape_ipc_py import Config, init, RawPublisher, ok

def match_callback(match):
    print(f"Publisher match status: {match.status}")

def main():
    try:
        # Initialize IPC
        config = Config()
        config.name = "Python Test Publisher"
        init(config)
        
        # Create publisher with match callback
        topic = "test_topic"
        publisher = RawPublisher(topic, match_callback)
        
        # Wait for subscriber to connect
        print("Publisher waiting for subscribers...")
        wait_count = 0
        while publisher.get_subscriber_count() == 0 and wait_count < 30:
            time.sleep(0.5)
            wait_count += 1
            
        if publisher.get_subscriber_count() == 0:
            print("No subscribers found, sending messages anyway...")
        else:
            print(f"Found {publisher.get_subscriber_count()} subscriber(s)")
        
        # Send test messages
        for i in range(5):
            message = f"Test message {i + 1}"
            data = message.encode('utf-8')
            success = publisher.publish(data)
            print(f"Message sent: '{message}' (success: {success})")
            time.sleep(1.0)  # Slower publishing to give subscriber time
            
        print("Publisher finished")
        return 0  # Success exit code
        
    except Exception as e:
        print(f"Publisher error: {e}")
        import traceback
        traceback.print_exc()
        return 1  # Error exit code

if __name__ == "__main__":
    sys.exit(main())