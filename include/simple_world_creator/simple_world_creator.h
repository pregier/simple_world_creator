#ifndef SIMPLE_WORLD_CREATOR_SIMPLE_WORLD_CREATOR_H_
#define SIMPLE_WORLD_CREATOR_SIMPLE_WORLD_CREATOR_H_

#include <ros/ros.h>
#include <ros/package.h>

#include <string>
#include <vector>
#include <stdio.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <cstdio>

#include <octomap/octomap.h>

struct ObjectBox
{
  std::string name;
  double bottomCenter[3];
  double size[3];
  double angle;
};

struct ObjectLineBox
{
  std::string name;
  double start[2];
  double end[2];
  double thickness;
  double height[2];
};

struct ObjectSphere
{
  std::string name;
  double bottom[3];
  double radius;
};

struct ObjectCylinder
{
  std::string name;
  double bottom[3];
  double height, radius;
};

class WorldCreator
{
public:
  std::string fileName;
  bool foundConfig;
  bool canCreateGazebo;
  bool canCreateOctomap;
  bool canCreatePNG;

  std::vector<ObjectBox> boxes;
  std::vector<ObjectSphere> spheres;
  std::vector<ObjectCylinder> cylinders;
  bool addFloor;
  std::string worldName;
  double resolution;
  double updateRate;
  double minX, minY, minZ, maxX, maxY, maxZ;

  octomap::OcTree* octree;

  std::vector<std::vector<bool> > occupancyMap;

  WorldCreator(std::string file);

  //methods for reading world config file
  bool readConfigFile();
  void setCreatePossibilities();

  void getKeyValuePair(std::string &str, std::pair<std::string, std::string> &keyValuePair);
  void readBox(std::ifstream &file);
  void readLineBox(std::ifstream &file);
  void readSphere(std::ifstream &file);
  void readCylinder(std::ifstream &file);
  void convertLineBoxToBox(const ObjectLineBox &lineBox, ObjectBox &box);

  //methods for creating gazebo world file
  bool createGazeboWorldFile();
  void addGazeboHead(std::ofstream &file);
  void addGazeboTail(std::ofstream &file);
  void addGazeboBox(std::ofstream &file, ObjectBox &box);
  void addGazeboSphere(std::ofstream &file, ObjectSphere &sphere);
  void addGazeboCylinder(std::ofstream &file, ObjectCylinder &cylinder);

  //methods for creating octomap world file
  bool createOctree();
  void addOctreeBox(octomap::OcTree &octree, ObjectBox &box);
  void addOctreeSphere(octomap::OcTree &octree, ObjectSphere &sphere);
  void addOctreeCylinder(octomap::OcTree &octree, ObjectCylinder &cylinder);

  //methods for creating png image
  bool createPNG();
  void createOccupancyMapFromOctomap();
  bool isOctreeNodeOccupied(const octomap::OcTreeKey &key) const;
  void writeDataToPNM(std::ofstream &file);
};

#endif // SIMPLE_WORLD_CREATOR_SIMPLE_WORLD_CREATOR_H_
