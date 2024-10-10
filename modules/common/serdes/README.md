# README: serdes

## Brief

Serialisation and deserialisation facility for structured data

## Detailed description

### Why serialize?

- Interoperability
  - Across programming languages (eg: C++, Python)
  - Across CPU architectures (eg: endianness, memory alignment)
  - Across data representation formats (eg: JSON)
- Data transport over IPC mechanisms
- Persistent storage and retrieval

### Requirements

In decreasing order of priority:

- Interoperable across C++ and Python
- Fast encoding and decoding
- Usable without a schema, and the need to process them into source files
- Usable without dynamic memory allocation
- Simple API, even for complex data structures composed from other data structures. Ideally, a free function to encode and to decode as follows:
  ```c++
  template<typename SerialiserType, typename DataType>
  void serialize(SerialiserType&, const DataType&)

  template<typename SerialiserType, typename DataType>
  void deserialize(SerialiserType&, DataType&)  
  ```
- Minimal additional software dependencies
- Suitable for resource-constrained systems (eg: bare metal)
- Optional features, for use-cases where we are willing to pay the performance penalty
  - Type-safe deserialisation
  - JSON deserialisable for manual introspectability
  - Self-describing (meta information is contained within data)
  - Failure notification by error code instead of exception

Since IPC relies on Zenoh, its builtin serialisation [zenoh::Bytes](https://zenoh-cpp.readthedocs.io/en/latest/serialization_deserialization.html) may be suitable for simple use-cases. However, as of Sept 2024, Zettascale advises against using it in performance-critical use cases.

### Options

(TODO)

[FlatBuffers](https://flatbuffers.dev/index.html) seems ugly but a good choice given these 
requirements. See benchmarks [[1]](https://flatbuffers.dev/flatbuffers_benchmarks.html) and 
[[2]](https://github.com/getml/reflect-cpp/tree/main/benchmarks). I want to avoid auto-generated 
code, but still want to follow a spec to serialise and deserialise data atleast across C++ and 
Python. Simpler schemes is worth a second look. Especially like 
[cppack](https://github.com/mikeloomisgg/cppack) and the 
[rationale](https://mikeloomisgg.github.io/2019-07-02-making-a-serialization-library/) on which it 
is based. This project is abandoned, but might make a good basis for rolling out my own. 
FastCDR API is really nice - just a single function to encode/decode - similar to 
cpppack and [cereal](https://github.com/USCiLab/cereal). Thinking of messagepack, cbor and cdr 
again for these reasons (i.e. simplicity, C++/Python interop). 
[nlohmann_json](https://github.com/nlohmann/json) supports all these. So does 
[reflect-cpp](https://github.com/getml/reflect-cpp). 

Others
- [CppSerialization](https://github.com/chronoxor/CppSerialization)
- [cista](https://github.com/felixguendling/cista)
- [libentity](https://github.com/emergent-design/libentity)

## TODO

- :done: benchmark serialisation and deserialisation against FastCDR, msgpack, flexbuffer
- :done: simple c++ example: serialise simple data structure consisting of standard data type
  - :done: flexbuffer
  - :done: msgpack
  - :done: fastcdr
- :done: simple flexbuffer python example
  - :done: deserialise into data structure
  - :done: deserialise as json
  - :done: cleanup docs
- :done: ways to speed up msgpack
- :done: ways to speed up flexbuffers
- :done: cdr python deserialise example
- :done: msgpack python deserialise example
- Full XCDRv2 decoder python example
- FastCDR decoder python example
- Add XCDRv2 spec to docs (chapter 10 of https://www.omg.org/spec/DDSI-RTPS/)
- advanced example: serialise complex data structure composed of other data structures
  - flexbuffer
  - msgpack
  - fastcdr
- Readme update
  - :done: why serialise
  - :done: requirements
  - Narrowing down my options: start wide, then eliminate. Why not protobuf
    - Interop is a side effect of standardisation of protocol. If the protocol is simple and well documented, its easy to write interfaces in any language  
  - benchmark results sample
  - future work: desired API
- unit tests