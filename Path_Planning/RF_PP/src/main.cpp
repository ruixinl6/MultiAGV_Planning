#include <iostream>
#include "AStar.h"
#include <time.h>
#include "ts.h"
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{


    LMap::Instance().ReadMap();
    TS myTS;
    myTS.ReadMap();

//    LMap::Instance().FunctionTest();

//    while (true) {

//    }

    while (true) {
        myTS.GetRobotPositions();
        myTS.Init();
        myTS.checkDeadLock();
        if (myTS.isDeadLock) {
            std::vector<int> robot_numbers;
            std::vector<int> from_ids;
            std::vector<int> to_ids;
            std::vector<double> start_work_time;
            double curr_time = time(NULL);
            for (int i = 0; i < myTS.RobotTasks.size(); i++) {
                start_work_time.push_back(curr_time);
                from_ids.push_back(myTS.robot_positions[i]);
                to_ids.push_back(myTS.RobotTasks[i].to);
                robot_numbers.push_back(i + 1);
            }
            std::vector<route> result = LMap::Instance().serve(robot_numbers, from_ids, to_ids, start_work_time);
            for (int i = 0; i < myTS.RobotTasks.size(); i++) {
                LMap::Instance().StoreRobotPath(i + 1, result[i]);
                LMap::Instance().PrintRobotPath(i + 1, result[i]);
            }
            myTS.isDeadLock = 0;
            myTS.setDeadLock(0);
        }
        for (int i = 0; i < myTS.RobotTasks.size(); i++) {
            if (myTS.RobotTasks[i].isFinished == 1) {
                rf_task t;
                t.from = myTS.robot_positions[i];
                if (false and myTS.robot_with_box[i] == 1) {
//                    if (i == 0) {
//                        t.to = 0;
//                        t.isFinished = 0;
//                    } else if (i == 1) {
//                        t.to = 231;
//                        t.isFinished = 0;
//                    } else if (i == 2) {
//                        t.to = 18;
//                        t.isFinished = 0;
//                    }
                } else {
                    while (true) {
                       std::srand( (unsigned)time( NULL ) );
                       int to = myTS.yardPoints[std::rand()%(myTS.yardPoints.size())];
                       bool flag = true;
                       for (int j = 0; j < myTS.RobotTasks.size(); j++) {
                           if (j == i) {
                               continue;
                           } else {
                               if (abs(myTS.physical_node_list[to].x - myTS.physical_node_list[myTS.RobotTasks[j].to].x) <= 3
                                       && abs(myTS.physical_node_list[to].y - myTS.physical_node_list[myTS.RobotTasks[j].to].y) <= 3) {
                                   flag = false;
                               }
                               if (abs(myTS.physical_node_list[to].x - myTS.physical_node_list[0].x) <= 3
                                       && abs(myTS.physical_node_list[to].y - myTS.physical_node_list[0].y) <= 3) {
                                   flag = false;
                               }
                           }
                       }
                       if (flag) {
                           t.to = to;
                           t.isFinished = 0;
                           break;
                       }
                    }
                }

                double curr_time = time(NULL);
                std::vector<int> from_ids;
                std::vector<int> to_ids;
                std::vector<int> robot_numbers;
                std::vector<double> start_work_time;
                from_ids.push_back(t.from);
                to_ids.push_back(t.to);
                robot_numbers.push_back(i + 1);
                start_work_time.push_back(curr_time);
                std::vector<route> result = LMap::Instance().serve(robot_numbers, from_ids, to_ids, start_work_time);
                if (result.size() != 0) {
                    myTS.RobotTasks[i] = t;
                    LMap::Instance().StoreRobotPath(i + 1, result[0]);
                    LMap::Instance().PrintRobotPath(i + 1, result[0]);
                    myTS.StoreRobotTask(i + 1, t);

                } else {

                }
            }
        }
    }
    return 0;
}
