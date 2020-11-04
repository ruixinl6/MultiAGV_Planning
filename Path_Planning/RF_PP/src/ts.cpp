#include "ts.h"
#include <fstream>
#include <iostream>

std::vector<string> TS::split(const string& str, const string& delim) {
    std::vector<string> res;
    if("" == str) return res;
    char * strs = new char[str.length() + 1];
    strcpy(strs, str.c_str());

    char * d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());

    char *p = strtok(strs, d);
    while(p) {
        string s = p;
        res.push_back(s);
        p = strtok(NULL, d);
    }
    return res;
}


std::string TS::int2str(int num)
{
    std::string c_id;
    std::stringstream ss;
    ss<<num;
    ss>>c_id;
    return c_id;
}

int TS::str2int(std::string num)
{
    int c_id;
    std::stringstream ss;
    ss<<num;
    ss>>c_id;
    return c_id;
}


TS::TS()
{
    ctx = redisConnect("127.0.0.1", 6379);
    if(ctx->err)
    {
        std::cout<<"No connection with ctx_get"<<std::endl;
    } else {
        std::cout << "Successfully connected to Redis!" << std::endl;
    }

}


TS::~TS()
{

}

void TS::ReadMap() {
    this->physical_node_list = myMapreader.readPhysicalNodes();
    std::vector<int> tempYardPoints;
//    for (int i = 0; i < this->physical_node_list.size(); i++) {
//        if (this->physical_node_list[i].type == 3) {
//            tempYardPoints.push_back(this->physical_node_list[i].physical_node_id);
//        }
//    }

//    redisReply * reply = (redisReply *)redisCommand(ctx, "GET yardPoints");
//    string pointsStr = reply->str;
//    freeReplyObject(reply);

//    std::vector<string> pointsVecStr = this->split(pointsStr, ",");
//    for (int i = 0; i < pointsVecStr.size(); i++) {
//        int tempPoint = this->str2int(pointsVecStr[i]);
//        tempYardPoints.push_back(tempPoint);
//    }
    for (int i = 0; i < this->physical_node_list.size(); i++) {
        tempYardPoints.push_back(this->physical_node_list[i].physical_node_id);
    }

    this->yardPoints = tempYardPoints;
    for (int i = 0; i < 3; i++) {
        rf_task t;
        this->RobotTasks.push_back(t);
    }
    return;
}



void TS::Init() {
    for (int i = 1; i <= this->RobotTasks.size(); i++) {
        rf_task t;
        redisReply * reply = (redisReply *)redisCommand(ctx, "HGET robot_task %d", i);
        std::string ids = reply->str;
        const char * data = ids.c_str();
        cJSON * json = cJSON_Parse(data);
        cJSON * temp_ptr = json->child;
        for (int i = 0; i < 3; i++) {
            std::string temp_str = temp_ptr->string;
            if (temp_str == "isFinished") {
                t.isFinished = temp_ptr->valueint;
            } else if (temp_str == "from") {
                t.from = temp_ptr->valueint;
            } else if (temp_str== "to") {
                t.to = temp_ptr->valueint;
            }
            temp_ptr = temp_ptr->next;
        }
        //delete(data);
        cJSON_Delete(json);
        cJSON_Delete(temp_ptr);

        this->RobotTasks[i - 1] = t;
        freeReplyObject(reply);
    }
    return;
}

void TS::GetRobotPositions() {
    std::vector<int> positions;
    for (int i = 1; i <= this->RobotTasks.size(); i++) {
        redisReply * reply = (redisReply *) redisCommand(ctx, "HGET robot_position %d", i);
        int tempPos = std::stoi(reply->str);
        positions.push_back(tempPos);
        freeReplyObject(reply);
    }
    this->robot_positions = positions;

    std::vector<int> with_box;
    for (int i = 1; i <= this->RobotTasks.size(); i++) {
        redisReply * reply = (redisReply *) redisCommand(ctx, "HGET robot_with_box %d", i);
        int tempWith_box = std::stoi(reply->str);
        with_box.push_back(tempWith_box);
        freeReplyObject(reply);
    }
    this->robot_with_box = with_box;
}

void TS::StoreRobotTask(int robot_id, rf_task t) {
    std::string s = "{\"from\":";
    s += std::to_string(t.from);
    s += ", \"to\":";
    s += std::to_string(t.to);
    s += ", \"isFinished\":";
    s += std::to_string(t.isFinished);
    s += "}";
    redisReply * reply = (redisReply *)redisCommand(ctx, "HSET robot_task %d %s", robot_id, s.c_str());
    freeReplyObject(reply);
}

void TS::checkDeadLock() {
    redisReply * reply = (redisReply *)redisCommand(ctx, "GET dead_lock");
    int islocked = std::stoi(reply->str);
    this->isDeadLock = islocked;
    freeReplyObject(reply);
}

void TS::setDeadLock(int a) {
    redisReply * reply = (redisReply *)redisCommand(ctx, "SET dead_lock %d", a);
    freeReplyObject(reply);
}
