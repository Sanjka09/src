#include "ros/ros.h"
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include "stdlib.h"
#include "std_msgs/String.h"
#include "std_msgs/Int16MultiArray.h"
#include "std_msgs/Int8MultiArray.h"
#include "std_msgs/UInt8.h"
#include "std_msgs/Int8.h"

float right_inter_y;
float silo_dist;
std::string color;
std::string color_inverse;
bool once = false;
int8_t sending_value;
int cnt = 0;
std::string comma=",";
int mode=0;



std::string choose[3];
std::string numeric[5];
std::stringstream stream;

std_msgs::Int8 decision;
std_msgs::Int8MultiArray silo_buffers;

ros::Publisher silo_choice;
ros::Publisher choice; // Declare the publisher globally


struct silo_sorting
{
    int x_cor;
    std::string name;
};

struct counter
{
    int tooluur;
};

struct counter my_side_same;
struct counter other_side_same;
struct counter my_side_mixed;
struct counter other_side_mixed;
struct counter silo_flag;
struct silo_sorting ball[10];

bool isNumber(const std::string &s);
void silo_checking_function(std::stringstream &iss);
bool isString(const std::string &s);
void checkStringArrayElements(int r_color, int b_color);


bool isNumber(const std::string &s) {
    // return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
    for (char const &ch : s){
        if(!isdigit(ch)) return false;
    }
    return true;
}


bool isString(const std::string &s) {
    // Check if the string ends with a comma
    if(s == "blue" || s == "red" || s == "purple"){
        return false;
    }else if(s == "blue," || s == "red," || s == "purple,"){
        return false;
    }
    if (s.back() == ',') {
        for (size_t i = 0; i < s.size() - 1; ++i) {
            if (!isalpha(s[i])) return false;
        }
        return true;
    }
    // Check if the string is alphabetic
    for (char const &ch : s) {
        if (!isalpha(ch)) return false;
    }
    return true;
}

// void silo_fl(const std_msgs::Int8::ConstPtr &flags)
// {
//     silo_flag.tooluur = flags->data;
// }

void encoder_clback(const std_msgs::Int16MultiArray::ConstPtr &enco)
{   
    if((enco->data[0] != 0) && (enco->data[1] != 0)){
        if (!once){
            if (enco->data[0] >= enco->data[1]){
                color = "R";
                color_inverse = "B";
            }
            else{
                color = "B";
                color_inverse = "R";
            }
            once = true;
        }
    }
}

void silo_checking_function(std::stringstream &iss){

        std::vector<std::string> tokens;
        std::string token;
        std::vector<std::vector<std::string>> filteredPatterns;

        // std::cout<< "1"<< std::endl;
        // Read tokens from the message
        while (iss >> token)
        {
            tokens.push_back(token);
        }
        // if(tokens.size()%4 == 3){
        //     std::cerr << "Error: Token ehleed too dangaara orj irsen " << std::endl;
            
        //     return; // Exit the function if not
        // }
        if(tokens.size() == 0 || tokens.size() == 1){
            std::cerr << "Error: Token size:"<< tokens.size()<< std::endl;
            return; 
        }
       
        // std::cout<< "1"<< std::endl;
        // for(auto& is: tokens){
        //     std:: cout << is << std::endl;
        // }
        // std::cout<< "1"<< std::endl;
        std::vector<std::vector<std::string>> patterns;
        size_t i = 0;
        while(i < tokens.size() - 3){
            // std::cout<< "4"<< std::endl;
            if (isNumber(tokens[i]) && isNumber(tokens[i + 1]) && isNumber(tokens[i + 2]) && isString(tokens[i+3]))
            {
                // std::cout<< "6"<< std::endl;
                std::vector<std::string> pattern = {tokens[i], tokens[i + 1], tokens[i + 2], tokens[i + 3]};
                patterns.push_back(pattern);
                i += 4;
            }else{
                // std::cout<< "5"<< std::endl;
                i++;
            }
        } 
        // std::cout<< "2"<< std::endl;
        // }
        // std::cout<< "Token sike is: " << tokens.size()<<std::endl;
        // std::cout<< "Pattern size is:" << patterns.size()<< std::endl;

       
        // std::cout<< "3"<< std::endl;
        for (const auto &pattern : patterns)
        {
            bool shouldAdd = true;
            // std::cout<< "4"<< std::endl;
            for (auto &filteredPattern : filteredPatterns)
            {
                int currSecondInt = std::stoi(pattern[1]);
                int filteredSecondInt = std::stoi(filteredPattern[1]);

                if (std::abs(currSecondInt - filteredSecondInt) <= 20)
                {
                    int currFirstInt = std::stoi(pattern[0]);
                    int filteredFirstInt = std::stoi(filteredPattern[0]);

                    if (currFirstInt > filteredFirstInt)
                    {
                        filteredPattern = pattern;
                    }
                    shouldAdd = false;
                    break;
                }
            }
            if (shouldAdd)
            {
                filteredPatterns.push_back(pattern);
            }
        }

        // Sort the patterns based on x coordinates (second element)
        std::sort(filteredPatterns.begin(), filteredPatterns.end(), [](const std::vector<std::string> &a, const std::vector<std::string> &b) {
            return std::stoi(a[1]) < std::stoi(b[1]);
        });

        cnt = filteredPatterns.size();

        // Handle different cases based on the number of patterns
        if (cnt == 3)
        {
            std::cout << "COUNTER IS 3 " << std::endl;
            // Assign sorted values directly
            choose[0] = filteredPatterns[0][3];
            choose[1] = filteredPatterns[1][3];
            choose[2] = filteredPatterns[2][3];
        }
        else if (cnt == 2)
        {
            std::cout << "COUNTER IS 2 " << std::endl;
            // Assign sorted values directly
            choose[0] = filteredPatterns[0][3];
            choose[1] = filteredPatterns[1][3];
        }else if(cnt == 4){
            choose[0] = filteredPatterns[0][3];
            choose[1] = filteredPatterns[1][3];
            choose[2] = filteredPatterns[2][3];
            choose[3] = filteredPatterns[3][3];
        }


        // Output the chosen class names
        // for (int i = 0; i < cnt; i++)
        // {
        //     ROS_INFO("%d = %s ", i, choose[i].c_str());
        // }
}

void cam_call_back(const std_msgs::String::ConstPtr &msg)
{
    // if(silo_flag.tooluur == 2 || silo_flag.tooluur == 4 || silo_flag.tooluur == 3){
        std::stringstream stream(msg->data);
        silo_checking_function(stream);
        // std::vector<std::vector<std::string>> correctPattern;
        std::cout<< "[";
        for(int p=0;p<5; p++){
            ROS_WARN("%s", numeric[p].c_str());
        }
        ROS_INFO("MOde: %d", mode);
        std::cout<< "]"<<std::endl;
    // }
}

void checkStringArrayElements(int r_color, int b_color)
{
    std::vector<int8_t> silo_buffer;
    bool foundTarget = false;
    sending_value = 0;
    int mode=0;
    //DEFINING DEFENCE OF OFFENCE MODE
    if(color == "R"){
        if (r_color <= b_color) {
            mode = 1; // defense
        } 
        else {
            mode = 2; // offense
        }
        std::cout << "Mode: " << (mode == 1 ? "Defense" : "Offense") << std::endl;
    }
    else{
        if (r_color <= b_color) {
            mode = 2; // offense
        } 
        else {
            mode = 1; // defense
        }
        std::cout << "Mode: " << (mode == 1 ? "Defense" : "Offense") << std::endl;
    }
 
    //PRIORITY LIST OF DEFENCE MODE
    if (mode == 1) {                                                // defense
        for (int m = 0; m < 5; m++) {
            if((numeric[m] == color+color_inverse) || (numeric[m] == color_inverse+color) || (numeric[m] == color+color_inverse+comma) || (numeric[m] == color_inverse+color+comma)){
                silo_buffer.push_back(1);
            }
            else if((numeric[m] == color_inverse+color_inverse) || (numeric[m] == color_inverse+color_inverse+comma)){
                silo_buffer.push_back(2);
            }
            else if((numeric[m] == color+color) ||(numeric[m] == color+color+comma)){
                silo_buffer.push_back(3);
            }
            else if ((numeric[m] == "SILO") || (numeric[m] == "SILO,")){
                silo_buffer.push_back(4);
            }
            else if ((numeric[m] == color) || (numeric[m] == color+comma)) {    
                silo_buffer.push_back(5);
            }
            else if ((numeric[m] == color_inverse) || (numeric[m] == color_inverse +comma)) { 
                silo_buffer.push_back(6);
            }
            else{
                silo_buffer.push_back(7);
            }    
        }
    }

    //PRIORITY LIST OF OFFENCE MODE
    else if (mode == 2) {                                                // offense
        for (int m = 0; m < 5; m++) {
            if((numeric[m] == (color+color_inverse)) || (numeric[m] == (color_inverse+color)) || (numeric[m] == (color+color_inverse+comma)) || (numeric[m] == color_inverse+color+comma)){
                silo_buffer.push_back(1);
            }
            else if((numeric[m] == color+color) ||(numeric[m] == color+color+comma)){
                silo_buffer.push_back(2);
            }
            else if((numeric[m] == color_inverse+color_inverse) || (numeric[m] == color_inverse+color_inverse+comma)){
                silo_buffer.push_back(3);
            }
            else if ((numeric[m] == color) || (numeric[m] == color+comma)) { 
                silo_buffer.push_back(4);
            }
            else if ((numeric[m] == "SILO") || (numeric[m] == "SILO,")){
                silo_buffer.push_back(5);
            }
            else if ((numeric[m] == color_inverse) || (numeric[m] == color_inverse +comma)) { 
                silo_buffer.push_back(6);
            }
            else{
                silo_buffer.push_back(7);
            }
        }
    }


    //SORTING WHICH SILO HAS HIGHEST PRIORITY OF BALLS
    auto min_it = std::min_element(silo_buffer.begin(), silo_buffer.end());
    int min_index = std::distance(silo_buffer.begin(), min_it);
    int minimum = *min_it;
    std::cout << "Minimum: " << static_cast<int>(minimum) << " at index " << min_index << std::endl;
    
    if(min_index != 2){
            if(silo_buffer[2] == static_cast<int>(minimum)){
                sending_value = 2;
                foundTarget = true;
            }
            else if(silo_buffer[1] == static_cast<int>(minimum)){
                min_index = 1;
                sending_value = 1;
                foundTarget = true;
            }
            else if(silo_buffer[3] == static_cast<int>(minimum)){
                min_index = 3;
                sending_value = 3;
                foundTarget = true;
            }else{
                sending_value = min_index;
            }
        // }
    }else{
        sending_value = 2;
        foundTarget = true;
    }

    //PUBLISH WHICH SILO CHOSEN
        std::cout<< sending_value <<std::endl;
        decision.data = sending_value;
        choice.publish(decision);
        sending_value = 0;
        // numeric->empty();
        silo_buffer.empty();
}

void silo_fl(const std_msgs::Int8::ConstPtr &flags)
{
    silo_flag.tooluur = flags->data;
    // right_inter_y = silo_data->data[0];
    // silo_dist = silo_data->data[1];
    int r=0, b=0;
    if (silo_flag.tooluur == 2)//(silo_dist < 133) && (silo_dist > 118) && (silo_flag.tooluur == 2))
    {
        std::cout << "2-r silo \n";
        if(cnt == 2){
            numeric[1] = choose[1];
            numeric[2] = choose[2];
        }else if(cnt == 4){
            numeric[0] = choose[0];
            numeric[1] = choose[1];
            numeric[2] = choose[2];
            numeric[3] = choose[3];
        }else{
            numeric[0] = choose[0];
            numeric[1] = choose[1];
            numeric[2] = choose[2];
            ROS_INFO("%s", numeric[0].c_str());
            ROS_INFO("%s", numeric[1].c_str());
            ROS_INFO("%s", numeric[2].c_str());
        }
    }
    else if (silo_flag.tooluur == 4 || silo_flag.tooluur == 3)//(silo_dist < 283) && (silo_dist > 268) && (silo_flag.tooluur == 4))
    {
        std::cout << "4-r silo \n";
        // for(auto& any: numeric){
        //     std::cout<< any<<std::endl;
        // }
        if(cnt == 3){
            numeric[2] = choose[0];
            numeric[3] = choose[1];
            numeric[4] = choose[2]; 
            ROS_INFO("%s", numeric[4].c_str());
        }else if(cnt == 2){
            ROS_INFO("%s", numeric[4].c_str());
            numeric[3] == choose[0];
            numeric[4] == choose[1];
        }
        ROS_INFO("%s", numeric[3].c_str());

        for(int i = 0; i<5; i++){
            if(numeric[i].empty()){
                numeric[i] = "SILO";
            }
        }
        
        int r = 0, b = 0;
        for (int m=0; m<5; m++)
        {
            if((numeric[m] == "R") || (numeric[m] == "R,")){
                r+=1;
            }
            else if((numeric[m] == "B") || (numeric[m] == "B,")){
                b+=1;
            }
            else if((numeric[m] == "RR") || (numeric[m] == "RR,")){
                r+=2;
            }
            else if((numeric[m] == "BB") || (numeric[m] == "BB,")){
                b+=2;
            }
            else if((numeric[m] == "RB") || (numeric[m] == "BR") || (numeric[m] == "RB,") || (numeric[m] == "BR,")){
                b+=1;
                r+=1;
            }
            else if((numeric[m] == "RRB") || (numeric[m] == "RBR") || (numeric[m] == "BRR") || (numeric[m] == "RRB,") || (numeric[m] == "RBR,") || (numeric[m] == "BRR,")){
                r+=2;
                b+=1;
            }
            else if((numeric[m] == "BBR") || (numeric[m] == "BRB") || (numeric[m] == "RBB") || (numeric[m] == "BBR,") || (numeric[m] == "BRB,") || (numeric[m] == "RBB,")){
                r+=1;
                b+=2;
            }
            else if((numeric[m] == "BBB") || (numeric[m] == "BBB,")){
                b+=3;
            }
            else if((numeric[m] == "RRR") || (numeric[m] == "RRR,")){
                r+=3;
            }
        }

        // Call the function to check the elements of the array
        checkStringArrayElements(r, b);
    }else{
        std::cout << "silo haraagui \n";
    }
    silo_flag.tooluur = 0;
}

int main(int argc, char *argv[])
{
    ros::init(argc, argv, "Silo_detection_node");
    ros::NodeHandle nh;
    choice = nh.advertise<std_msgs::Int8>("/silo_detection", 1); // Initialize the publisher
// ("/silo_detection", 10, silo_songolt_hiih);
    // silo_choice = nh.advertise<std_msgs::Int8MultiArray>("/silo_detection/buffer", 5);
    ros::Subscriber camera = nh.subscribe("/silo_data", 1024, cam_call_back);
    // ros::Subscriber camera = nh.subscribe("/camera_data", 1024, cam_call_back);
    // ros::Subscriber silo_index = nh.subscribe("/silo_index", 128, silo_callback);
    ros::Subscriber encoder = nh.subscribe("/encoder", 10, encoder_clback);
    ros::Subscriber silo_harah = nh.subscribe("/silo_stop", 10, silo_fl);
        // ros::Subscriber silo_songolt = nh.subscribe("/silo_detection", 10, silo_songolt_hiih);

    ros::spin();
    return 0;
}