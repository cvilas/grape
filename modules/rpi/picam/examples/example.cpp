#include <thread>

#include "grape/picam/pi_camera.h"

int main() {
  grape::picam::PiCamera camera(nullptr);
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}