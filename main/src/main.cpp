// #include <main.h>
#include <iostream>
#include <cmath>
#include <string>
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/String.h>
#include <std_msgs/Bool.h>
#include <std_msgs/Float32.h>
#include <std_msgs/Int16MultiArray.h>
#include <std_msgs/Int8MultiArray.h>
#include <std_msgs/UInt8.h>
#include <std_msgs/Int8.h>
#include <std_msgs/UInt16.h>
#include <std_msgs/Int16.h>


int game_status = 0;//game_status = [1]:Robot is starting zone;
                    //              [2]:Robot is retry zone;    
                    //              [0]:Robot is stopped;
bool game_side  = 0;
int status = 0; 
int prev_status = 0;
std::string ongo;//="blue";
std::string ulaan ="red";
int ehlel = 0 ;
float theta = 0;
int left_distance = 0;
int front_distance=0;
int right_distance=0;
int back_distance=0;
int distance=0;
bool start_cmd=false;

int interrupt_cnt=0;

int nearest_ball=0;
int camera_point_x=300;
int camera_point_y=-430;
int shortest_distance_x=0;
int shortest_distance_y=0;
bool take_ball=false;
bool crushing_flag = false;
float shortest_theta=0;
int u,w;
int min_val=1000;

bool recieved_color=false;
bool robot_ball = false;

int16_t robot_x=0;
int16_t robot_y=0;
int16_t robot_theta=0;

int last_x = 0;
int last_y = 0;
int last_theta = 0;
int true_ball = 0;
int prev_theta = 0;
int now_x=0;
int now_y=0;
int now_theta=0;

int back_x = 0;
int back_y = 0;
int back_theta = 0;
float last_error_x=0;
float last_error_y=0;
float last_error_z=0;
float i_error_x=0;
float i_error_y=0;
float i_error_z=0;
std::vector<int8_t> silo_vec={1,1,1,1,1};
int silo_dist = 221;
bool ir_ball = false;
bool flag = false;

int silo_data_dist;
int silo_flag=0;

// geometry_msgs::Twist vel_pub;
geometry_msgs::Twist vel_pub;
std_msgs::Int8 BLDC;
std_msgs::UInt16 dcPos;
std_msgs::Int16 reset_cordinate;
std_msgs::Int8 silo_data_stop;

void velocity_publishing(int x, int y, int z){
    vel_pub.linear.x = x;
    vel_pub.linear.y = y;
    vel_pub.angular.z= z;
}

void printing_vel(int stat, int x, int y, int z){
    ROS_INFO("Game status is %d", stat);
    ROS_INFO("ROBOT hashlaganas[%d] zaita bna", x);
    ROS_INFO("ROBOT uragshaa [%d] zaitai bna.", y);
    ROS_INFO("ROBOT [%d] gradus hazaij bna.", z);
}
int PID(double kp, double ki, double kd, float setpoint, float processVariable, float max_speed, float min_speed, float *last_error, float *i_error)
{
    double error;
    double out;
    double d_error;
    error=setpoint-processVariable;
    d_error= (*last_error)-error;
    *last_error=error;
    *i_error=(*i_error)+error;
    out=kp*error + (*i_error)*ki + d_error*kd;
        if(out > max_speed)
        out = max_speed;
        if(out < -max_speed)
        out = -max_speed;
        if(out < min_speed && out>0){
            out = min_speed;
        }
        if(out>-min_speed && out<0){
            out = -min_speed;
        }
        return out;
}


void camera_detect(const std_msgs::String::ConstPtr& ball_detect)
{
    // // Split the read data by space
    
    std::istringstream iss(ball_detect->data);
    std::vector<std::string> tokens;
    std::string token;
    std::vector<int16_t> area;
    std::string color; 
    std::vector<int> percentage;
    // std::vector<std::vector<std::string>> filteredPatterns;
    // std::string ongo = "blue";//ali tald togloh ve gedeg ongo
    int counter = 0;
    int min_val = 1000;
    int flag_counter=0;
    // float theta_ball=0;
    
    while(getline(iss, token, ' ')){
        tokens.push_back(token);
    }
    for (const auto& token : tokens) {
        try {    
            int f = std::stoi(token); // Convert token to int
            if(counter==0){              
                shortest_distance_x = f;
                counter = 1;    
            }else{
                shortest_distance_y = -f;
            }
            
        } catch (const std::invalid_argument& e) {
            counter = 0;
            color = token;
            nearest_ball = (shortest_distance_y-camera_point_y)/2 +  sqrt(pow(shortest_distance_x - camera_point_x, 2) + pow(shortest_distance_y - camera_point_y, 2));
            if((((ongo.compare(color)) == 0) || (ulaan.compare(color) == 0))  /*|| (ulaan.compare(color)) == 0*/){//(ongo.compare(color)) == 0) ||
                // flag = true;
                take_ball = true;
                flag_counter++;
                if(min_val >= nearest_ball){
                    u = shortest_distance_x;
                    w = shortest_distance_y;
                    min_val = nearest_ball;//herev manai togloh taliin bombog haragdsan uyed hamgin oirhon zaitai bombogruu yvna
                    std::cout<<u<<","<<w<<","<<min_val<<"-----"<<nearest_ball<<std::endl;   
                }                                                                                                                                                                                       
            }
        }
    }
    if(interrupt_cnt  > 15){
        if(flag_counter != 0){
            take_ball = true;
        }else{
            take_ball = false;
        }
        interrupt_cnt = 0;
    }
    interrupt_cnt++;
    
    
}

// LIDARIIN DATA IREH URD HOID BARUN ZUUN TALIN ZAIG HEMJIH
void wall_clk(const std_msgs::Int16MultiArray::ConstPtr& wall_data)
{
   left_distance = wall_data->data[0];
   right_distance =wall_data->data[1];
   theta=wall_data->data[2];
   front_distance =wall_data->data[3];
   back_distance = wall_data->data[4];

//    std::cout<<wall_data->data[5]<<std::endl;
   if(ongo=="red"){
        if(wall_data->data[4] < 60){
            ehlel = 1;
        }else{
            ehlel = 2;
        }
   }
   else{
        if(wall_data->data[3] < 60){
            ehlel = 1;
        }
        else{
            ehlel = 2;
        }
   }
}

// ROBOTIN BAIRLAL OLOH HAVTANGAAS IREH /encoder TOPIC
void encoder_call(const std_msgs::Int16MultiArray::ConstPtr& encoder_data){
    robot_theta = encoder_data->data[2];
    robot_x = encoder_data->data[3];
    robot_y = encoder_data->data[4];
    if((encoder_data->data[0] != 0) && encoder_data->data[1]){
        if(!recieved_color){
            recieved_color = true;
            if(encoder_data->data[0] >= encoder_data->data[1]){
                ongo = "red";
            }else{
                ongo = "blue";
            }
        }
    }
}

void sensor_clk(const std_msgs::Int8MultiArray::ConstPtr& sensor_data)
{
    ROS_INFO("%d", sensor_data->data[0]);
    // if(sensor_data->data[0]==48 || sensor_data->data[0]==60 ){
    //     robot_ball = true;
    // }
    if(((sensor_data->data[0] & 0b00010000) == 0b00010000) || ((sensor_data->data[0] & 0b00100000) == 0b00100000)){
        robot_ball = true;
    }
    else{
        robot_ball = false;
    }
    true_ball = sensor_data->data[1];// 0 = purple, 1 = blue, 2 = red.
    // if(sensor_data->data[0] == 64||sensor_data->data[0]==65||sensor_data->data[0]==66||sensor_data->data[0]==67){
    //     ir_ball = true;
    // }
    if((sensor_data->data[0] & 0b01000000) == 0b01000000){
        ir_ball = true;
    }
    else{
        ir_ball = false;
    }
}

// ROBOT EHLEH COMMAND HULEEH HESEG
void key_changed(const std_msgs::String::ConstPtr& msg)//this flag is start or stop 
{
    ROS_ERROR("[Admin] key pressed: [%s]", msg->data.c_str());
    if(msg->data == "s"){//start 
        ROS_INFO("[Admin] key pressed: [%s]", msg->data.c_str());
        start_cmd = true;
        game_status = 2;
    }
    else if(msg->data == "r"){//retry
        ROS_INFO("[Admin] key pressed: [%s]", msg->data.c_str());
        start_cmd = true;
        // reset_cordinate.data = 0;
        // res_encoder.publish(reset_cordinate);
        // dcPos.data=150;
        // dc_pos.publish(dcPos);
        game_status = 3; 
    }else{
    start_cmd = false;
    }
}
void silo_callback(const std_msgs::Int16MultiArray::ConstPtr &silo_data){
    silo_data_dist=silo_data->data[1];
}

// void silo_stack(const std_msgs::Int8MultiArray::ConstPtr &silo_number){
// //     // std::vector<int8_t>silo;
//     silo_vec.push_back(silo_number->data[4]);
//     silo_vec.push_back(silo_number->data[3]);
//     silo_vec.push_back(silo_number->data[2]);
//     silo_vec.push_back(silo_number->data[1]);
//     silo_vec.push_back(silo_number->data[0]);
// //     // std::vector<int8_t> silo = silo_number->data;
// }
void silo_songolt_hiih(const std_msgs::Int8::ConstPtr &songolt_data){
    // silo_dist = (songolt_data->data-1) * 75 + 50;
    // silo_dist = (silo.back() - 1) * 75 + 50;
    switch ((songolt_data->data) + 1)
    {
        case 1:
            silo_dist=73;
            break;
        case 2:
            silo_dist=145;
            break;
        case 3:
            silo_dist=221;
            break;
        case 4:
            silo_dist=295;
            break;
        case 5:
            silo_dist=371;
            break;

    }   
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "MAIN_PROGRAMM");
    ROS_INFO("Main programm is working now!!!");
    ros::NodeHandle nh;
   
    // int counter = 1;
    
    ros::Subscriber  keychecker_sub = nh.subscribe("/key_checker", 10, &key_changed);//waiting starting commmand(from python code or STM32 IR sensor)
    ros::Subscriber back = nh.subscribe("/wall", 10, &wall_clk);//checking back side Start or Retry zone    
    ros::Subscriber sensor_status = nh.subscribe("/sensor", 100, &sensor_clk);
    ros::Publisher speed = nh.advertise<geometry_msgs::Twist>("/cmd_vel", 100);
    ros::Subscriber camera = nh.subscribe("/camera_data", 10, &camera_detect);//from darknet 
    // ros::Subscriber camera_hor = nh.subscribe("/silo_data", 100, &camera_detect);//from darknet _horizontal
    ros::Subscriber silo_songolt = nh.subscribe("/silo_detection", 1, silo_songolt_hiih);
    // ros::Subscriber silo_detection = nh.subscribe("/silo_detection/buffer", 10, silo_stack);
    ros::Publisher brushless = nh.advertise<std_msgs::Int8>("/brushdc", 20);//brushless motoriin 
    ros::Subscriber encoder = nh.subscribe("/encoder", 10, &encoder_call);
    ros::Publisher  dc_pos = nh.advertise<std_msgs::UInt16>("pos_cmd", 10);
    ros::Publisher res_encoder = nh.advertise<std_msgs::Int16>("coordinate", 10);
    ros::Subscriber silo_index = nh.subscribe("silo_index", 128, silo_callback);
    ros::Publisher silo_stop = nh.advertise<std_msgs::Int8>("silo_stop", 10);
    
    // silo_dist = (silo_vec.back() - 1) * 75 + 50;
    // switch (silo_vec.back())
    // {
    //     case 1:
    //         silo_dist=73;
    //         break;
    //     case 2:
    //         silo_dist=145;
    //         break;
    //     case 3:
    //         silo_dist=221;
    //         break;
    //     case 4:
    //         silo_dist=295;
    //         break;
    //     case 5:
    //         silo_dist=371;
    //         break;

    // }   
    // silo_vec.pop_back();
    // silo_vec.cle

    ros::Rate loop_rate(100);  
    while(ros::ok()){
        // system("clear");
        ROS_WARN("Game status = [%d]", game_status);
        ROS_WARN("Games  side : [%s]", ongo.c_str());
        if(game_status < 5){
            if(game_status == 0){
                    if(ehlel == 1){
                        game_status = 1;
                        prev_status = 0;
                        velocity_publishing(0, 0, 0);
                        speed.publish(vel_pub);
                    }else if(ehlel == 2){
                        game_status = 3;
                        prev_status = 0;
                    }
            }
            
            else if(game_status == 1){
                
                if(start_cmd){ 
                    game_status = 2;
                }else{
                    velocity_publishing(0,0,0);
                   // vel_pub.angular.z= PID(0.6, 0.0, 0.000, 90, theta, 127, 20);
                    speed.publish(vel_pub);
                    dcPos.data=150;
                    dc_pos.publish(dcPos);
                    reset_cordinate.data = 0;
                    res_encoder.publish(reset_cordinate);       
                    system("clear");    
                    ROS_WARN_ONCE("The robot is wating start command!!!!");
                }

            }
            
            else if(game_status == 2){
                //start to retry distance
                ROS_WARN_ONCE("Robot is moving");
                int left_setpoint=20;
                int op=-1;
                if(ongo=="red"){
                    op=-1;
                    distance=front_distance;
                }
                else if(ongo=="blue"){
                    op=1;
                    distance=back_distance;
                }
                prev_status = 2;
                if(distance > 150 || distance==0){
                    ROS_INFO("The distance is [150] of [%d]", distance);
                    vel_pub.linear.x = PID(1.4, 0, 0.800, left_setpoint,left_distance, 127, 5, &i_error_x, &last_error_x);//int utga butsaana
                    vel_pub.linear.y =  op*127;//127 baisan
                    vel_pub.angular.z= -PID(2.0,  0.0, 1.000, 90, theta, 127, 5, &i_error_z, &last_error_z);//int utga butsaana
                    speed.publish(vel_pub);
                    last_y = robot_y - op*250;
                }else{
                    game_status=19;
                }
            }
            
            
            else if(game_status == 3){

                BLDC.data = 0;
                brushless.publish(BLDC);
                // vel_pub.data[3] = 0;
                speed.publish(vel_pub);
                dcPos.data=150;
                dc_pos.publish(dcPos);  
                reset_cordinate.data = 0;
                res_encoder.publish(reset_cordinate); 
                if(start_cmd){
                    game_status = 22;
                }else{
                    ROS_WARN_ONCE("The ROBOT waiting start cmd!!!");
                    ROS_WARN_ONCE("The ROBOT IS IN RETRY!!!");
                    velocity_publishing(0,0,0);
                    // vel_pub.linear.x = 0;
                    // vel_pub.linear.y = 0;    
                    // vel_pub.angular.z = PID(0.6, 0.0, 0.000, 90, theta, 127, 20);
                   
                }
            }
            
            else if (game_status == 4){
                
                int front_setpoint=60;
                int left_setpoint=30;
                int op=1;
                if(ongo=="red"){
                    distance=front_distance;
                    op=1;
                    front_setpoint=65;//80 baisan
                }
                else if(ongo=="blue"){
                    distance=back_distance;
                    op=-1;
                    front_setpoint=60;
                }
                if(distance<200){
                    left_setpoint=402;
                }
                else{
                    left_setpoint=120;
                }
                if(vel_pub.linear.x < 10 && vel_pub.linear.x > -10  && theta > 88 && theta < 92 && left_distance >= 396 && left_distance <= 405){
                    game_status = 20;
                }else{
                    ROS_INFO("left distance = %d", left_distance);
                    ROS_INFO("front distance = %d", front_distance);
                    ROS_INFO("thete of left = %f", theta);
                    if(prev_status == 22){
                        vel_pub.linear.x = PID(0.8, 0.0, 0.4, left_setpoint, left_distance, 127, 9, &i_error_x, &last_error_x); 
                    }else{
                        vel_pub.linear.x = PID(0.8, 0.0, 0.8, left_setpoint, left_distance, 127, 9, &i_error_x, &last_error_x); 
                    }
                    
                    vel_pub.linear.y = op*PID(1.2, 0.0, 0.700, front_setpoint, distance, 127, 3, &i_error_y, &last_error_y);
                    vel_pub.angular.z = -PID(4.3, 0.0, 1.8, 90, theta, 127, 10, &i_error_z, &last_error_z);
                    speed.publish(vel_pub);
                    last_y = robot_y + op*300;
                    last_theta =robot_theta;
                }
                // }
            }
        }else{
            // ROS_WARN("ROBOT IS IN AREA 3!!!");

        // // ROBOT AREA 3 DEER GARAAD BOMBOGRUU HARNA
            if(game_status == 5){
                ROS_INFO("Talbai deer garch ireed 90 gradus ergeh");
                dcPos.data=0;
                dc_pos.publish(dcPos);
                // int op = 0;
                // if(ongo == "red"){
                //     op = -1;
                // }else if(ongo == "blue"){
                //     op = 1;
                // }
                if(robot_theta <= 85 || robot_theta >=92){
                    vel_pub.linear.x = 0;
                    vel_pub.linear.y = 0;
                    vel_pub.angular.z = -PID(1.3, 0, .5, 90, robot_theta, 127, 40, &i_error_z, &last_error_z);
                    speed.publish(vel_pub);

                }else{
                //90 gradus erge 

                //ergesen bol
                // float ttetha=theta;
                // if(ttetha>180){
                //     ttetha=ttetha-360;
                // }
                    reset_cordinate.data = theta - 90;
                    res_encoder.publish(reset_cordinate);
                
                    last_x = 0;
                    last_y = 0;
                    last_theta = theta - 90;
                    
                    if(!crushing_flag){
                        game_status = 12;
                    }else{
                        BLDC.data = 1;
                        brushless.publish(BLDC);
                        game_status = 6;
                    }
                        
                }
            }
            else if(game_status == 12){
                BLDC.data = 4;
                brushless.publish(BLDC);
                if(robot_x > -200){
                    velocity_publishing(0, -100, 0);
                    speed.publish(vel_pub);
                }else{
                    game_status = 6;
                    crushing_flag = true;

                }
            }
            else if(game_status == 6){
                ROS_INFO("Cameraar bumbug haraad pid setpointruu hureh");
                ROS_INFO("last_x = [%d]", last_x);
                ROS_INFO("last_y = [%d]", last_y);
                ROS_INFO("robot_x = [%d]", robot_x);
                ROS_INFO("robot_y = [%d]", robot_y);
                // int op = 0;
                int distance = 0;
                if(ongo == "red"){
                    // op = 1;
                    distance = right_distance;
                }else if(ongo == "blue"){
                    // op = -1;
                    distance = left_distance;
                }
                BLDC.data = 1;
                brushless.publish(BLDC);
                // vel_pub.data[3] = 0;
                if(take_ball == true){//bomgog haragdah uyed 
                    // system("clear");
                    // ROS_INFO("W = %d", w);
                    // if(robot_x ) 
                    if(!((w <= camera_point_y + 8) && abs(u-camera_point_x )<6)){  
                        
                        if(robot_y > 150 ||  robot_y < -150){
                            vel_pub.linear.x = -PID(0.5, 0, 0., camera_point_x, u, 50, 10, &i_error_x, &last_error_x);   
                            vel_pub.linear.y = PID(0.5, 0, 0., camera_point_y, w, 50, 10, &i_error_y, &last_error_y);
                        }else{
                        //     if(!crushing_flag){
                        //     // game_status = 11;
                        //     vel_pub.linear.y = PID(0.5, 0, 0., camera_point_y, w, 125, 5, &i_error_y, &last_error_y); 
                        // }else{
                        //     // BLDC.data = 1;
                        //     // brushless.publish(BLDC);
                            vel_pub.linear.y = PID(0.4, 0, 0., camera_point_y, w, 80, 5, &i_error_y, &last_error_y);
                            // }
                            vel_pub.linear.x = -PID(0.355, 0, 0., camera_point_x, u, 70, 5, &i_error_x, &last_error_x);       
                            vel_pub.angular.z = -PID(3, 0, 0., 90, robot_theta, 127, 20, &i_error_z, &last_error_z);
                            speed.publish(vel_pub);
                        }

                        
                        
                        ROS_INFO("nearest_ball : [%d]", nearest_ball);
                        ROS_INFO("X: - [%d]", u);
                        ROS_INFO("Y: - [%d]", w);
                        ROS_INFO("Min val: [%d]", min_val);

                    }else{
                        game_status=18;

                    }if(robot_ball){
                        velocity_publishing(0,0,0);
                        speed.publish(vel_pub);
                        now_x = robot_x + 50;
                        game_status = 7;
                        crushing_flag = true;
                    }

                }else{

                    ROS_ERROR("Bonbog oldoogui!!");  
                    if(right_distance > 150){
                        
                    }
                    velocity_publishing(0, 40, 0);
                    speed.publish(vel_pub);
                }
            }
            
            else if(game_status == 7){
                int op1 = 0;
                int op = 0;
                if(ongo == "red"){
                    op1 = 1;
                }else if(ongo == "blue"){
                    op1 = -1;
                }
                ROS_INFO("last_x = [%d]", last_x);
                ROS_INFO("last_y = [%d]", last_y);
                if(true_ball != 0){// 0 = purple, 1 = red or blue/
                    //PID last pos
                    int diff_x = last_x - robot_x;
                    int diff_y = last_y - robot_y;
                    float diff_theta = atan2(diff_y, diff_x) - (robot_theta * 3.14/180);
                    int length_xy = sqrt(pow(diff_x, 2) + pow(diff_y, 2));
                    diff_x=cos(diff_theta)*length_xy;
                    diff_y=sin(diff_theta)*length_xy;
                    ROS_INFO("orson");
                    ROS_INFO("diff_x = [%d]", diff_x);
                    ROS_INFO("diff_y = [%d]", diff_y);
                    if(abs(last_x-robot_x)>=220){
                        op=90;
                        // op=1;
                    }
                    else{
                        op=-120;
                        // op=-1;
                    }
                    
                    if((robot_theta < -20) && (silo_flag==0)){
                        if(robot_theta < -180){
                            velocity_publishing(0,0,-20);
                            speed.publish(vel_pub);
                        }else{
                            vel_pub.linear.x = -PID(0.9, 0, 0, 0.3, diff_x, 120, 10, &i_error_x, &last_error_x);
                            vel_pub.linear.y = PID(0.9, 0, 0, 0.3, diff_y, 120, 10, &i_error_y, &last_error_y);
                            vel_pub.angular.z = PID(0.9, 0, 0.6, 135, silo_data_dist, 120, 20, &i_error_z, &last_error_z);
                            speed.publish(vel_pub);      
                        }

                        if(abs(silo_data_dist-135)<5){
                            velocity_publishing(0,0,0);
                            speed.publish(vel_pub);
                            // usleep(300000);
                            // ros::Duration(0.3).sleep();
                            float ttetha=theta;
                            if(ttetha>180){
                                ttetha=ttetha-360;
                            }
                            reset_cordinate.data = ttetha - 90;
                            res_encoder.publish(reset_cordinate);
                            // usleep(100000);
                            for(int i=0; i<3; i++){
                                silo_data_stop.data=2;
                                silo_stop.publish(silo_data_stop);
                                ros::Duration(0.05).sleep();
                            }
                            // usleep(100000);
                            // ros::Duration(0.3).sleep();
                            silo_data_stop.data=0;
                            silo_stop.publish(silo_data_stop);
                            silo_flag=1;
                        }
                    }

                    else if(robot_theta < -60 && silo_flag==1){
                                             
                        // float ttetha=theta;
                        // if(ttetha>180){
                        //     ttetha=ttetha-360;
                        // }
                        // reset_cordinate.data = ttetha - 90;
                        // res_encoder.publish(reset_cordinate);
                        if(robot_theta < -180){
                            velocity_publishing(0,0,-20);
                            speed.publish(vel_pub);
                        }else{
                            vel_pub.linear.x = -PID(0.9, 0, 0, 0.3, diff_x, 120, 10, &i_error_x, &last_error_x);
                            vel_pub.linear.y = PID(0.9, 0, 0, 0.3, diff_y, 120, 10, &i_error_y, &last_error_y);
                            vel_pub.angular.z = PID(0.9, 0, 0.6, 285, silo_data_dist, 120, 20, &i_error_z, &last_error_z);
                            speed.publish(vel_pub);   
                        }
                        if(abs(silo_data_dist-285)<5){
                            velocity_publishing(0,0,0);
                            speed.publish(vel_pub);
                            // usleep(500000);
                            // ros::Duration(0.5).sleep();

                            float ttetha=theta;
                            if(ttetha>180){
                                ttetha=ttetha-360;
                            }
                            reset_cordinate.data = ttetha - 90;
                            res_encoder.publish(reset_cordinate);

                            for(int i=0; i<3; i++){
                                silo_data_stop.data=4;
                                silo_stop.publish(silo_data_stop);
                                ros::Duration(0.05).sleep();
                            }
                            silo_data_stop.data=0;
                            silo_stop.publish(silo_data_stop);
                            silo_flag=0;
                            // usleep(100000);
                            
                            game_status=16;
                        }
                    }
                    else{
                        vel_pub.linear.x = -PID(1.0, 0, 0.3, 5, diff_x, 110, 15, &i_error_x, &last_error_x);
                        vel_pub.linear.y = PID(1.0, 0, 0.3, 5, diff_y, 110, 15, &i_error_y, &last_error_y);
                        vel_pub.angular.z = -PID(6.5, 0, 3., op, robot_theta, 127, 20, &i_error_z, &last_error_z);
                        speed.publish(vel_pub);                        
                    }
                    
                    ROS_WARN("SILO_DATA_FLAG: [%d]", silo_flag);
                    ROS_WARN("Robot_theta is: {%d}", robot_theta);
                    ROS_INFO("last_x =========== [%d]", last_x);
                    ROS_INFO("last_y =========== [%d]", last_y);
                    
                }else{
                    
                    if(abs(now_x-robot_x) <= 5){
                        prev_theta = robot_theta + op1*45;
                        game_status = 21;
                        ROS_INFO("game status 21, now x [%d]" , now_x);
                    }else{
                        vel_pub.linear.x=0;
                        vel_pub.linear.y= 100; 
                        vel_pub.angular.z=-PID(3.5, 0, 0.7, 90, robot_theta, 127, 20, &i_error_z, &last_error_z);
                        speed.publish(vel_pub);

                    }
                    
                }
            }
            
            else if(game_status == 8){
                ROS_INFO("ROBOT theta = [%d]", robot_theta);
                      
                ROS_INFO("last_x =========== [%d]", last_x);
                ROS_INFO("last_y =========== [%d]", last_y);
                if(ongo == "blue"){   
                    if(255 >= robot_theta || 270 <= robot_theta){
                        vel_pub.linear.x = 0;
                        vel_pub.linear.y = 0;
                        vel_pub.angular.z = -PID(1.5, 0, 0.7, 270, robot_theta, 100, 10, &i_error_z, &last_error_z);
                        speed.publish(vel_pub);
                        
                    }else{
                        game_status = 16;//ergej harad ontsog zasah    
                    } 
                }  
                if(ongo == "red"){
                    if(260 >= robot_theta || 275 <= robot_theta){
                        vel_pub.linear.x = 0;
                        vel_pub.linear.y = 0;
                        vel_pub.angular.z = -PID(1.5, 0, 0.7, 270, robot_theta, 100, 10, &i_error_z, &last_error_z);
                        speed.publish(vel_pub);
                        
                    }else{
                            game_status = 16;//ergej harad ontsog zasah    
                    } 
                }             
            }
            
            // // Racknaas hoish uhrah
            else if(game_status == 9){
                int op=0;
                int y_distance=0;
                if(ongo == "red"){
                    op = 1;
                    // distance = right_distance + 22;
                    y_distance=front_distance;
                }
                if(ongo == "blue"){
                    op = -1;
                    // distance = right_distance - 22;
                    y_distance=back_distance;
                }                    
                ROS_INFO("ROBOT racknas uharch bna");
                // if(!robot_ball){
                    if(theta>175 && theta<185){
                        game_status = 10;
                        dcPos.data=0;
                        dc_pos.publish(dcPos);
                    //     usleep(50000);
                    // }
                        
                    }else{
                        int diff_x = 170 - right_distance;
                        int diff_y = 200 - y_distance;
                        float diff_theta = atan2(diff_y, diff_x) - (theta * 3.14/180)-90;
                        int length_xy = sqrt(pow(diff_x, 2) + pow(diff_y, 2));
                        diff_x=cos(diff_theta)*length_xy;
                        diff_y=sin(diff_theta)*length_xy;
                        ROS_INFO("orson");
                        ROS_INFO("diff_x = [%d]", diff_x);
                        ROS_INFO("diff_y = [%d]", diff_y);
                        if(right_distance<=90){
                            vel_pub.linear.x = -PID(1, 0, 0, 0.5, diff_x, 127, 10, &i_error_x, &last_error_x);
                            vel_pub.linear.y = PID(1, 0, 0, 0.5, diff_y, 127, 10, &i_error_y, &last_error_y);
                            float ttetha=theta;
                            if(ttetha>180){
                                ttetha=ttetha-360;
                            }
                            reset_cordinate.data = ttetha - 90;
                            vel_pub.angular.z = -PID(3.5, 0, 0.9, 90, ttetha, 127, 30, &i_error_z, &last_error_z);
                        }
                        else{
                            //  dcPos.data=0;
                            // dc_pos.publish(dcPos);
                            reset_cordinate.data = theta - 90;
                            res_encoder.publish(reset_cordinate);
                            vel_pub.linear.x = -PID(0.9, 0, 0, 0.3, diff_x, 80, 20, &i_error_x, &last_error_x);
                            vel_pub.linear.y = PID(0.9, 0, 0, 0.3, diff_y, 80, 20, &i_error_y, &last_error_y);
                            vel_pub.angular.z = -PID(5.5, 0, 1.5, 180, theta, 127, 10, &i_error_z, &last_error_z);                        
                        }   

                    }
                    // vel_pub.angular.z = -PID(2.0, 0, 0.7, 90, robot_theta, 127, 40, &i_error_z, &last_error_z); 
                    // if((180-theta)<theta){
                    //     vel_pub.angular.z = -PID(2.0, 0, 0.7, 180, robot_theta, 127, 40, &i_error_z, &last_error_z);
                    // }
                    // else{
                    //     vel_pub.angular.z = -PID(2.0, 0, 0.7, 0, robot_theta, 127, 40, &i_error_z, &last_error_z);
                    // }
                    speed.publish(vel_pub);
                // }
            }
            else if (game_status==10)
            {
                int op;
                int y_distance;
                if(ongo == "red"){
                    op = -1;
                    // distance = right_distance + 22;
                    y_distance=front_distance-20;
                }
                if(ongo == "blue"){
                    op = 1;
                    // distance = right_distance - 22;
                    y_distance=back_distance+20;
                }      
                dcPos.data=0;
                dc_pos.publish(dcPos);  
                if((abs(right_distance-170)<5)){
                    game_status=17;
                   
                    // for(int i = 0; i<4; i++){
                    // float ttetha=theta;
                    // if(ttetha>180){
                    //     ttetha=ttetha-360;
                    // }
                    reset_cordinate.data = theta - 90;
                    res_encoder.publish(reset_cordinate);
                }
                else{
                    vel_pub.linear.x = op*PID(1.2, 0, 0.6, 200, y_distance, 127, 20, &i_error_x, &last_error_x);
                    vel_pub.linear.y = -PID(1.2, 0, 0.5, 170, right_distance, 127, 20, &i_error_y, &last_error_y);
                    vel_pub.angular.z = -PID(3.5, 0, 1.2, 180, theta, 100, 20, &i_error_z, &last_error_z);     
                    speed.publish(vel_pub);
                }
            }

            // else if(game_status == 11){
            //     if(w > -370){
            //         vel_pub.linear.y = PID(0.5, 0, 0., camera_point_y, w, 127, 5, &i_error_y, &last_error_y);
            //         vel_pub.linear.x = -PID(0.2, 0, 0., camera_point_x, u, 80, 5, &i_error_x, &last_error_x);       
            //         vel_pub.angular.z = -PID(3, 0, 0., 90, robot_theta, 127, 20, &i_error_z, &last_error_z);
            //         speed.publish(vel_pub);
            //     }else{
            //         game_status = 12;
            //     }
            // }

            // else if(game_status == 12){
            //     velocity_publishing(0,-60,0);
            //     speed.publish(vel_pub);
            //     BLDC.data = 1;
            //     brushless.publish(BLDC);
            //     game_status = 6;
            //     crushing_flag = true;
            //     ros::Duration(0.7).sleep();
            // }
            
            else if(game_status == 15){
                int op =0;
                if(ongo == "red"){
                    op = 1;
                }
                if(ongo == "blue"){
                    op = -1;
                } 
                if(op*prev_theta < op*robot_theta){
                        vel_pub.linear.x = 0;
                        vel_pub.linear.y = 0;
                        vel_pub.angular.z = -PID(2, 0, 0.7, prev_theta, robot_theta, 120, 20, &i_error_z, &last_error_z);
                        speed.publish(vel_pub);
                }else{
                    
                    game_status = 6;
                }
            }
            
            else if(game_status == 16){
                ROS_INFO("ROBOT siloruu yvah hesegt bna: ");
                int silo;
                int op=0;
                  
                if(ongo == "red"){
                    op = 1;
                    distance = front_distance;
                    silo = silo_dist-44;
                }
                if(ongo == "blue"){
                    op = -1;
                    distance = back_distance;
                    silo = silo_dist;
                }                    
    
                if(distance == silo && right_distance <= 34 )  {
                    if(!ir_ball){
                        BLDC.data = 2;  
                        brushless.publish(BLDC);
                        // vel_pub.data[3] = 2;
                        velocity_publishing(0,0,0);                // if(robot_theta>-88||robot_theta<-92){
            //     vel_pub.angular.z = -PID(2.0, 0, 0.7, -90, robot_theta, 127, 20, &i_error_z, &last_error_z);// hasah bolgoson 
            // }
            // else{
                        speed.publish(vel_pub);
                        // reset_cordinate.data = 270;
                        // res_encoder.publish(reset_cordinate);
                    }else{ 
                        


                        game_status = 9;
                    }
                        
                }else{
                    ROS_INFO("silo dist     = [%d]", silo);
                    ROS_INFO("left distance = [%d]", distance);
                    if(ongo == "blue"){
                        vel_pub.linear.x =  op*PID(1.0, 0, 0.6, silo, distance, 90, 10, &i_error_x, &last_error_x);
                    }
                    if(ongo == "red"){
                        vel_pub.linear.x =  op*PID(1.0, 0, 0.4, silo, distance, 90, 10, &i_error_x, &last_error_x);
                    }
                    vel_pub.linear.y = PID(1, 0, 0.5, 32, right_distance, 75, 10, &i_error_y, &last_error_y);
                    float ttetha=theta;
                    if(ttetha>180){
                        ttetha=ttetha-360;
                    }
                    // reset_cordinate.data = ttetha;
                    // res_encoder.publish(reset_cordinate);
                    vel_pub.angular.z = -PID(2.8, 0, 0.9, 0, ttetha, 127, 10, &i_error_z, &last_error_z);
                    speed.publish(vel_pub);
                }   
                // }
                // ROS_INFO("Ttheta : [%f]", ttheta);
                ROS_INFO("theta : [%f]", theta);
                // ROS_INFO("")
            }
            
            else if(game_status == 17){

                ROS_INFO("Robot racknas uharch bna 90 gradusruu ergej hardag");
                if(robot_theta <= 85 || robot_theta >=92){

                    float ttetha=theta -90;
                    vel_pub.linear.x = 0;
                    vel_pub.linear.y = 0;
                    vel_pub.angular.z = -PID(2.5, 0, 0.7, 90, ttetha, 127, 10, &i_error_z, &last_error_z);
                    speed.publish(vel_pub);

                }else{

                    // reset_cordinate.data = 90;
                    // res_encoder.publish(reset_cordinate);
                    // switch (silo.back())
                    // {
                    //     case 1:
                    //         silo_dist=73;
                    //         break;
                    //     case 2:
                    //         silo_dist=145;
                    //         break;
                    //     case 3:
                    //         silo_dist=221;
                    //         break;
                    //     case 4:
                    //         silo_dist=295;
                    //         break;
                    //     case 5:
                    //         silo_dist=371;
                    //         break;

                    // }   
                    // silo.pop_back();
                    
                     // float ttetha=theta;
                    // if(ttetha>180){
                    //     ttetha=ttetha-360;
                    // }
                    reset_cordinate.data = theta - 90;
                    res_encoder.publish(reset_cordinate);
                    last_theta = robot_theta - 90;
                    last_x = 0;
                    last_y = 0;
                    game_status = 6;

                }
            }
            else if(game_status == 18){
                ROS_INFO("ROBOT bombog gargah uildel");
                if(!robot_ball){
                    velocity_publishing(0,-50,0);
                    speed.publish(vel_pub);
                }else{
                    velocity_publishing(0,0,0);
                    speed.publish(vel_pub);
                    game_status = 7;
                    crushing_flag =true;
                    prev_theta = robot_theta + 45;
                }
            }

            else if(game_status == 19){
                int op=-1;
                if(ongo=="red"){
                    op=-1;
                }
                else if(ongo=="blue"){
                    op=1;

                }
                if((robot_y * op) >= (last_y * op)){
                    vel_pub.linear.x = 0;
                    vel_pub.linear.y = op*127;
                    vel_pub.angular.z = 0;
                    speed.publish(vel_pub);
                }else{
                    game_status = 4;
                }
            }

            else if(game_status == 20){
                ROS_INFO("Robot naluugar ogsoh");
                int op=1;
                if(ongo=="red"){
                    op=1;
                }
                else if(ongo=="blue"){
                    op=-1;
                }
                if((-robot_y * op) >= (-last_y * op)){
                    vel_pub.linear.x = 0;
                    vel_pub.linear.y = op*-80;
                    vel_pub.angular.z = -PID(2.5, 0, 0.7, last_theta, robot_theta, 127, 5, &i_error_z, &last_error_z);;
                    speed.publish(vel_pub);
                }else{
                    // float ttetha=theta;
                    // if(ttetha>180){
                    //     ttetha=ttetha-360;
                    // }
                    reset_cordinate.data = theta - 90;
                    res_encoder.publish(reset_cordinate);
                    game_status = 5;
                }
            }

            else if(game_status == 21){
                int op = 0;
                if(ongo == "red"){
                    op = 1;
                }else if(ongo == "blue"){
                    op = -1;
                }
                if(op*prev_theta >= op*robot_theta){
                    ROS_INFO("prev_theta [%d]", prev_theta);
                    ROS_INFO("robot_theta ");
                    vel_pub.linear.x = 0;
                    vel_pub.linear.y = 0;
                    vel_pub.angular.z = -PID(2, 0, 1.0, prev_theta, robot_theta, 120, 40, &i_error_z, &last_error_z);
                    speed.publish(vel_pub);
                }else{
                    ROS_INFO("bombog gargah uildel !!!!!");
                    
                    BLDC.data = 3;
                    brushless.publish(BLDC);

                    // vel_pub.data[3] = 3;
                    ros::Duration(0.9).sleep();
                    // usleep(800000);
                    prev_theta-= op*45;
                    game_status = 15;
                }
            }
            else if(game_status == 22){
                int left_setpoint=50;
                prev_status = 22;
                int op = 0;
                  dcPos.data=150;
                dc_pos.publish(dcPos);
                BLDC.data = 0;
                brushless.publish(BLDC);
                if(ongo == "red"){
                    op = 1;
                    distance=front_distance;
                }else if(ongo == "blue"){
                    op = -1;
                    distance=back_distance;
                }  
                if(distance > 100){
                    vel_pub.linear.x = PID(1.0, 0, 0.6000, left_setpoint,left_distance, 110, 5, &i_error_x, &last_error_x);//int utga butsaana
                    vel_pub.linear.y =  -op*70;
                    vel_pub.angular.z= -PID(2.3,  0.0, 1.3000, 90, theta, 127, 5, &i_error_z, &last_error_z);//int utga butsaana
                    speed.publish(vel_pub);
                }else{
                    game_status=4;
                }
            }
        }
         ros::spinOnce();
        loop_rate.sleep(); 
    }
    return 0;

}