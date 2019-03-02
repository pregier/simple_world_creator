#include <simple_world_creator/simple_world_creator.h>

WorldCreator::WorldCreator(std::string file)
{
  octree = NULL;
  canCreateOctomap = false;
  canCreateGazebo = false;
  canCreatePNG = false;
  worldName = "";
  resolution = 0.0;
  updateRate = 0.0;
  addFloor = false;
  minZ = 0.0;
  maxZ = 5.0;

  fileName = file;
  foundConfig = readConfigFile();
}

bool WorldCreator::readConfigFile()
{
  std::ifstream file(fileName.c_str());
  if (!file.good())
    return false;

  std::string line;
  std::pair<std::string, std::string> keyValuePair;
  while (std::getline(file, line))
  {
    if (line.empty() || line[0] == '#')
      continue;

    getKeyValuePair(line, keyValuePair);

    if (keyValuePair.first == "world_name" && !keyValuePair.second.empty())
      worldName = keyValuePair.second;
    else if (keyValuePair.first == "update_rate" && !keyValuePair.second.empty())
    {
      std::istringstream iss(keyValuePair.second);
      iss >> updateRate;
    }
    else if (keyValuePair.first == "add_floor" && !keyValuePair.second.empty())
    {
      if (keyValuePair.second == "true")
        addFloor = true;
      else
        addFloor = false;
    }
    else if (keyValuePair.first == "resolution" && !keyValuePair.second.empty())
    {
      std::istringstream iss(keyValuePair.second);
      iss >> resolution;
    }
    else if (keyValuePair.first == "-box" && keyValuePair.second.empty())
      readBox(file);
    else if (keyValuePair.first == "-line_box" && keyValuePair.second.empty())
      readLineBox(file);
    else if (keyValuePair.first == "-sphere" && keyValuePair.second.empty())
      readSphere(file);
    else if (keyValuePair.first == "-cylinder" && keyValuePair.second.empty())
      readCylinder(file);
  }

  file.close();

  return true;
}

void WorldCreator::setCreatePossibilities()
{
  if (boxes.size() == 0 && cylinders.size() == 0 && spheres.size() == 0)
    return;

  if (worldName != "" && updateRate != 0.0)
    canCreateGazebo = true;

  if (resolution >= 0.0)
  {
    canCreateOctomap = true;
    canCreatePNG = true;
  }
}

void WorldCreator::getKeyValuePair(std::string &str, std::pair<std::string, std::string> &keyValuePair)
{
  keyValuePair.first.clear();
  keyValuePair.second.clear();

  bool readValue = false;
  for (std::string::iterator it = str.begin(); it != str.end(); ++it)
  {
    if (*it == ':')
    {
      readValue = true;
      continue;
    }

    if (!readValue)
      keyValuePair.first.push_back(*it);
    else
      keyValuePair.second.push_back(*it);
  }

}

void WorldCreator::readBox(std::ifstream &file)
{
  std::string line;
  std::pair<std::string, std::string> keyValuePair;

  ObjectBox box;
  bool gotName = false, gotBottom = false, gotSize = false, gotAngle = false;

  while (std::getline(file, line))
  {
    if (line.empty())
      continue;

    getKeyValuePair(line, keyValuePair);

    if (keyValuePair.first == "name" && !keyValuePair.second.empty())
    {
      box.name = keyValuePair.second;
      gotName = true;
    }
    else if (keyValuePair.first == "bottom_center" && !keyValuePair.second.empty())
    {
      std::istringstream iss(keyValuePair.second);
      if (!(iss >> box.bottomCenter[0] >> box.bottomCenter[1] >> box.bottomCenter[2]))
        continue;
      gotBottom = true;
    }
    else if (keyValuePair.first == "size" && !keyValuePair.second.empty())
    {
      std::istringstream iss(keyValuePair.second);
      if (!(iss >> box.size[0] >> box.size[1] >> box.size[2]))
        continue;
      gotSize = true;
    }
    else if (keyValuePair.first == "angle" && !keyValuePair.second.empty())
    {
      std::istringstream iss(keyValuePair.second);
      if (!(iss >> box.angle))
        continue;
      gotAngle = true;
      box.angle = box.angle * M_PI / 180.0;
    }

    if (gotName && gotBottom && gotSize && gotAngle)
    {
      boxes.push_back(box);
      break;
    }
  }
}

void WorldCreator::readLineBox(std::ifstream &file)
{
  std::string line;
  std::pair<std::string, std::string> keyValuePair;

  ObjectLineBox lineBox;
  bool gotName = false, gotStart = false, gotEnd = false, gotHeight = false, gotThickness = false;

  while (std::getline(file, line))
  {
    if (line.empty())
      continue;

    getKeyValuePair(line, keyValuePair);

    if (keyValuePair.first == "name" && !keyValuePair.second.empty())
    {
      lineBox.name = keyValuePair.second;
      gotName = true;
    }
    else if (keyValuePair.first == "start" && !keyValuePair.second.empty())
    {
      std::istringstream iss(keyValuePair.second);
      if (!(iss >> lineBox.start[0] >> lineBox.start[1]))
        continue;
      gotStart = true;
    }
    else if (keyValuePair.first == "end" && !keyValuePair.second.empty())
    {
      std::istringstream iss(keyValuePair.second);
      if (!(iss >> lineBox.end[0] >> lineBox.end[1]))
        continue;
      gotEnd = true;
    }
    else if (keyValuePair.first == "thickness" && !keyValuePair.second.empty())
    {
      std::istringstream iss(keyValuePair.second);
      if (!(iss >> lineBox.thickness))
        continue;
      gotThickness = true;
    }
    else if (keyValuePair.first == "height" && !keyValuePair.second.empty())
    {
      std::istringstream iss(keyValuePair.second);
      if (!(iss >> lineBox.height[0] >> lineBox.height[1]))
        continue;
      gotHeight = true;
    }

    if (gotName && gotStart && gotEnd && gotThickness && gotHeight)
    {
      ObjectBox box;
      convertLineBoxToBox(lineBox, box);
      boxes.push_back(box);
      break;
    }
  }
}

void WorldCreator::readSphere(std::ifstream &file)
{
  std::string line;
  std::pair<std::string, std::string> keyValuePair;

  ObjectSphere sphere;
  bool gotName = false, gotBottom = false, gotRadius = false;

  while (std::getline(file, line))
  {
    if (line.empty())
      continue;

    getKeyValuePair(line, keyValuePair);

    if (keyValuePair.first == "name" && !keyValuePair.second.empty())
    {
      sphere.name = keyValuePair.second;
      gotName = true;
    }
    else if (keyValuePair.first == "bottom" && !keyValuePair.second.empty())
    {
      std::istringstream iss(keyValuePair.second);
      if (!(iss >> sphere.bottom[0] >> sphere.bottom[1] >> sphere.bottom[2]))
        continue;
      gotBottom = true;
    }
    else if (keyValuePair.first == "radius" && !keyValuePair.second.empty())
    {
      std::istringstream iss(keyValuePair.second);
      if (!(iss >> sphere.radius))
        continue;
      gotRadius = true;
    }

    if (gotName && gotBottom && gotRadius)
    {
      spheres.push_back(sphere);
      break;
    }
  }
}

void WorldCreator::readCylinder(std::ifstream &file)
{
  std::string line;
  std::pair<std::string, std::string> keyValuePair;

  ObjectCylinder cylinder;
  bool gotName = false, gotBottom = false, gotRadius = false, gotHeight = false;

  while (std::getline(file, line))
  {
    if (line.empty())
      continue;

    getKeyValuePair(line, keyValuePair);

    if (keyValuePair.first == "name" && !keyValuePair.second.empty())
    {
      cylinder.name = keyValuePair.second;
      gotName = true;
    }
    else if (keyValuePair.first == "bottom" && !keyValuePair.second.empty())
    {
      std::istringstream iss(keyValuePair.second);
      if (!(iss >> cylinder.bottom[0] >> cylinder.bottom[1] >> cylinder.bottom[2]))
        continue;
      gotBottom = true;
    }
    else if (keyValuePair.first == "radius" && !keyValuePair.second.empty())
    {
      std::istringstream iss(keyValuePair.second);
      if (!(iss >> cylinder.radius))
        continue;
      gotRadius = true;
    }
    else if (keyValuePair.first == "height" && !keyValuePair.second.empty())
    {
      std::istringstream iss(keyValuePair.second);
      if (!(iss >> cylinder.height))
        continue;
      gotHeight = true;
    }

    if (gotName && gotBottom && gotRadius && gotHeight)
    {
      cylinders.push_back(cylinder);
      break;
    }
  }
}

void WorldCreator::convertLineBoxToBox(const ObjectLineBox &lineBox, ObjectBox &box)
{
  box.name = lineBox.name;

  box.bottomCenter[0] = 0.5 * (lineBox.start[0] + lineBox.end[0]);
  box.bottomCenter[1] = 0.5 * (lineBox.start[1] + lineBox.end[1]);
  box.bottomCenter[2] = lineBox.height[0];

  box.size[0] = lineBox.thickness;
  box.size[1] = sqrt(pow(lineBox.start[0] - lineBox.end[0], 2) + pow(lineBox.start[1] - lineBox.end[1], 2));
  box.size[2] = lineBox.height[1] - lineBox.height[0];

  box.angle = atan2(lineBox.end[0] - lineBox.start[0], -lineBox.end[1] + lineBox.start[1]);
}

bool WorldCreator::createGazeboWorldFile()
{
  if (!canCreateGazebo)
  {
    std::cout << "Cannot create gazebo file, because not all necessary parameters have been set. Need 'world_name', 'update_rate', and at least one object."
        << std::endl;
    return false;
  }

  std::string fileNameGazebo = fileName + ".world";
  std::ofstream file(fileNameGazebo.c_str());
  addGazeboHead(file);

  if (addFloor)
  {
    ObjectBox floorBox;
    floorBox.name = "floor";
    //floorBox.minimum[0] = floorBox.minimum[1] = -100.0;
    //floorBox.minimum[2] = -0.1;
    //floorBox.maximum[0] = floorBox.maximum[1] = 100.0;
    //floorBox.maximum[2] = 0.0;
    addGazeboBox(file, floorBox);
  }

  for (int i = 0; i < boxes.size(); ++i)
    addGazeboBox(file, boxes[i]);

  for (int i = 0; i < spheres.size(); ++i)
    addGazeboSphere(file, spheres[i]);

  for (int i = 0; i < cylinders.size(); ++i)
    addGazeboCylinder(file, cylinders[i]);

  addGazeboTail(file);

  return true;
}

void WorldCreator::addGazeboHead(std::ofstream &file)
{
  file << "<sdf version='1.5'>" << std::endl;
  file << "  <world name='" << worldName << "'>" << std::endl;
  file << "    <light name='sun' type='directional'>" << std::endl;
  file << "      <cast_shadows>1</cast_shadows>" << std::endl;
  file << "      <pose frame=''>0 0 10 0 -0 0</pose>" << std::endl;
  file << "      <diffuse>0.8 0.8 0.8 1</diffuse>" << std::endl;
  file << "      <specular>0.2 0.2 0.2 1</specular>" << std::endl;
  file << "      <attenuation>" << std::endl;
  file << "        <range>1000</range>" << std::endl;
  file << "        <constant>0.9</constant>" << std::endl;
  file << "        <linear>0.01</linear>" << std::endl;
  file << "        <quadratic>0.001</quadratic>" << std::endl;
  file << "      </attenuation>" << std::endl;
  file << "      <direction>-0.5 0.1 -0.9</direction>" << std::endl;
  file << "    </light>" << std::endl;
  file << "    <physics name='default_physics' default='0' type='ode'>" << std::endl;
  file << "      <max_step_size>" << 1.0 / updateRate << "</max_step_size>" << std::endl;
  file << "      <real_time_factor>1</real_time_factor>" << std::endl;
  file << "      <real_time_update_rate>" << updateRate << "</real_time_update_rate>" << std::endl;
  file << "      <gravity>0 0 -9.8</gravity>" << std::endl;
  file << "      <magnetic_field>5.5645e-06 2.28758e-05 -4.23884e-05</magnetic_field>" << std::endl;
  file << "    </physics>" << std::endl;
  file << "    <scene>" << std::endl;
  file << "      <ambient>0.4 0.4 0.4 1</ambient>" << std::endl;
  file << "      <background>0.7 0.7 0.7 1</background>" << std::endl;
  file << "      <shadows>1</shadows>" << std::endl;
  file << "    </scene>" << std::endl;
}

void WorldCreator::addGazeboTail(std::ofstream &file)
{
  file << "  </world>" << std::endl;
  file << "</sdf>" << std::endl;
}

void WorldCreator::addGazeboBox(std::ofstream &file, ObjectBox &box)
{
  file << "    <model name='" << box.name << "'>" << std::endl;
  file << "      <pose frame=''>" << box.bottomCenter[0] << " " << box.bottomCenter[1] << " " << box.bottomCenter[2] + 0.5 * box.size[2] << " 0 0 " << box.angle
      << " </pose>" << std::endl;
  file << "      <static>1</static>" << std::endl;
  file << "      <link name='link'>" << std::endl;
  file << "        <inertial>" << std::endl;
  file << "          <mass>1</mass>" << std::endl;
  file << "          <inertia>" << std::endl;
  file << "            <ixx>1</ixx>" << std::endl;
  file << "            <ixy>0</ixy>" << std::endl;
  file << "            <ixz>0</ixz>" << std::endl;
  file << "            <iyy>1</iyy>" << std::endl;
  file << "            <iyz>0</iyz>" << std::endl;
  file << "            <izz>1</izz>" << std::endl;
  file << "          </inertia>" << std::endl;
  file << "        </inertial>" << std::endl;
  file << "        <collision name='collision'>" << std::endl;
  file << "          <geometry>" << std::endl;
  file << "            <box>" << std::endl;
  file << "              <size>" << box.size[0] << " " << box.size[1] << " " << box.size[2] << "</size>" << std::endl;
  file << "            </box>" << std::endl;
  file << "          </geometry>" << std::endl;
  file << "          <max_contacts>10</max_contacts>" << std::endl;
  file << "          <surface>" << std::endl;
  file << "            <contact>" << std::endl;
  file << "              <ode/>" << std::endl;
  file << "            </contact>" << std::endl;
  file << "            <bounce/>" << std::endl;
  file << "            <friction>" << std::endl;
  file << "              <ode/>" << std::endl;
  file << "            </friction>" << std::endl;
  file << "          </surface>" << std::endl;
  file << "        </collision>" << std::endl;
  file << "        <visual name='visual'>" << std::endl;
  file << "          <geometry>" << std::endl;
  file << "            <box>" << std::endl;
  file << "              <size>" << box.size[0] << " " << box.size[1] << " " << box.size[2] << "</size>" << std::endl;
  file << "            </box>" << std::endl;
  file << "          </geometry>" << std::endl;
  file << "          <material>" << std::endl;
  file << "            <script>" << std::endl;
  file << "              <uri>file://media/materials/scripts/gazebo.material</uri>" << std::endl;
  file << "              <name>Gazebo/Grey</name>" << std::endl;
  file << "            </script>" << std::endl;
  file << "          </material>" << std::endl;
  file << "        </visual>" << std::endl;
  file << "        <self_collide>0</self_collide>" << std::endl;
  file << "        <kinematic>0</kinematic>" << std::endl;
  file << "        <gravity>0</gravity>" << std::endl;
  file << "      </link>" << std::endl;
  file << "    </model>" << std::endl;
}

void WorldCreator::addGazeboSphere(std::ofstream &file, ObjectSphere &sphere)
{
  file << "    <model name='" << sphere.name << "'>" << std::endl;
  file << "      <pose frame=''>" << sphere.bottom[0] << " " << sphere.bottom[1] << " " << sphere.bottom[2] + sphere.radius << " 0 0 0</pose>" << std::endl;
  file << "      <static>1</static>" << std::endl;
  file << "      <link name='link'>" << std::endl;
  file << "        <inertial>" << std::endl;
  file << "          <mass>1</mass>" << std::endl;
  file << "          <inertia>" << std::endl;
  file << "            <ixx>1</ixx>" << std::endl;
  file << "            <ixy>0</ixy>" << std::endl;
  file << "            <ixz>0</ixz>" << std::endl;
  file << "            <iyy>1</iyy>" << std::endl;
  file << "            <iyz>0</iyz>" << std::endl;
  file << "            <izz>1</izz>" << std::endl;
  file << "          </inertia>" << std::endl;
  file << "        </inertial>" << std::endl;
  file << "        <collision name='collision'>" << std::endl;
  file << "          <geometry>" << std::endl;
  file << "            <sphere>" << std::endl;
  file << "              <radius>" << sphere.radius << "</radius>" << std::endl;
  file << "            </sphere>" << std::endl;
  file << "          </geometry>" << std::endl;
  file << "          <max_contacts>10</max_contacts>" << std::endl;
  file << "          <surface>" << std::endl;
  file << "            <contact>" << std::endl;
  file << "              <ode/>" << std::endl;
  file << "            </contact>" << std::endl;
  file << "            <bounce/>" << std::endl;
  file << "            <friction>" << std::endl;
  file << "              <ode/>" << std::endl;
  file << "            </friction>" << std::endl;
  file << "          </surface>" << std::endl;
  file << "        </collision>" << std::endl;
  file << "        <visual name='visual'>" << std::endl;
  file << "          <geometry>" << std::endl;
  file << "            <sphere>" << std::endl;
  file << "              <radius>" << sphere.radius << "</radius>" << std::endl;
  file << "            </sphere>" << std::endl;
  file << "          </geometry>" << std::endl;
  file << "          <material>" << std::endl;
  file << "            <script>" << std::endl;
  file << "              <uri>file://media/materials/scripts/gazebo.material</uri>" << std::endl;
  file << "              <name>Gazebo/Grey</name>" << std::endl;
  file << "            </script>" << std::endl;
  file << "          </material>" << std::endl;
  file << "        </visual>" << std::endl;
  file << "        <self_collide>0</self_collide>" << std::endl;
  file << "        <kinematic>0</kinematic>" << std::endl;
  file << "        <gravity>0</gravity>" << std::endl;
  file << "      </link>" << std::endl;
  file << "    </model>" << std::endl;
}

void WorldCreator::addGazeboCylinder(std::ofstream &file, ObjectCylinder &cylinder)
{
  file << "    <model name='" << cylinder.name << "'>" << std::endl;
  file << "      <pose frame=''>" << cylinder.bottom[0] << " " << cylinder.bottom[1] << " " << cylinder.bottom[2] + 0.5 * cylinder.height << " 0 0 0</pose>"
      << std::endl;
  file << "      <static>1</static>" << std::endl;
  file << "      <link name='link'>" << std::endl;
  file << "        <inertial>" << std::endl;
  file << "          <mass>1</mass>" << std::endl;
  file << "          <inertia>" << std::endl;
  file << "            <ixx>1</ixx>" << std::endl;
  file << "            <ixy>0</ixy>" << std::endl;
  file << "            <ixz>0</ixz>" << std::endl;
  file << "            <iyy>1</iyy>" << std::endl;
  file << "            <iyz>0</iyz>" << std::endl;
  file << "            <izz>1</izz>" << std::endl;
  file << "          </inertia>" << std::endl;
  file << "        </inertial>" << std::endl;
  file << "        <collision name='collision'>" << std::endl;
  file << "          <geometry>" << std::endl;
  file << "            <cylinder>" << std::endl;
  file << "              <radius>" << cylinder.radius << "</radius>" << std::endl;
  file << "              <length>" << cylinder.height << "</length>" << std::endl;
  file << "            </cylinder>" << std::endl;
  file << "          </geometry>" << std::endl;
  file << "          <max_contacts>10</max_contacts>" << std::endl;
  file << "          <surface>" << std::endl;
  file << "            <contact>" << std::endl;
  file << "              <ode/>" << std::endl;
  file << "            </contact>" << std::endl;
  file << "            <bounce/>" << std::endl;
  file << "            <friction>" << std::endl;
  file << "              <ode/>" << std::endl;
  file << "            </friction>" << std::endl;
  file << "          </surface>" << std::endl;
  file << "        </collision>" << std::endl;
  file << "        <visual name='visual'>" << std::endl;
  file << "          <geometry>" << std::endl;
  file << "            <cylinder>" << std::endl;
  file << "              <radius>" << cylinder.radius << "</radius>" << std::endl;
  file << "              <length>" << cylinder.height << "</length>" << std::endl;
  file << "            </cylinder>" << std::endl;
  file << "          </geometry>" << std::endl;
  file << "          <material>" << std::endl;
  file << "            <script>" << std::endl;
  file << "              <uri>file://media/materials/scripts/gazebo.material</uri>" << std::endl;
  file << "              <name>Gazebo/Grey</name>" << std::endl;
  file << "            </script>" << std::endl;
  file << "          </material>" << std::endl;
  file << "        </visual>" << std::endl;
  file << "        <self_collide>0</self_collide>" << std::endl;
  file << "        <kinematic>0</kinematic>" << std::endl;
  file << "        <gravity>0</gravity>" << std::endl;
  file << "      </link>" << std::endl;
  file << "    </model>" << std::endl;
}

bool WorldCreator::createOctree()
{
  if (!canCreateOctomap)
  {
    std::cout << "Cannot create octree files, because not all necessary parameters have been set. Need 'resolution' and at least one object." << std::endl;
    return false;
  }

  octree = new octomap::OcTree(resolution);

  for (int i = 0; i < boxes.size(); ++i)
  {
    addOctreeBox(*octree, boxes[i]);
    if (!ros::ok())
    {
      puts("Terminated. No octomap created!\n");
      return false;
    }
  }

  for (int i = 0; i < spheres.size(); ++i)
  {
    addOctreeSphere(*octree, spheres[i]);
    if (!ros::ok())
    {
      puts("Terminated. No octomap created!\n");
      return false;
    }
  }

  for (int i = 0; i < cylinders.size(); ++i)
  {
    addOctreeCylinder(*octree, cylinders[i]);
    if (!ros::ok())
    {
      puts("Terminated. No octomap created!\n");
      return false;
    }
  }

  double dummy;
  octree->getMetricMin(minX, minY, dummy);
  octree->getMetricMax(maxX, maxY, dummy);

  if(addFloor)
  {
    ObjectBox box;
    box.angle = 0.0;
    box.name = "floor";
    box.size[0] = maxX - minX;
    box.size[1] = maxY - minY;
    box.size[2] = resolution;
    box.bottomCenter[0] = (maxX + minX)*0.5;
    box.bottomCenter[1] = (maxY + minY)*0.5;
    box.bottomCenter[2] = minZ - resolution;
    addOctreeBox(*octree, box);
    if (!ros::ok())
    {
      puts("Terminated. No octomap created!\n");
      return false;
    }
  }

  octree->prune();

  freopen("/dev/null", "w", stderr);
  octree->writeBinary(fileName + ".bt");
  octree->write(fileName + ".ot");
  return true;
}

void WorldCreator::addOctreeBox(octomap::OcTree &octree, ObjectBox &box)
{
  double minPoint[3], maxPoint[3];
  const double sizeMax = 0.5 * std::max(box.size[0], box.size[1]);

  minPoint[0] = resolution * (std::floor((box.bottomCenter[0] - sizeMax) / resolution)) + resolution * 0.49;
  minPoint[1] = resolution * (std::floor((box.bottomCenter[1] - sizeMax) / resolution)) + resolution * 0.49;
  minPoint[2] = resolution * (std::floor(box.bottomCenter[2] / resolution)) + resolution * 0.49;

  maxPoint[0] = resolution * (std::ceil((box.bottomCenter[0] + sizeMax) / resolution)) - resolution * 0.49;
  maxPoint[1] = resolution * (std::ceil((box.bottomCenter[1] + sizeMax) / resolution)) - resolution * 0.49;
  maxPoint[2] = resolution * (std::ceil((box.bottomCenter[2] + box.size[2]) / resolution)) - resolution * 0.49;

  for (double x = minPoint[0]; x <= maxPoint[0]; x += resolution)
  {
    for (double y = minPoint[1]; y <= maxPoint[1]; y += resolution)
    {
      for (double z = minPoint[2]; z <= maxPoint[2]; z += resolution)
      {
        const double xTrans = (x - box.bottomCenter[0]) * cos(-box.angle) - (y - box.bottomCenter[1]) * sin(-box.angle);
        const double yTrans = (x - box.bottomCenter[0]) * sin(-box.angle) + (y - box.bottomCenter[1]) * cos(-box.angle);

        if (xTrans >= -0.5 * box.size[0] && xTrans <= 0.5 * box.size[0] && yTrans >= -0.5 * box.size[1] && yTrans <= 0.5 * box.size[1]
            && z >= box.bottomCenter[2] && z <= box.bottomCenter[2] + box.size[2])
          octree.updateNode(octomap::point3d(x, y, z), true, true);
      }
    }
  }
}

void WorldCreator::addOctreeSphere(octomap::OcTree &octree, ObjectSphere &sphere)
{
  double minPoint[3], maxPoint[3];

  minPoint[0] = resolution * (std::floor((sphere.bottom[0] - sphere.radius) / resolution)) + resolution * 0.49;
  minPoint[1] = resolution * (std::floor((sphere.bottom[1] - sphere.radius) / resolution)) + resolution * 0.49;
  minPoint[2] = resolution * (std::floor((sphere.bottom[2]) / resolution)) + resolution * 0.49;

  maxPoint[0] = resolution * (std::ceil((sphere.bottom[0] + sphere.radius) / resolution)) - resolution * 0.49;
  maxPoint[1] = resolution * (std::ceil((sphere.bottom[1] + sphere.radius) / resolution)) - resolution * 0.49;
  maxPoint[2] = resolution * (std::ceil((sphere.bottom[2] + 2 * sphere.radius) / resolution)) - resolution * 0.49;

  for (double x = minPoint[0]; x <= maxPoint[0]; x += resolution)
  {
    for (double y = minPoint[1]; y <= maxPoint[1]; y += resolution)
    {
      for (double z = minPoint[2]; z <= maxPoint[2]; z += resolution)
      {
        if ((x - sphere.bottom[0]) * (x - sphere.bottom[0]) + (y - sphere.bottom[1]) * (y - sphere.bottom[1])
            + (z - sphere.bottom[2] - sphere.radius) * (z - sphere.bottom[2] - sphere.radius) <= sphere.radius * sphere.radius)
          octree.updateNode(octomap::point3d(x, y, z), true, true);
      }
    }
  }
}

void WorldCreator::addOctreeCylinder(octomap::OcTree &octree, ObjectCylinder &cylinder)
{
  double minPoint[3], maxPoint[3];

  minPoint[0] = resolution * (std::floor((cylinder.bottom[0] - cylinder.radius) / resolution)) + resolution * 0.49;
  minPoint[1] = resolution * (std::floor((cylinder.bottom[1] - cylinder.radius) / resolution)) + resolution * 0.49;
  minPoint[2] = resolution * (std::floor((cylinder.bottom[2]) / resolution)) + resolution * 0.49;

  maxPoint[0] = resolution * (std::ceil((cylinder.bottom[0] + cylinder.radius) / resolution)) - resolution * 0.49;
  maxPoint[1] = resolution * (std::ceil((cylinder.bottom[1] + cylinder.radius) / resolution)) - resolution * 0.49;
  maxPoint[2] = resolution * (std::ceil((cylinder.bottom[2] + cylinder.height) / resolution)) - resolution * 0.49;

  for (double x = minPoint[0]; x <= maxPoint[0]; x += resolution)
  {
    for (double y = minPoint[1]; y <= maxPoint[1]; y += resolution)
    {
      for (double z = minPoint[2]; z <= maxPoint[2]; z += resolution)
      {
        if ((x - cylinder.bottom[0]) * (x - cylinder.bottom[0]) + (y - cylinder.bottom[1]) * (y - cylinder.bottom[1]) <= cylinder.radius * cylinder.radius
            && z <= cylinder.bottom[2] + cylinder.height && z >= cylinder.bottom[2])
          octree.updateNode(octomap::point3d(x, y, z), true, true);
      }
    }
  }
}

bool WorldCreator::createPNG()
{
  if (system("which pnmtopng > /dev/null 2>&1"))
  {
    std::cout << "pnmtopng is not installed on your system. Install it via '$ sudo apt-get install netpbm'" << std::endl;
    return false;
  }

  if (!canCreatePNG)
  {
    std::cout << "Cannot create png, because not all necessary parameters have been set. Need 'resolution' and at least one object." << std::endl;
    return false;
  }

  if (octree == NULL)
    createOctree();

  createOccupancyMapFromOctomap();

  std::string fileNamePNM = fileName + ".pnm";
  std::ofstream file(fileNamePNM.c_str());
  writeDataToPNM(file);
  file.close();

  std::string command = "pnmtopng " + fileNamePNM + " > " + fileName + ".png";
  system(command.c_str());
  std::remove(fileNamePNM.c_str());
  return true;
}

void WorldCreator::createOccupancyMapFromOctomap()
{
  int octreeKeyMinX, octreeKeyMinY, octreeKeyMinZ;
  int octreeKeyMaxX, octreeKeyMaxY, octreeKeyMaxZ;

  double mapResolutionRecip = 1.0 / resolution;

  octomap::OcTreeKey keyMinPlane = octree->coordToKey(minX + resolution * 0.5, minY + resolution * 0.5, minZ + resolution * 0.5);
  octomap::OcTreeKey keyMaxPlane = octree->coordToKey(maxX - resolution * 0.5, maxY - resolution * 0.5, maxZ - resolution * 0.5);

  octreeKeyMinX = keyMinPlane[0];
  octreeKeyMinY = keyMinPlane[1];
  octreeKeyMaxX = keyMaxPlane[0];
  octreeKeyMaxY = keyMaxPlane[1];
  octreeKeyMinZ = keyMinPlane[2];
  octreeKeyMaxZ = keyMaxPlane[2];

  occupancyMap.clear();
  occupancyMap.resize(octreeKeyMaxX - octreeKeyMinX + 1, std::vector<bool>(octreeKeyMaxY - octreeKeyMinY + 1, false));

  octomap::OcTreeKey key;
  for (int x = 0; x < occupancyMap.size(); ++x)
  {
    for (int y = 0; y < occupancyMap[0].size(); ++y)
    {
      for (int z = 0; z < octreeKeyMaxZ - octreeKeyMinZ; ++z)
      {
        key[0] = x + octreeKeyMinX;
        key[1] = y + octreeKeyMinY;
        key[2] = z + octreeKeyMinZ;
        if (isOctreeNodeOccupied(key))
        {
          occupancyMap[x][y] = true;
          z = octreeKeyMaxZ - octreeKeyMinZ;
        }
      }
    }
  }
}

bool WorldCreator::isOctreeNodeOccupied(const octomap::OcTreeKey &key) const
{
  octomap::OcTreeNode* node = octree->search(key);

  if (node != NULL)
    return octree->isNodeOccupied(node);
  else
    return false;
}

void WorldCreator::writeDataToPNM(std::ofstream &file)
{
  file << "P1" << std::endl;
  file << occupancyMap[0].size() << " " << occupancyMap.size() << std::endl;

  for (int x = 0; x < occupancyMap.size(); ++x)
  {
    for (int y = 0; y < occupancyMap[0].size(); ++y)
    {
      if (occupancyMap[x][y])
      {
        file << "1 ";
      }
      else
      {
        file << "0 ";
      }
    }
    file << std::endl;
  }
}
