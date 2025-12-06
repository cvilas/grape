# plot

Realtime signal plotting

## Design considerations

- User workflow: `grape_plot --topics="/some/topic/name","/another/topic/name"`
- Uses SDL3 for plotting (See scratch/plot_tests/sdlplot.cpp) 
- Making this work requires serdes schema to be provided
  - Schema could be shared at topic discovery stage as meta data

### Schema  
- The schema specification for topic data is the real challenge that needs to be solved. Gets especially complex with nested data types. *Solve this first*
- The schema could potentially come from a schema registry (database), accessible via IPC from any component. 
- The schema could be registered by serdes functions

```C++
void serialise(Serialiser& ser, const Datatype& data, std::optional<SchemaRegister>& schema = nullopt) {
  // pack data as you would
  ser.pack(data.field1);
  ser.pack(data.field2);
  // if schema_register is specified, specify schema in the same way
  if(schema_register) {
    schema.setName(getTypeName(data));
    schema.addField(getTypeName(data.field1));
    schema.addField(getTypeName(data.field2));
  }
}

auto generateSchema(const T& data) -> std::string {
  // reflect each field
  // generate string description
}
```

```C++
void plot(Plotter& plotter, const Schema& schema, std::span<const std::byte> data) {
  plotter.plot(schema, data);
}
```