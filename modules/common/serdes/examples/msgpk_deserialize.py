#=================================================================================================
# Copyright (C) 2024 GRAPE Contributors
#=================================================================================================

# This example pairs with msgpk_example.cpp. Run that to serialise State to file as 'state.msgpk'

import msgpack # `pip3 install msgpack`
import json

# Define the State class for deserialized data
class State:
    def __init__(self, name, timestamp, position):
        self.name = name
        self.timestamp = timestamp
        self.position = position

    def __repr__(self):
        return f"State(name={self.name}, timestamp={self.timestamp}, position={self.position})"

# Read binary data from file
def read_file(filename):
    with open(filename, 'rb') as file:
        return file.read()

# Main execution
if __name__ == "__main__":
    serialized_data = read_file("state.msgpk")
    unpacked_data = msgpack.unpackb(serialized_data, raw=False)

    # unpack into State
    state = State(name=unpacked_data[0], timestamp=unpacked_data[1], position=unpacked_data[2])
    print(state)

    # Unpack as json (without field names as we used an array instead of a map to pack)
    json_data = json.dumps(unpacked_data)
    print(json_data)

