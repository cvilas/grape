/*
MIT License

Copyright (c) Foxglove Technologies Inc

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#define MCAP_IMPLEMENTATION  // Define this in exactly one .cpp file

#include <chrono>
#include <cstring>
#include <iostream>

#include <mcap/writer.hpp>

namespace {
// Returns the system time in nanoseconds. std::chrono is used here, but any
// high resolution clock API (such as clock_gettime) can be used.
auto now() -> mcap::Timestamp {
  return mcap::Timestamp(std::chrono::duration_cast<std::chrono::nanoseconds>(
                             std::chrono::system_clock::now().time_since_epoch())
                             .count());
}
}  // namespace

int main() {
  // Initialize an MCAP writer with the "ros1" profile and write the file header
  mcap::McapWriter writer;
  auto status = writer.open("output.mcap", mcap::McapWriterOptions("ros1"));
  if (!status.ok()) {
    std::cerr << "Failed to open MCAP file for writing: " << status.message << "\n";
    return 1;
  }

  // Register a Schema
  mcap::Schema stdMsgsString("std_msgs/String", "ros1msg", "string data");
  writer.addSchema(stdMsgsString);

  // Register a Channel
  mcap::Channel chatterPublisher("/chatter", "ros1", stdMsgsString.id);
  writer.addChannel(chatterPublisher);

  // Create a message payload. This would typically be done by your own
  // serialization library. In this example, we manually create ROS1 binary data
  std::array<std::byte, 4 + 13> payload;
  const uint32_t length = 13;
  std::memcpy(payload.data(), &length, 4);
  std::memcpy(payload.data() + 4, "Hello, world!", 13);

  // Write our message
  mcap::Message msg;
  msg.channelId = chatterPublisher.id;
  msg.sequence = 1;               // Optional
  msg.logTime = now();            // Required nanosecond timestamp
  msg.publishTime = msg.logTime;  // Set to logTime if not available
  msg.data = payload.data();
  msg.dataSize = payload.size();
  std::ignore = writer.write(msg);

  // Finish writing the file
  writer.close();
}
