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

using ConfigTableException = grape::Exception<grape::script::ConfigTable::Error>;

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
    for (const auto& r : robot_cluster.members) {
      const auto pose = r.pose;
      std::println(" '{}' at ({}, {}, {})", r.name, pose.x, pose.y, pose.rz);
    }
    return EXIT_SUCCESS;
  } catch (const ConfigTableException& ex) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = fprintf(stderr, "[%s]\n", toString(ex.data()).data());
    ConfigTableException::consume();
    return EXIT_FAILURE;
  } catch (const grape::script::ConfigScriptException& ex) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::ignore = fprintf(stderr, "[%s]\n", toString(ex.data()).data());
    grape::script::ConfigScriptException::consume();
    return EXIT_FAILURE;
  } catch (...) {
    grape::AbstractException::consume();
    return EXIT_FAILURE;
  }
}

//-------------------------------------------------------------------------------------------------
void Pose::configure(const grape::script::ConfigTable& table) {
  const auto x_res = table.read<float>("x");
  if (x_res.has_value()) {
    x = x_res.value();
  } else {
    grape::panic<ConfigTableException>("Key 'x' {}", x_res.error());
  }
  const auto y_res = table.read<float>("y");
  if (y_res.has_value()) {
    y = y_res.value();
  } else {
    grape::panic<ConfigTableException>("Key 'y' {}", y_res.error());
  }
  const auto rz_res = table.read<float>("rz");
  if (rz_res.has_value()) {
    rz = rz_res.value();
  } else {
    grape::panic<ConfigTableException>("Key 'rz'", rz_res.error());
  }
}

//-------------------------------------------------------------------------------------------------
void Robot::configure(const grape::script::ConfigTable& table) {
  const auto name_res = table.read<std::string>("name");
  if (name_res.has_value()) {
    name = name_res.value();
  } else {
    grape::panic<ConfigTableException>("Key 'name'", name_res.error());
  }
  const auto pose_tab = table.read<grape::script::ConfigTable>("pose");
  if (pose_tab.has_value()) {
    pose.configure(pose_tab.value());
  } else {
    grape::panic<ConfigTableException>("Key 'pose'", pose_tab.error());
  }
}

//-------------------------------------------------------------------------------------------------
void RobotCluster::configure(const grape::script::ConfigTable& table) {
  const auto name_res = table.read<std::string>("cluster_name");
  if (name_res.has_value()) {
    name = name_res.value();
  } else {
    grape::panic<ConfigTableException>("Key 'cluster_name'", name_res.error());
  }
  const auto members_res = table.read<grape::script::ConfigTable>("members");
  if (not members_res.has_value()) {
    grape::panic<ConfigTableException>("Key 'members'", members_res.error());
  }
  const auto& members_table = members_res.value();
  const auto sz = members_table.size();
  members.resize(sz);
  for (size_t i = 0; i < sz; ++i) {
    const auto robot_res = members_table.read<grape::script::ConfigTable>(i);
    if (not robot_res.has_value()) {
      grape::panic<ConfigTableException>(std::format("Robot({})", i), robot_res.error());
    }
    members.at(i).configure(robot_res.value());
  }
}
