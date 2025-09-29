#!/usr/bin/env python3
"""Subscriber test script for IPC communication."""
import sys
import time
from grape_ipc_py import Config, init, RawSubscriber, ok

class TestSubscriber:
    def __init__(self):
        self.messages_received = 0
        self.matched = False
        
    def data_callback(self, sample):
        try:
            data = sample.data()  # Get bytes data
            message = data.decode('utf-8')
            self.messages_received += 1
            print(f"Message received: '{message}' (count: {self.messages_received})")
        except Exception as e:
            print(f"Error in data callback: {e}")
    
    def match_callback(self, match):
        print(f"Subscriber match status: {match.status}")
        if match.status.name == "Matched":
            self.matched = True

def main():
    try:
        # Initialize IPC
        config = Config()
        config.name = "Python Test Subscriber"
        init(config)
        
        # Create subscriber
        topic = "test_topic"
        test_sub = TestSubscriber()
        subscriber = RawSubscriber(topic, test_sub.data_callback, test_sub.match_callback)
        
        print("Subscriber started, waiting for publishers...")
        
        # Wait for publisher to match
        wait_count = 0
        while not test_sub.matched and wait_count < 20:
            time.sleep(0.5)
            wait_count += 1
            
        if test_sub.matched:
            print("Matched with publisher, waiting for messages...")
        else:
            print("No publisher match found, but continuing to listen...")
        
        # Wait for messages
        start_time = time.time()
        while time.time() - start_time < 15:  # Wait up to 15 seconds
            time.sleep(0.1)
            if test_sub.messages_received >= 3:  # Exit early if we got some messages
                break
                
        print(f"Subscriber finished. Total messages received: {test_sub.messages_received}")
        # Return success if we received at least one message
        return 0 if test_sub.messages_received > 0 else 1
        
    except Exception as e:
        print(f"Subscriber error: {e}")
        import traceback
        traceback.print_exc()
        return 1  # Error exit code

if __name__ == "__main__":
    sys.exit(main())