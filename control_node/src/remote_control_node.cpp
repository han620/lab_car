#include "control_node/remote_control_node.h"

RemoteControlNode::RemoteControlNode(){
    ros::NodeHandle nh;
    std::string path = "config file path";
    if(LoadConfig(path)){
        node_state = State::HAVE_CONFIG;
    }else{
        ROS_WARN("remote control _node: Config failed");
    }
    state_pub = nh.advertise<robot_msgs::PerceptionNodeMsg>("remote_control_state",10);
    cmd_sub = nh.subscribe<robot_msgs::Cmd>("remote_control_cmd",10,&RemoteControlNode::CmdCallback,this);
}


bool RemoteControlNode::LoadConfig(std::string file){
    ROS_INFO("remote control _node: load config");
    //TODO 应该在这里读取配置文件，目前只初始化更新频率和节点名称；
    update_frequence = 10;
    node_name = "remote control node";
}

void RemoteControlNode::UpdateState(){
    ros::NodeHandle nh;
    ros::Rate loop(update_frequence);
    double start_time = ros::Time().now().toSec();
    ROS_INFO("remote control _node: state thread start");
    while(nh.ok() && node_state!=State::EXIT){
        robot_msgs::PerceptionNodeMsg msg;
        //TODO 目前获取的是ros秒可能需要进一步处理
        msg.time = ros::Time().fromSec(ros::Time().now().toSec() - start_time);
        msg.node_name = "driver_node";
        msg.state.state = (uint8_t)node_state;
        //msg.state.pose = 
        state_pub.publish(msg);
        loop.sleep();
        ros::spinOnce();
    }
}


State RemoteControlNode::Start(){
    ROS_INFO("remote control source running");
    return State::RUNNING;
}
//Stop需要reset参数
State RemoteControlNode::Stop(){
    ROS_INFO("remote control source stop");
    return State::STOP;
}
//退出需要清理线程
State RemoteControlNode::Exit(){
    ROS_INFO("remote control source exit");
    state_pub.shutdown();
    cmd_sub.shutdown();
    return State::EXIT;
}
//pause和resume不需要对数据进行处理
State RemoteControlNode::Pause(){
    if(node_state!=State::RUNNING)
        return node_state;
    ROS_INFO("remote control source pause");
    return State::PAUSED;
}
State RemoteControlNode::Resume(){
    if(node_state!=State::PAUSED)
        return node_state;
    ROS_INFO("remote control source resume");
    return State::RUNNING;
}

void RemoteControlNode::CmdCallback(const robot_msgs::CmdConstPtr &msg){
    ROS_INFO("remote control  source get command");
    switch(msg->cmd){
        case (int)Cmd::START : node_state = Start();  break;
        case (int)Cmd::PAUSE : node_state = Pause();  break;
        case (int)Cmd::STOP  : node_state = Stop();   break;
        case (int)Cmd::RESUME: node_state = Resume(); break;
        case (int)Cmd::EXIT  : node_state = Exit();   break;
    }
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "remote control source");
    RemoteControlNode remote_control_node;
    remote_control_node.UpdateState();
    return 0;
}


