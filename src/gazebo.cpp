/* Position Feedback Controller for Turtlebot
 *
 * Copyright (C) 2015 Jafar Qutteineh. All rights reserved.
 * License (3-Cluase BSD): TODO: Add github link
 *
 * This code uses and is based on code from:
 *   Project: trychomp https://github.com/poftwaresatent/trychomp
 *   Copyright (C) 2014 Roland Philippsen. All rights reserved.
 *   License (3-Clause BSD) : https://github.com/poftwaresatent/trychomp
 * **
 * \file chomp.cpp
 *
 * CHOMP for point vehicles (x,y) moving holonomously in the plane. It will
 * plan a trajectory (xi) connecting start point (qs) to end point (qe) while
 * avoiding obstacles (obs)
 */

#include <stdlib.h>
#include <ros/ros.h>
#include <cargo_ants_msgs/Goal.h>
#include <cargo_ants_msgs/ObstacleMap.h>
#include <cargo_ants_msgs/Obstacle.h>
#include <cargo_ants_msgs/Origin.h>
#include "gazebo_msgs/ModelStates.h"
#include "std_msgs/UInt32.h"
#include "cargo_ants_msgs/Path.h"
#include <math.h>

#define CONTROL_LOOP_RATE 10
#define VEHICLE_NAME  "mobile_base"
#define GOAL_NAME  "Goal_Cone"
#define OBSTACLE_NAME  "Obs"
#define OBSTACLE_RADIUS 1.5

cargo_ants_msgs::Path path;
cargo_ants_msgs::ObstacleMap obstacle_map;

void model_states_callback(const gazebo_msgs::ModelStates::ConstPtr &msg)
{
  int obs_prefix_length = std::string(OBSTACLE_NAME).length();
  path.goals.clear();
  obstacle_map.obstacles.clear();
  std::vector<cargo_ants_msgs::Obstacle> obstacles;
  for(std::vector<std::string>::size_type ii = 0; ii != msg->name.size(); ++ii) {
    if (msg->name[ii] == VEHICLE_NAME){
      cargo_ants_msgs::Goal start_goal;
      start_goal.gx = msg->pose[ii].position.x;
      start_goal.gy = msg->pose[ii].position.y;
      start_goal.gth = asin(msg->pose[ii].orientation.z) * 2.0;
      path.goals.insert(path.goals.begin(),start_goal);
    }
    else if (msg->name[ii] == GOAL_NAME){
      cargo_ants_msgs::Goal end_goal;
      end_goal.gx = msg->pose[ii].position.x;
      end_goal.gy = msg->pose[ii].position.y;
      end_goal.gth = asin(msg->pose[ii].orientation.z) * 2.0;
      path.goals.push_back(end_goal);
    }
    else if (msg->name[ii].substr(0,obs_prefix_length) == OBSTACLE_NAME){
      cargo_ants_msgs::Obstacle obstacle;
      obstacle.origin.ox = msg->pose[ii].position.x;
      obstacle.origin.oy = msg->pose[ii].position.y;
      //Some cheating is happening here. "oth" is used to express obstacle radius.
      obstacle.origin.oth = OBSTACLE_RADIUS;
      obstacle_map.obstacles.push_back(obstacle);
    }
  }
}


int main(int argc, char **argv)
{
	ros::init(argc, argv, "chomp_scene_descriptor_gazebo");
  ros::NodeHandle node;
	ros::Publisher obs_pub = node.advertise<cargo_ants_msgs::ObstacleMap> (
                            "/obstacles", 1);
  ros::Publisher path_pub = node.advertise<cargo_ants_msgs::Path> (
                                              "/path_planner", 100);
	ros::Subscriber odom_sub = node.subscribe("/gazebo/model_states",
                                              1000, model_states_callback);
  ros::Subscriber goal_idx_sub = node.subscribe("/pf_controller/current_goal_idx",
                                              1000, model_states_callback);
  ros::Rate loop_rate(CONTROL_LOOP_RATE);

	ROS_INFO("CHOMP Scene Descriptor started");


  size_t last_goal_idx = 0;
	while (ros::ok()) {
		ros::spinOnce();
    path_pub.publish(path);
    obs_pub.publish(obstacle_map);
		loop_rate.sleep();
	}
	return 0;
}
