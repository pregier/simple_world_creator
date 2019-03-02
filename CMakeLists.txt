cmake_minimum_required(VERSION 2.8.3)
project(simple_world_creator)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
)

find_package(octomap REQUIRED)
find_package(octomap_msgs REQUIRED)

catkin_package(
  INCLUDE_DIRS include
  CATKIN_DEPENDS roscpp rospy
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${OCTOMAP_INCLUDE_DIRS}
)

add_executable(simple_world_creator src/main.cpp src/simple_world_creator.cpp)
target_link_libraries(simple_world_creator ${catkin_LIBRARIES} ${OCTOMAP_LIBRARIES})

