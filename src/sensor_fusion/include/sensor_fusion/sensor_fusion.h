#pragma once

/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2017, Torc Robotics, LLC
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Torc Robotics, LLC nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */



#include <cav_srvs/GetDriversWithCapabilities.h>
#include <cav_srvs/Bind.h>

#include <nav_msgs/Odometry.h>
#include <sensor_msgs/NavSatFix.h>
#include <geometry_msgs/TwistStamped.h>
#include <cav_msgs/HeadingStamped.h>
#include <cav_msgs/ExternalObjectList.h>
#include <cav_msgs/BSMCoreData.h>


#include <boost/bind.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <bondcpp/bond.h>

#include <ros/ros.h>

#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <queue>



/**
 * @brief A ROS node that monitors multiple sources to produce a filtered version
 *
 * This class monitors all drivers that provide position and tracked objects api*
 *
 */
class SensorFusionApplication
{
public:

    /**
     * @brief Initializes the ROS context of this node
     * @param argc Command line argument count
     * @param argv Command line arguments
     * @param name Name of the node
     */
    SensorFusionApplication(int argc, char** argv, std::string name="sensor_fusion") : uuid_(boost::uuids::random_generator()())
    {
        ros::init(argc, argv, name);
    }


    /**
     * @brief Main process function
     * @return 0 on Success
     */
    int run();

private:

    std::unique_ptr<ros::NodeHandle> nh_;
    const boost::uuids::uuid uuid_;
    std::unordered_map<std::string, std::unique_ptr<bond::Bond>> bond_map_;

    /**
     * @brief This function is the bond call back for on_broken event    *
     * @param node_name name of the node this callback is firing for
     *
     * When a bond is broken , we clear our record of the bond destroying our reference to the bond
     */
    void on_broken_cb(const std::string& node_name)
    {
        bond_map_.erase(node_name);
    }

    /**
     * @brief This functions is the bond callback for on_connected event
     * @param node_name name of the node this callback is firing for
     *
     * Right now this is only used for debugging
     */

    void on_conneced_cb(const std::string& node_name)
    {
        ROS_DEBUG_STREAM("Bonded to " << node_name);
    }



    /**
     * @brief Handles the call to the interface manager to receive api of the given node
     * @param type The driver_type we are querying
     * @param name THe name of the service to query
     * @return A vector containing the FQN of the services queried
     */
    std::vector<std::string> get_api(const cav_srvs::GetDriversWithCapabilitiesRequest::_category_type type, const std::string& name);

    std::unordered_map<std::string, ros::Subscriber> sub_map_;
    /**
     * @brief Manages subscribing to driver services
     *
     * This should be called periodically to maintain updates
     */
    void get_services();

    ros::Publisher odom_pub_, navsatfix_pub_, heading_pub_, velocity_pub_, objects_pub_, vehicles_pub_;

    /**
     * @brief Publishes the filtered updates
     *
     * At the moment since this is just a skeleton application
     * we are only re-publishing the receive data
     *
     * Later this will publish processed data
     */
    void publish_updates();

    std::unordered_map<std::string, nav_msgs::OdometryConstPtr> odom_map_;
    void odom_cb(const ros::MessageEvent<nav_msgs::Odometry const>& event)
    {
        std::string name = event.getPublisherName();
        odom_map_[name] = event.getMessage();
    }

    std::unordered_map<std::string, sensor_msgs::NavSatFixConstPtr> navsatfix_map_;
    void navsatfix_cb(const ros::MessageEvent<sensor_msgs::NavSatFix const>& event)
    {
        std::string name = event.getPublisherName();
        navsatfix_map_[name] = event.getMessage();
    }

    std::unordered_map<std::string, cav_msgs::HeadingStampedConstPtr> heading_map_;
    void heading_cb(const ros::MessageEvent<cav_msgs::HeadingStamped>& event)
    {
        std::string name = event.getPublisherName();
        heading_map_[name] = event.getMessage();
    }

    std::unordered_map<std::string, geometry_msgs::TwistStampedConstPtr> velocity_map_;
    void velocity_cb(const ros::MessageEvent<geometry_msgs::TwistStamped>& event)
    {
        std::string name = event.getPublisherName();
        velocity_map_[name] = event.getMessage();
    }

    std::unordered_map<std::string, cav_msgs::ExternalObjectListConstPtr> objects_map_;
    void trackedobjects_cb(const ros::MessageEvent<cav_msgs::ExternalObjectList>& event)
    {
        std::string name = event.getPublisherName();
        objects_map_[name] = event.getMessage();
    }

    std::queue<cav_msgs::BSMCoreDataConstPtr> bsm_q_;
    void bsm_cb(const cav_msgs::BSMCoreDataConstPtr& msg)
    {
        bsm_q_.push(msg);

    }
};