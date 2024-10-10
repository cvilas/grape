#=================================================================================================
# Copyright (C) 2024 GRAPE Contributors
#=================================================================================================

# This example pairs with cdr_example.cpp. Run that to serialise State to file as 'state.cdr'
# NOTE: Decoder implemented here works for default encoder settings for eprosima::fastcdr::Cdr:
# - Encoding follows XCDR v2 spec
# - Encoding does not have the 4-byte header containing REP_ID and REP_OPTIONS
# - Encoding of fixed length sequences (eg: std::array) does not include array length 
# TODO: Implement a full XCDRv2 decoder following specification from chapter 10 of https://www.omg.org/spec/DDSI-RTPS/

import struct

# Define the State class for deserialized data
class State:
    def __init__(self, name, timestamp, position):
        self.name = name
        self.timestamp = timestamp
        self.position = position

    def __repr__(self):
        return f"State(name={self.name}, timestamp={self.timestamp}, position={self.position})"

import struct

# Define a CDR decoding class
class CDRReader:
    def __init__(self, byte_buffer):
        self.buf = byte_buffer
        self.pos = 0

        # NOTE 1: By default eprosima::fastcdr::Cdr does not REPR_ID and REPR_OPT. 
        # NOTE 2: Assume Little-Endian encoding (would have been set in REPR_ID)
        #self.align(2)
        #self.repr_id = struct.unpack('<H', self.buf[self.pos:self.pos + 2])[0]
        #self.align(2)
        #self.repr_opt = struct.unpack('<H', self.buf[self.pos:self.pos + 2])[0]
        #self.little_endian = ((self.repr_id & 0x0001) == 0x0001)
        self.little_endian = True 

    def read_byte(self):
        result = self.buf[self.pos]
        self.pos += 1
        return result

    def read_bytes(self, count):
        slice = self.buf[self.pos:self.pos + count]
        self.pos += count
        return slice

    def align(self, alignment):
        modulo = (self.pos + 4) % alignment
        if modulo > 0:
            self.pos += alignment - modulo

    def read_int16(self):
        self.align(2)
        result = struct.unpack('<h' if self.little_endian else '>h', self.read_bytes(2))[0]
        return result

    def read_uint16(self):
        self.align(2)
        result = struct.unpack('<H' if self.little_endian else '>H', self.read_bytes(2))[0]
        return result

    def read_int32(self):
        self.align(4)
        result = struct.unpack('<i' if self.little_endian else '>i', self.read_bytes(4))[0]
        return result

    def read_uint32(self):
        self.align(4)
        result = struct.unpack('<I' if self.little_endian else '>I', self.read_bytes(4))[0]
        return result

    def read_int64(self):
        self.align(4)
        result = struct.unpack('<q' if self.little_endian else '>q', self.read_bytes(8))[0]
        return result

    def read_uint64(self):
        self.align(4)
        result = struct.unpack('<Q' if self.little_endian else '>Q', self.read_bytes(8))[0]
        return result

    def read_float32(self):
        self.align(4)
        result = struct.unpack('<f' if self.little_endian else '>f', self.read_bytes(4))[0]
        return result

    def read_float64(self):
        self.align(4)
        result = struct.unpack('<d' if self.little_endian else '>d', self.read_bytes(8))[0]
        return result

    def read_char(self):
        return chr(self.read_byte())

    def read_string(self):
        length = self.read_uint32()
        string = self.read_bytes(length - 1).decode('utf-8')
        self.pos += 1  # Skip null-termination character
        return string
    
    # For decoding source arrays with known fixed number of elements (eg: std::array)
    def read_fixed_length_sequence(self, length, element_reader):
        return [element_reader() for _ in range(length)]
        
    # For decoding source arrays that were variable length (eg: std::vector) 
    def read_variable_length_sequence(self, element_reader):
        length = self.read_uint32() 
        self.align(4)  # Sequences are aligned to 4 bytes
        return [element_reader() for _ in range(length)]

# Read binary data from the CDR file
def read_cdr_file(filename):
    with open(filename, 'rb') as file:
        return file.read()

# Main execution
if __name__ == "__main__":
    serialized_data = read_cdr_file("state.cdr")
    reader = CDRReader(serialized_data)
    print('Raw data:', serialized_data)

    # unpack as State
    name = reader.read_string()
    timestamp = reader.read_float64()
    position = reader.read_fixed_length_sequence(3, reader.read_float64)
    state = State(name, timestamp, position)
    print(state)