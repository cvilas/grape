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
- Error handling preferably as return codes instead of exceptions
- Suitable for resource-constrained systems (eg: bare metal systems)
- Minimal additional software dependencies
- (Optional) Self-describing [Note 2] (i.e. meta information is contained within data)

[Note 1] Schema-based methods require pre-processing an IDL to autogenerate code, which is an abomination. I want to write native code conforming to project-specific coding standards. However, schemaless encoding requires knowledge of composition of data structures both at the encoding and decoding ends to manually process them in the right order. These are minor inconveniences compared to the compromise of dealing with the complexity of integrating schemas and the mechanism to autogenerate non-conforming code.

[Note 2] Enables type-safe deserialisation, introspection (as text or JSON, for instance), version control and backwards compatibility. Beware, this comes at a cost in terms of reduced performance and increased data sizes

### Options

*tl;dr:* Prefer to use FastCDR! 

*Longer version:*

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
------------------------------------------------------------------
Benchmark                        Time             CPU   Iterations
------------------------------------------------------------------
bmFastCDRSerialize            12.7 ns         12.7 ns     53660564
bmFastCDRDeserialize          23.5 ns         23.4 ns     30604937
bmCDRSerialize                24.3 ns         24.3 ns     28724470
bmCDRDeserialize              34.6 ns         34.6 ns     21413645
bmMsgpackSerialize            45.7 ns         45.7 ns     15186918
bmMsgpackDeserialize           109 ns          109 ns      6421654
bmFlexBufferSerialize          155 ns          155 ns      4447683
bmFlexBufferDeserialize       29.2 ns         29.2 ns     23926965
```

FastCDR is simple and extremely fast. A drawback is that it modifies the CDR standard. Therefore  encoder/decoder in other languages would have to be implemented by reverse engineering the C++ implementation to do so. But it needs to be done only once.

The next obvious candidate is CDR (also provided by FastCDR library), which implements Extended CDR version 2 (see chapter 10 of the [DDS interoperability wire protocol](https://www.omg.org/spec/DDSI-RTPS/)).

Where painless interoperability across programming languages is the main objective, MessagePack is the clear choice. 

With FastBuffer, the main advantage is its flexibility in representation. Serialisation is terribily slow, but deserialisation is incredibly fast. It's  possible to deference individual elements in the serialised object without deserialising the entire object. But I am not sure how useful this actually is in practice.

## TODO

- Generic API to pick any supported serialisation mechanism at runtime.
- Python XCDRv2 decoder module
- Python FastCDR decoder module
- Add XCDRv2 spec to docs (chapter 10 of https://www.omg.org/spec/DDSI-RTPS/)
- Advanced example: serialise complex data structure composed of other data structures
  - flexbuffer
  - msgpack
  - fastcdr
- unit tests