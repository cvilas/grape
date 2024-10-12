# README: serdes

## Brief

Serialisation and deserialisation facility for structured data

## Detailed description

### Why serialize data?

- Interoperability
  - Across programming languages
  - Across CPU architectures (eg: endianness, memory alignment)
  - Across data representation formats (eg: JSON, XML, CBOR)
- Transport over IPC mechanisms
- Persistent storage and retrieval

### Requirements

In decreasing order of priority:

- Interoperable across C++ and Python
- Fast encoding and decoding
- Usable without a schema [Note 1]
- Usable without dynamic memory allocation
- Simple API, even for complex data structures composed from other data structures. Ideally, a free function to encode and to decode as follows:
  ```c++
  template<typename SerialiserType, typename DataType>
  void serialize(SerialiserType&, const DataType&)

  template<typename SerialiserType, typename DataType>
  void deserialize(SerialiserType&, DataType&)  
  ```
- Good documentation (spec., usage)
- Error handling preferably as return codes instead of exceptions
- Suitable for resource-constrained systems (eg: bare metal systems)
- Minimal additional software dependencies
- (Optional) Self-describing [Note 2] (i.e. meta information is contained within data)

[Note 1] Schema-based methods require pre-processing an IDL to autogenerate code, which is an abomination. I want to write native code conforming to project-specific coding standards. However, schemaless encoding requires knowledge of composition of data structures both at the encoding and decoding ends to manually process them in the right order. These are minor inconveniences compared to the compromise of dealing with the complexity of integrating schemas and the mechanism to autogenerate non-conforming code.

[Note 2] Enables type-safe deserialisation, introspection (as text or JSON, for instance), version control and backwards compatibility. Beware, this comes at a cost in terms of reduced performance and increased data sizes

### Options

For POD types and data structures based on them, in many cases, this may be all you need:

```c++
struct MyCustomObject{/* fields */};

void serialise(std::stringstream& io, const MyCustomObject& obj) {
  io.write(reinterpret_cast<char* const>(&obj), sizeof(MyCustomObject));
}

auto deserialise(std::stringstream& io) -> MyCustomObject {
  MyCustomObject obj;
  io.read(reinterpret_cast<char*>(&obj), sizeof(MyCustomObject));
  return obj;
}
```

This is not portable across different architectures and compiler settings due to potential differences in memory alignment (padding), endianness, etc. It is also not suitable for complex structures containing pointers or dynamic memory (`std::vector`, `std::map`, etc). Error handling is non-existant. Therefore, we want a scheme built on top of a _specification_ defining rules for encoding and decoding data-types. Interoperability is a side-effect of such specification; if it is simple and well documented, its easy to implement encoders and decoders in any programming language.

The requirements stated above narrowed down options to the following:

- [FlexBuffers](https://flatbuffers.dev/flexbuffers.html) (Schemaless extension to FlatBuffers)
- [MessagePack](https://msgpack.org/)
- [FastCDR](https://github.com/eProsima/Fast-CDR)
- [zenoh::Bytes](https://zenoh-cpp.readthedocs.io/en/latest/serialization_deserialization.html)

The main advantage With `zenoh::Bytes` is convenience, as it comes with Zenoh IPC. However, Zettascale advises against using it in performance-critical use cases (as of Oct 2024). Also, we would like something that is standalone, as serialisation is used not just for IPC. Therefore it was eliminated as an option.

Representative [benchmarks](./examples/bench.cpp) for the rest of them: 

```text

// benchmarked data structure
struct Person {
  std::string name;
  int32_t id{};
  std::string email;
  std::vector<float> scores;
};

-------------------------------------------------------------
Benchmark                     CPU Time  Serialised Data Size
-------------------------------------------------------------
bmFastCDRSerialize            12.7 ns   58 bytes
bmFastCDRDeserialize          23.5 ns    
bmCDRSerialize                24.3 ns   64 bytes
bmCDRDeserialize              34.6 ns   
bmMsgpackSerialize            45.7 ns   48 bytes
bmMsgpackDeserialize           109 ns
bmFlexBufferSerialize          155 ns   63 bytes
bmFlexBufferDeserialize       29.2 ns   
```

serdes          | C++/Python interop | encode speed | decode speed | API simplicity | Documentation 
----------------|--------------------|--------------|--------------|----------------|---------------
FastCDR/FastCDR | -                  | =======      | ====         | ====           | =
FastCDR/CDR     | -                  | ======       | ===          | ====           | ==
Messagepack     | ====               | ===          | =            | ===            | ====
Flexbuffers     | ====               | =            | ===          | ==             | ===

FastCDR is simple and extremely fast. Drawbacks are that it modifies the CDR standard, the spec is not well documented, and there is no out-of-the-box Python support. Reverse engineering the C++ implementation for a Python deserialiser may not be too hard though. If speed is the main criterion, the next obvious candidate is CDR (also provided by FastCDR library), which implements Extended CDR version 2 (see chapter 10 of the [DDS interoperability wire protocol](https://www.omg.org/spec/DDSI-RTPS/)).

Messagepack wins overall for painless interoperability, fairly simple API, smallest serialised data size (wire efficiency), good documentation and reasonable performance.

With FastBuffer, the main advantage is incredibly fast deserialisation. It's  possible to deference individual elements in the serialised object without deserialising the entire object. Serialisation is the slowest of all tested protocols.

## TODO

- Generic API to pick any supported serialisation mechanism at runtime. See [reflect-cpp](https://github.com/getml/reflect-cpp)
- Python XCDRv2 decoder module
- Python FastCDR decoder module
- Advanced example: serialise complex data structure composed of other data structures
  - flexbuffer
  - msgpack
  - fastcdr
- unit tests