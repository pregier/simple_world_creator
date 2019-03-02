#include <simple_world_creator/simple_world_creator.h>

int main(int argc, char* argv[])
{
  ros::init(argc, argv, "simple_world_creator");
  ros::NodeHandle nh;

  if (argc < 3)
  {
    printf("Usage: simple_world_creator <file> [WORLDS]     ([WORLDS] may include '--octomap', '--gazebo', and '--png')\n");
    printf("\n");
    return 0;
  }

  std::string fileName;
  for (int i = 1; i < argc; ++i)
  {
    std::string s(argv[i]);
    if (s[0] == '-')
      continue;
    fileName = s;
  }

  WorldCreator worldCreator(fileName);

  if (!worldCreator.foundConfig)
  {
    ROS_ERROR("Issue reading the config file. Could not create world files.");
    return 0;
  }

  worldCreator.setCreatePossibilities();

  for (int i = 1; i < argc; ++i)
  {
    std::string s(argv[i]);
    if (s == "--octomap")
    {
      ROS_INFO("Creating octomap...");
      worldCreator.createOctree();
      ROS_INFO("Done!");
    }
    else if (s == "--gazebo")
    {
      ROS_INFO("Creating gazebo world file...");
      worldCreator.createGazeboWorldFile();
      ROS_INFO("Done!");
    }
    else if(s == "--png")
    {
      ROS_INFO("Creating png...");
      worldCreator.createPNG();
      ROS_INFO("Done!");
    }
  }

  return 0;
}
