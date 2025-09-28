//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
//=================================================================================================

#include <print>
#include <vector>

#include "grape/exception.h"
#include "grape/script/script.h"

//=================================================================================================
// Example demonstrates
// - configuring fields in complex custom data structures from a script
// - using lua built-in libraries (e.g. math) within the script
// - configuring C++ arrays from lua arrays
//=================================================================================================

// defines a 3 d.o.f position on a plane
struct Pose {
  void configure(const grape::script::ConfigTable& table);
  float x;
  float y;
  float rz;
};

// defines a mobile robot
struct Robot {
  void configure(const grape::script::ConfigTable& table);
  std::string name;
  Pose pose{};
};

// defines a cluster of mobile robots
struct RobotCluster {
  void configure(const grape::script::ConfigTable& table);
  std::string name;
  std::vector<Robot> members;
};

// configuration for a cluster of robots (standard Lua script)
static constexpr std::string_view CONFIG = R"(
  cluster_name = "Test Cluster"    
  members = {
    { name = "robot_1", pose = { x = 0.0, y =  0.1, rz = 0.0     } },
    { name = "robot_2", pose = { x = 0.0, y = -0.1, rz = math.pi } }    
  }
)";

//=================================================================================================
auto main(int argc, const char* argv[]) -> int {
  (void)argc;
  (void)argv;
  try {
    const auto script = grape::script::ConfigScript(std::string(CONFIG));
    const auto table = script.table();
    RobotCluster robot_cluster;
    robot_cluster.configure(table);
    std::println("Cluster '{}' has following robots:", robot_cluster.name);
    for (const auto& robot : robot_cluster.members) {
      const auto pose = robot.pose;
      std::println(" '{}' at ({}, {}, {})", robot.name, pose.x, pose.y, pose.rz);
    }
    return EXIT_SUCCESS;
  } catch (...) {
    grape::Exception::print();
    return EXIT_FAILURE;
  }
}

//-------------------------------------------------------------------------------------------------
void Pose::configure(const grape::script::ConfigTable& table) {
  x = table.readOrThrow<float>("x");
  y = table.readOrThrow<float>("y");
  rz = table.readOrThrow<float>("rz");
}

//-------------------------------------------------------------------------------------------------
void Robot::configure(const grape::script::ConfigTable& table) {
  name = table.readOrThrow<std::string>("name");
  const auto pose_tab = table.read<grape::script::ConfigTable>("pose");
  if (not pose_tab.has_value()) {
    grape::panic(std::format("Key 'pose' {}", toString(pose_tab.error())));
  }
  pose.configure(pose_tab.value());
}

//-------------------------------------------------------------------------------------------------
void RobotCluster::configure(const grape::script::ConfigTable& table) {
  name = table.readOrThrow<std::string>("cluster_name");
  const auto members_res = table.read<grape::script::ConfigTable>("members");
  if (not members_res.has_value()) {
    grape::panic(std::format("Key 'members' {}", toString(members_res.error())));
  }
  const auto& members_table = members_res.value();
  const auto sz = members_table.size();
  members.resize(sz);
  for (size_t i = 0; i < sz; ++i) {
    const auto robot_res = members_table.read<grape::script::ConfigTable>(i);
    if (not robot_res.has_value()) {
      grape::panic(std::format("Robot({}) {}", i, toString(robot_res.error())));
    }
    members.at(i).configure(robot_res.value());
  }
}
