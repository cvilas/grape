#=================================================================================================
# Copyright (C) 2024 GRAPE Contributors
#=================================================================================================

# This program pairs with example.cpp. Run that to serialise State to file as 'state.grp'

import struct

# Readable stream class
class InStream:
    def __init__(self, byte_buffer):
        self.buf = byte_buffer
        self.pos = 0

    def read_byte(self):
        result = self.buf[self.pos]
        self.pos += 1
        return result

    def read_bytes(self, count):
        slice = self.buf[self.pos:self.pos + count]
        self.pos += count
        return slice
    
# Define a decoding class
class Deserialiser:
    def __init__(self, in_stream:InStream):
        self.stream = in_stream

    def read_int16(self):
        result = struct.unpack('<h', self.stream.read_bytes(2))[0]
        return result

    def read_uint16(self):
        result = struct.unpack('<H', self.stream.read_bytes(2))[0]
        return result

    def read_int32(self):
        result = struct.unpack('<i', self.stream.read_bytes(4))[0]
        return result

    def read_uint32(self):
        result = struct.unpack('<I', self.stream.read_bytes(4))[0]
        return result

    def read_int64(self):
        result = struct.unpack('<q', self.stream.read_bytes(8))[0]
        return result

    def read_uint64(self):
        result = struct.unpack('<Q', self.stream.read_bytes(8))[0]
        return result

    def read_float32(self):
        result = struct.unpack('<f', self.stream.read_bytes(4))[0]
        return result

    def read_float64(self):
        result = struct.unpack('<d', self.stream.read_bytes(8))[0]
        return result

    def read_char(self):
        return chr(self.read_byte())

    def read_string(self):
        length = self.read_uint32()
        string = self.stream.read_bytes(length).decode('utf-8')
        return string
    
    # For decoding source arrays with known fixed number of elements (eg: std::array)
    def read_fixed_length_sequence(self, length, element_reader):
        return [element_reader() for _ in range(length)]
        
    # For decoding source arrays that were variable length (eg: std::vector) 
    def read_variable_length_sequence(self, element_reader):
        length = self.read_uint32() 
        return [element_reader() for _ in range(length)]

# Define the State class for deserialized data
class State:
    def __init__(self, name, timestamp, position):
        self.name = name
        self.timestamp = timestamp
        self.position = position

    def __repr__(self):
        return f"State(name={self.name}, timestamp={self.timestamp}, position={self.position})"
    
# Read binary serialised data from the file
def read_file(filename):
    with open(filename, 'rb') as file:
        return file.read()

# Main execution
if __name__ == "__main__":
    serialized_data = read_file("state.grp")
    stream = InStream(serialized_data)
    reader = Deserialiser(stream)
    print('Raw data:', serialized_data)

    # unpack as State
    name = reader.read_string()
    timestamp = reader.read_float64()
    position = reader.read_fixed_length_sequence(3, reader.read_float64)
    state = State(name, timestamp, position)
    print(state)