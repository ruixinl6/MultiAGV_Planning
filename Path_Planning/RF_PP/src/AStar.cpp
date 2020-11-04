#include "AStar.h"
#include <fstream>
#include <iostream>
#include <math.h>
#include "mapreader.h"

#define INF 10000000000
#define EPS 0.001

LMap::LMap()
{
    ctx = redisConnect("127.0.0.1", 6379);
    if(ctx->err)
    {
        std::cout<<"No connection with ctx_get"<<std::endl;
    } else {
        std::cout << "Successfully connected to Redis!" << std::endl;
    }
}


LMap::~LMap()
{

}


std::vector<string> LMap::split(const string& str, const string& delim) {
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


std::string LMap::int2str(int num)
{
    std::string c_id;
    std::stringstream ss;
    ss<<num;
    ss>>c_id;
    return c_id;
}

int LMap::str2int(std::string num)
{
    int c_id;
    std::stringstream ss;
    ss<<num;
    ss>>c_id;
    return c_id;
}

std::string LMap::double2str(double num)
{
    std::string c_id;
    std::stringstream ss;
    ss << std::setprecision(17) <<num;
    ss>>c_id;
    return c_id;
}


double LMap::str2double(std::string num)
{
    double c_id;
    std::stringstream ss;
    ss<< std::setprecision(17) << num;
    ss>> std::setprecision(17) >> c_id;
    return c_id;
}



void LMap::init() {
    std::vector<std::vector<std::pair<double, double> > > new_node_locked_time_window;
    for (int i = 0; i < this->neighbour_node_list.size(); i++) {
        std::vector<std::pair<double, double>> temp;
        new_node_locked_time_window.push_back(temp);
    }
    this->node_locked_time_window = new_node_locked_time_window;

}


double LMap::getDistance(int node1, int node2) {
    double node1X = this->node_list[node1].x;
    double node1Y = this->node_list[node1].y;
    double node2X = this->node_list[node2].x;
    double node2Y = this->node_list[node2].y;
    double result = sqrt((node1X - node2X) * (node1X - node2X) + (node1Y - node2Y) * (node1Y - node2Y));

    return result;
}


double LMap::getManhatenDistance(int node1, int node2) {
    double node1X = this->node_list[node1].x;
    double node1Y = this->node_list[node1].y;
    double node2X = this->node_list[node2].x;
    double node2Y = this->node_list[node2].y;
    return fabs(node1X - node2X) + fabs(node1Y - node2Y);
}

void LMap::ReadMap()
{
    MapReader myMapReader;
    std::vector<physical_node> physical_nodes = myMapReader.readPhysicalNodes();
    std::vector<physical_link> physical_links = myMapReader.readPhysicalLinks();

    std::vector<logical_node> logical_nodes;

    for (int i = 0; i < physical_nodes.size(); i++) {
        logical_node temp;
        temp.adjacent_nodes = physical_nodes[i].phy_adjacent_nodes;
        temp.logical_node_id = physical_nodes[i].physical_node_id;
        temp.physical_node_id = physical_nodes[i].physical_node_id;
        logical_nodes.push_back(temp);
    }

    std::vector<XOY> temp_node_list;
    for (int i = 0; i < logical_nodes.size(); i++) {
        XOY temp_node;
        temp_node.x = physical_nodes[logical_nodes[i].physical_node_id].x;
        temp_node.y = physical_nodes[logical_nodes[i].physical_node_id].y;
        temp_node_list.push_back(temp_node);
    }
    this->node_list = temp_node_list;

    std::vector<std::vector<int> > temp_neighbour_node_list;
    for (int i = 0; i < logical_nodes.size(); i++) {
        temp_neighbour_node_list.push_back(logical_nodes[i].adjacent_nodes);
    }
    this->neighbour_node_list = temp_neighbour_node_list;

    std::vector<std::vector<double> > temp_travel_time_list;
    for (int i = 0; i < logical_nodes.size(); i++) {
        std::vector<double> travel_times;
        for (int j = 0; j < logical_nodes[i].adjacent_nodes.size(); j++) {
            int current_physical_node_id = logical_nodes[i].physical_node_id;
            int other_physical_node_id = logical_nodes[logical_nodes[i].adjacent_nodes[j]].physical_node_id;
            double x0 = physical_nodes[current_physical_node_id].x;
            double y0 = physical_nodes[current_physical_node_id].y;
            double x1 = physical_nodes[other_physical_node_id].x;
            double y1 = physical_nodes[other_physical_node_id].y;
            travel_times.push_back(sqrt((x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1)));
        }
        temp_travel_time_list.push_back(travel_times);
    }
    this->travel_time_list = temp_travel_time_list;
    for (int i = 0; i < this->travel_time_list.size(); i++) {
        for (int j = 0; j < this->travel_time_list[i].size(); j++) {
            if (this->travel_time_list[i][j] > 14) {
                this->travel_time_list[i][j] = this->travel_time_list[i][j] * 3;
            }
        this->travel_time_list[i][j] = this->travel_time_list[i][j];

        }
    }


    std::vector<std::vector<int> > temp_lock_node_list;
    for (int i = 0; i < physical_nodes.size(); i++) {
        temp_lock_node_list.push_back(physical_nodes[i].lock_nodes);
    }
    this->lock_node_list = temp_lock_node_list;

    std::vector<std::vector<int> > temp_link_lock_node_list;
    for (int i = 0; i < physical_links.size(); i++) {
        temp_link_lock_node_list.push_back(physical_links[i].lock_nodes);
    }
    this->link_lock_node_list = temp_link_lock_node_list;

    map<int, map<int, int>> temp_link_id_map;
    for (int i = 0; i < physical_links.size(); i++) {
        map<int, map<int, int>>::iterator it;
        it = temp_link_id_map.find(physical_links[i].from_physical_node);
        if (it == temp_link_id_map.end()) {
            map<int, int> temp;
            temp[physical_links[i].to_physical_node] = i;
            temp_link_id_map[physical_links[i].from_physical_node] = temp;
        } else {
            temp_link_id_map[physical_links[i].from_physical_node][physical_links[i].to_physical_node] = i;
        }
    }
    this->link_id_map = temp_link_id_map;

    return;
}



double LMap::findTravelTimeBetweenTwoNodes(int node1, int node2) {
    std::vector<int> tempNeighbourList = this->neighbour_node_list[node1];
    std::vector<double> tempTravelTimeList = this->travel_time_list[node1];
    int i;
    for (i = 0; i < tempNeighbourList.size(); i++) {
        if (tempNeighbourList[i] == node2) {
            break;
        }
    }
    return tempTravelTimeList[i];

}




void LMap::StoreRobotPath(int robot_id, route curr_route) {

    std::vector<int> vpoint = curr_route.vps;
    std::vector<double> tts = curr_route.tts;
    std::vector<double> wts = curr_route.wts;
    std::string vps_xy_str = "";
    std::string vps_str = curr_route.vps_str;
    std::string wts_str = "";
    std::string tts_str = "";
    std::string arr_and_depar_time_str = curr_route.arr_and_depar_time_str;
    for (int i = 0; i < vpoint.size(); i++) {
        vps_xy_str += this->double2str(this->node_list[vpoint[i]].x);
        vps_xy_str += ",";
        vps_xy_str += this->double2str(this->node_list[vpoint[i]].y);
        wts_str += this->double2str(wts[i] / this->time_stamp);
        tts_str += this->double2str(tts[i] / this->time_stamp);

        if (i < vpoint.size() - 1) {
            vps_xy_str += "|";
            wts_str += "|";
            tts_str += "|";
        }
    }
    redisReply *reply = (redisReply *)redisCommand(ctx, "HSET robot_path %d %s", robot_id, vps_xy_str.c_str());
    freeReplyObject(reply);
    redisReply *reply_vps = (redisReply *)redisCommand(ctx, "HSET robot_vpoint %d %s", robot_id, vps_str.c_str());
    freeReplyObject(reply_vps);
    redisReply *reply_wts = (redisReply *)redisCommand(ctx, "HSET robot_waittime %d %s", robot_id, wts_str.c_str());
    freeReplyObject(reply_wts);
    redisReply *reply_tts = (redisReply *)redisCommand(ctx, "HSET robot_traveltime %d %s", robot_id, tts_str.c_str());
    freeReplyObject(reply_tts);
    redisReply *reply_arr_and_depar_time = (redisReply *)redisCommand(ctx, "HSET robot_arr_and_depar_time %d %s", robot_id, arr_and_depar_time_str.c_str());
    freeReplyObject(reply_arr_and_depar_time);
}

route LMap::RetriveRobotPath(int robot_id) {

    route result;

    std::string vps_str;
    std::string wts_str;
    std::string tts_str;
    std::string arr_and_depar_time_str;

    redisReply *reply_vps = (redisReply *)redisCommand(ctx, "HGET robot_vpoint %d", robot_id);
    vps_str = reply_vps->str;
    freeReplyObject(reply_vps);
    redisReply *reply_wts = (redisReply *)redisCommand(ctx, "HGET robot_waittime %d", robot_id);
    wts_str = reply_wts->str;
    freeReplyObject(reply_wts);
    redisReply *reply_tts = (redisReply *)redisCommand(ctx, "HGET robot_traveltime %d", robot_id);
    tts_str = reply_tts->str;
    freeReplyObject(reply_tts);
    redisReply *reply_arr_and_depar_time = (redisReply *)redisCommand(ctx, "HGET robot_arr_and_depar_time %d", robot_id);
    arr_and_depar_time_str = reply_arr_and_depar_time->str;
    freeReplyObject(reply_arr_and_depar_time);

    std::vector<int> vps;
    std::vector<double> wts;
    std::vector<double> tts;
    std::vector<std::pair<double, double>> arr_and_depar_time;
    std::vector<string> temp_vps = this->split(vps_str, "|");
    std::vector<string> temp_wts = this->split(wts_str, "|");
    std::vector<string> temp_tts = this->split(tts_str, "|");
    std::vector<string> temp_arr_and_depar_time = this->split(arr_and_depar_time_str, "|");

    for (int i = 0 ; i < temp_vps.size(); i++) {
        vps.push_back(this->str2int(temp_vps[i]));
    }

    for (int i = 0; i < temp_wts.size(); i++) {
        wts.push_back(this->str2double(temp_wts[i]));
    }

    for (int i = 0; i < temp_tts.size(); i++) {
        tts.push_back(this->str2double(temp_tts[i]));
    }

    for (int i = 0; i < temp_arr_and_depar_time.size(); i++) {
        std::pair<double, double> temp_pair;
        std::vector<string> temp_string = this->split(temp_arr_and_depar_time[i], ",");
        temp_pair.first = this->str2double(temp_string[0]);
        temp_pair.second = this->str2double(temp_string[1]);
        arr_and_depar_time.push_back(temp_pair);
    }
    result.arr_and_depar_time = arr_and_depar_time;
    result.arr_and_depar_time_str = arr_and_depar_time_str;
    result.tts = tts;
    result.vps = vps;
    result.wts = wts;
    result.tts_str = tts_str;
    result.wts_str = wts_str;
    result.vps_str = vps_str;

    return result;
}

































void LMap::FunctionTest() {
    std::cout << "let's play, honey!" << std::endl;
    bool debug4printShortestPath = false;
    bool debug4mergeFeasibleArriveTime = false;
    bool debug4TimeWindowReductionForNode = false;
    bool debug4LockTimeWindow = false;
    bool debug4getLegalInterVals4Node = false;
    bool debug4MultiPP = true;
    if (debug4printShortestPath) {
        multimap<int, ruofan_time_node*> openList;
        std::vector<ruofan_time_node*> closeList;
        for (int i = 0; i < 2; i++) {
            ruofan_time_node * curr_node;
            curr_node = new ruofan_time_node;
            std::vector<std::pair<double, double> > pairVec;
            std::pair<double, double> pair1;
            pair1.first = i;
            pair1.second = INF;
            pairVec.push_back(pair1);
            curr_node->feasible_arrival_time = pairVec;
            curr_node->node_id = i;
            curr_node->travel_time_from_parent = 1;
            openList.insert(make_pair(90 - i, curr_node));
            closeList.push_back(curr_node);
        }

        for (int i = 2; i < 10; i++) {
            ruofan_time_node * curr_node;
            curr_node = new ruofan_time_node;
            std::vector<std::pair<double, double> > pairVec;
            std::pair<double, double> pair1;
            pair1.first = i;
            pair1.second = 15;
            pairVec.push_back(pair1);
            curr_node->feasible_arrival_time = pairVec;
            std::pair<double, double> pair2;
            pair2.first = i + 20;
            pair2.second = INF;
            pairVec.push_back(pair2);
            curr_node->feasible_arrival_time = pairVec;
            curr_node->node_id = i;
            curr_node->travel_time_from_parent = 1;
            openList.insert(make_pair(90 - i, curr_node));
            closeList.push_back(curr_node);
        }

        for (int i = 50; i < 52; i++) {
            ruofan_time_node * curr_node;
            curr_node = new ruofan_time_node;
            std::vector<std::pair<double, double> > pairVec;
            std::pair<double, double> pair1;
            pair1.first = i;
            pair1.second = INF;
            pairVec.push_back(pair1);
            curr_node->feasible_arrival_time = pairVec;
            curr_node->node_id = i;
            curr_node->travel_time_from_parent = 1;
            openList.insert(make_pair(90 - i, curr_node));
            closeList.push_back(curr_node);
        }

        for (int i = closeList.size() - 1; i >= 1; i--) {
            ruofan_time_node * curr_node;
            curr_node = closeList[i];
            curr_node->pre_node = closeList[i - 1];
        }
        printShortestPath(closeList[closeList.size() - 1]);
    }
    if (debug4mergeFeasibleArriveTime) {
        std::vector<std::pair<double, double> > pair1Vec;
        std::vector<std::pair<double, double> > pair2Vec;
        std::pair<double, double> tempPair;
        tempPair.first = 1;
        tempPair.second = 5;
        pair1Vec.push_back(tempPair);
        tempPair.first = 8;
        tempPair.second = 10;
        pair1Vec.push_back(tempPair);
        tempPair.first = 20;
        tempPair.second = 26;
        pair1Vec.push_back(tempPair);
        tempPair.first = 40;
        tempPair.second = INF;
        pair1Vec.push_back(tempPair);

        tempPair.first = 1;
        tempPair.second = 6;
        pair2Vec.push_back(tempPair);
        tempPair.first = 9;
        tempPair.second = 15;
        pair2Vec.push_back(tempPair);
        tempPair.first = 22;
        tempPair.second = 23;
        pair2Vec.push_back(tempPair);
        tempPair.first = 30;
        tempPair.second = 46;
        pair2Vec.push_back(tempPair);

        std::vector<std::pair<double, double> > result = this->merge_feasible_arrtime(pair1Vec, pair2Vec);
        int brk;
    }
    if (debug4TimeWindowReductionForNode) {
        this->init();
        if (this->node_locked_time_window.size() == 0) {
            for (int i = 0; i < 5000; i++) {
                std::vector<std::pair<double, double> > tempPairVec;
                this->node_locked_time_window.push_back(tempPairVec);
            }
        }
        std::vector<std::pair<double, double> > pairVec;
        std::pair<double, double> tempPair;
        tempPair.first = 0;
        tempPair.second = 2;
        pairVec.push_back(tempPair);
        tempPair.first = 1.5;
        tempPair.second = 2.5;
        pairVec.push_back(tempPair);
        tempPair.first = 4;
        tempPair.second = 6;
        pairVec.push_back(tempPair);
        tempPair.first = 7;
        tempPair.second = 8;
        pairVec.push_back(tempPair);
        tempPair.first = 7.5;
        tempPair.second = 9;
        pairVec.push_back(tempPair);
        tempPair.first = 10;
        tempPair.second = 11;
        pairVec.push_back(tempPair);
        tempPair.first = 11;
        tempPair.second = 13;
        pairVec.push_back(tempPair);
        tempPair.first = 13;
        tempPair.second = 15;
        pairVec.push_back(tempPair);
        tempPair.first = 17;
        tempPair.second = INF;
        pairVec.push_back(tempPair);
        for (int i = 0; i < this->node_locked_time_window.size(); i++) {
            this->node_locked_time_window[i] = pairVec;
        }

        this->TimeWindowReduction4Node();
    }
    if (debug4LockTimeWindow) {
        multimap<int, ruofan_time_node*> openList;
        std::vector<ruofan_time_node*> closeList;
        int cnt = 0;
        for (int i = 0; i < 2; i++) {
            ruofan_time_node * curr_node;
            curr_node = new ruofan_time_node;
            std::vector<std::pair<double, double> > pairVec;
            std::pair<double, double> pair1;
            pair1.first = i;
            pair1.second = INF;
            pairVec.push_back(pair1);
            curr_node->feasible_arrival_time = pairVec;
            curr_node->node_id = cnt * 2;
            curr_node->travel_time_from_parent = 1;
            openList.insert(make_pair(90 - i, curr_node));
            closeList.push_back(curr_node);
            cnt += 1;
        }

        for (int i = 2; i < 10; i++) {
            ruofan_time_node * curr_node;
            curr_node = new ruofan_time_node;
            std::vector<std::pair<double, double> > pairVec;
            std::pair<double, double> pair1;
            pair1.first = i;
            pair1.second = 15;
            pairVec.push_back(pair1);
            curr_node->feasible_arrival_time = pairVec;
            std::pair<double, double> pair2;
            pair2.first = i + 20;
            pair2.second = INF;
            pairVec.push_back(pair2);
            curr_node->feasible_arrival_time = pairVec;
            curr_node->node_id = cnt * 2;
            curr_node->travel_time_from_parent = 1;
            openList.insert(make_pair(90 - i, curr_node));
            closeList.push_back(curr_node);
            cnt += 1;
        }

        for (int i = 50; i < 52; i++) {
            ruofan_time_node * curr_node;
            curr_node = new ruofan_time_node;
            std::vector<std::pair<double, double> > pairVec;
            std::pair<double, double> pair1;
            pair1.first = i;
            pair1.second = INF;
            pairVec.push_back(pair1);
            curr_node->feasible_arrival_time = pairVec;
            curr_node->node_id = cnt * 2;
            curr_node->travel_time_from_parent = 1;
            openList.insert(make_pair(90 - i, curr_node));
            closeList.push_back(curr_node);
            cnt += 1;
        }

        for (int i = closeList.size() - 1; i >= 1; i--) {
            ruofan_time_node * curr_node;
            curr_node = closeList[i];
            curr_node->pre_node = closeList[i - 1];
        }

        route result = printShortestPath(closeList[closeList.size() - 1]);

        this->LockTimeWindow(result);
        this->TimeWindowReduction4Node();

        int brk = 1;
    }
    if (debug4getLegalInterVals4Node) {
        std::pair<double, double> tempPair;
        tempPair.first = 0;
        tempPair.second = 3;
        this->node_locked_time_window[0].push_back(tempPair);
        tempPair.first = 5;
        tempPair.second = 8;
        this->node_locked_time_window[0].push_back(tempPair);
        tempPair.first = 9;
        tempPair.second = 10;
        this->node_locked_time_window[0].push_back(tempPair);
        tempPair.first = 15;
        tempPair.second = INF;
        this->node_locked_time_window[0].push_back(tempPair);
        std::vector<std::pair<double, double> > result;
        result = this->getLegalInterVals4Node(0);
        int brk = 1;

    }
    if (debug4MultiPP) {
        std::vector<route> result;
        double start = time(NULL);
        std::vector<int> robot_numbers;
        //robot_numbers.push_back(1);
        robot_numbers.push_back(1);
        robot_numbers.push_back(2);
        robot_numbers.push_back(3);
        robot_numbers.push_back(4);
        robot_numbers.push_back(5);
        robot_numbers.push_back(6);
        double curr_time = time(NULL) + 100;
        std::vector<double> start_work_time;
        start_work_time.push_back(curr_time);
        start_work_time.push_back(curr_time);
        start_work_time.push_back(curr_time);
        start_work_time.push_back(curr_time);
        start_work_time.push_back(curr_time);
        start_work_time.push_back(curr_time);
        //start_work_time.push_back(curr_time + 120);
        this->init();
        std::vector<int> from_ids;
        from_ids.push_back(219);
        from_ids.push_back(4433);
        from_ids.push_back(218);
        from_ids.push_back(4365);
        from_ids.push_back(437);
        from_ids.push_back(4369);
        std::vector<int> to_ids;
        to_ids.push_back(4433);
        to_ids.push_back(219);
        to_ids.push_back(4365);
        to_ids.push_back(218);
        to_ids.push_back(4369);
        to_ids.push_back(437);
        result = this->serve(robot_numbers, from_ids, to_ids, start_work_time);
        double end = time(NULL);
        for (int i = 0; i < result.size(); i++) {
            std::cout << "Robot ID : " << robot_numbers[i] << std::endl;
            std::cout << "vps:" << result[i].vps_str << std::endl;
            std::cout << "wts:" << result[i].wts_str << std::endl;
            std::cout << "tts:" << result[i].tts_str << std::endl;
            std::cout << std::endl;
            this->StoreRobotPath(i + 1, result[i]);
        }
        std::cout << "runtime : " << end - start << std::endl;
    }

    return;
}

void LMap::PrintRobotPath(int robot_id, route curr_route) {
        std::cout << "Robot ID : " << robot_id << std::endl;
        std::cout << "vps:" << curr_route.vps_str << std::endl;
        std::cout << "wts:" << curr_route.wts_str << std::endl;
        std::cout << "tts:" << curr_route.tts_str << std::endl;
        std::cout << std::endl;
        //this->StoreRobotPath(i + 1, result[i]);

}


void LMap::initLockArea(std::vector<int> needToInit) {
    for (int i = 0; i < needToInit.size(); i++) {
        int robot_id = needToInit[i];
        route r = this->RetriveRobotPath(robot_id);
        this->LockTimeWindow(r);
        this->TimeWindowReduction4Node();
    }

}

std::vector<route> LMap::serve(std::vector<int> robot_numbers, std::vector<int> from_ids, std::vector<int> to_ids, std::vector<double> start_working_time) {

    this->init();
    std::vector<route> result;

    std::vector<int> needToInit;
    for (int i = 1; i <= this->robot_number; i++) {
        bool flag = true;
        for (int j = 0; j < robot_numbers.size(); j++) {
            if (robot_numbers[j] == i) {
                flag = false;
                break;
            }
        }
        if (flag) {
            needToInit.push_back(i);
        }
    }
    this->GetRobotPositions();
    this->initLockArea(needToInit);
    result = multi_pp(robot_numbers, from_ids, to_ids, start_working_time);
    if (result.size() == robot_numbers.size()) {
        for (int i = 0; i < robot_numbers.size(); i++) {
            //this->StoreRobotPath(robot_numbers[i], result[i]);
        }
    }
    return result;
}

std::vector<route> LMap::multi_pp(std::vector<int> robot_numbers, std::vector<int> from_ids, std::vector<int> to_ids, std::vector<double> start_working_time) {
    std::vector<route> result;
    //this->init();
    for (int k = 0; k < from_ids.size(); k++) {
        set<int> openListSet;
        multimap <double, ruofan_time_node *> openList;
        std::vector<ruofan_time_node *> closeList;
        int start_id = from_ids[k];
        int end_id = to_ids[k];
        ruofan_time_node * startNode;
        startNode = new ruofan_time_node();
        startNode->node_id = start_id;
        startNode->pre_node = NULL;
        startNode->GValue = 0;
        startNode->HValue = getManhatenDistance(start_id, end_id);
        startNode->FValue = startNode->GValue + startNode->HValue;
        startNode->Punish = 0;
        startNode->FValue += startNode->Punish;
        std::vector<std::pair<double, double> > temp_feasible_arrive_time;
        std::pair<double, double> fesible4start;


        fesible4start.first = start_working_time[k];
        if (this->node_locked_time_window[startNode->node_id].size() != 0) {
            fesible4start.second = this->node_locked_time_window[startNode->node_id][0].first;
        } else {
            fesible4start.second = INF;
        }
        temp_feasible_arrive_time.push_back(fesible4start);
        std::vector<std::pair<double, double> > temp_legal_time;
//        temp_legal_time = this->getLegalInterVals4Node(startNode->node_id);

        std::vector<int> start_node_set;
        start_node_set = this->lock_node_list[startNode->node_id];
        temp_legal_time = this->getLegalInterVals4Node(startNode->node_id);
        for (int j = 0; j < start_node_set.size(); j++) {
            std::vector<std::pair<double, double> > curr_legal_time;
            curr_legal_time = this->getLegalInterVals4Node(start_node_set[j]);
            temp_legal_time = this->merge_feasible_arrtime(temp_legal_time, curr_legal_time);
        }

        temp_feasible_arrive_time = this->merge_feasible_arrtime(temp_feasible_arrive_time, temp_legal_time);
        startNode->feasible_arrival_time = temp_feasible_arrive_time;
        //Todo some bugs
        if (startNode->feasible_arrival_time.size() == 0) {
            //return result;
            std::vector<route> standDone;
            for (int i = 0; i < robot_numbers.size(); i++) {
                route onePointRoute;
                int robot_id = robot_numbers[i];
                int currPos = this->robot_positions[robot_id - 1];
                std::vector<int> vps;
                std::vector<double> wts;
                std::vector<double> tts;
                std::vector<std::pair<double, double>> arr_and_depar_time;
                vps.push_back(currPos);
                wts.push_back(0);
                tts.push_back(0);
                std::pair<double, double> tempPair;
                tempPair.first = start_working_time[0];
                tempPair.second = INF;
                arr_and_depar_time.push_back(tempPair);
                onePointRoute.vps = vps;
                onePointRoute.wts = wts;
                onePointRoute.tts = tts;
                onePointRoute.arr_and_depar_time = arr_and_depar_time;
                onePointRoute.vps_str = "";
                onePointRoute.vps_str += std::to_string(vps[0]);
                onePointRoute.wts_str = "";
                onePointRoute.wts_str += std::to_string(wts[0]);
                onePointRoute.tts_str = "";
                onePointRoute.tts_str += std::to_string(tts[0]);
                onePointRoute.arr_and_depar_time_str = "";
                onePointRoute.arr_and_depar_time_str += std::to_string(tempPair.first);
                onePointRoute.arr_and_depar_time_str += ",";
                onePointRoute.arr_and_depar_time_str += std::to_string(tempPair.second);
                standDone.push_back(onePointRoute);

            }

            std::cout << "standDone" << std::endl;
            return standDone;
        }

        if (startNode->feasible_arrival_time.size() > 0) {
            std::vector<std::pair<double, double> > tmp;
            tmp.push_back(startNode->feasible_arrival_time[0]);
            startNode->feasible_arrival_time = tmp;
        }

        double key = startNode->feasible_arrival_time[0].first;
        openList.insert(make_pair(key, startNode));
        openListSet.insert(startNode->node_id);


        while (true) {
            if (openList.size() == 0) {
                std::vector<route> standDone;
                for (int i = 0; i < robot_numbers.size(); i++) {
                    route onePointRoute;
                    int robot_id = robot_numbers[i];
                    int currPos = this->robot_positions[robot_id - 1];
                    std::vector<int> vps;
                    std::vector<double> wts;
                    std::vector<double> tts;
                    std::vector<std::pair<double, double>> arr_and_depar_time;
                    vps.push_back(currPos);
                    wts.push_back(0);
                    tts.push_back(0);
                    std::pair<double, double> tempPair;
                    tempPair.first = start_working_time[0];
                    tempPair.second = INF;
                    arr_and_depar_time.push_back(tempPair);
                    onePointRoute.vps = vps;
                    onePointRoute.wts = wts;
                    onePointRoute.tts = tts;
                    onePointRoute.arr_and_depar_time = arr_and_depar_time;
                    onePointRoute.vps_str = "";
                    onePointRoute.vps_str += std::to_string(vps[0]);
                    onePointRoute.wts_str = "";
                    onePointRoute.wts_str += std::to_string(wts[0]);
                    onePointRoute.tts_str = "";
                    onePointRoute.tts_str += std::to_string(tts[0]);
                    onePointRoute.arr_and_depar_time_str = "";
                    onePointRoute.arr_and_depar_time_str += std::to_string(tempPair.first);
                    onePointRoute.arr_and_depar_time_str += ",";
                    onePointRoute.arr_and_depar_time_str += std::to_string(tempPair.second);
                    standDone.push_back(onePointRoute);

                }

                std::cout << "standDone" << std::endl;
                return standDone;
                //return result;
            }
            ruofan_time_node * parent_node;
            parent_node = openList.begin()->second;


            closeList.push_back(parent_node);
            openList.erase(openList.begin());
            if (parent_node->node_id == end_id) {
                break;
            } else {
                std::vector<int> tempNeighbourList;
                tempNeighbourList = this->neighbour_node_list[parent_node->node_id];
                std::vector<double> tempTravelTimeList;
                tempTravelTimeList = this->travel_time_list[parent_node->node_id];
                for (int i = 0; i < tempNeighbourList.size(); i++) {
                    if (openListSet.count(tempNeighbourList[i]) > 0) {
                        continue;
                    }
                    ruofan_time_node * curr_time_node;
                    curr_time_node = new ruofan_time_node();
                    curr_time_node->pre_node = parent_node;
                    curr_time_node->node_id = tempNeighbourList[i];
                    curr_time_node->travel_time_from_parent = tempTravelTimeList[i];
                    temp_feasible_arrive_time = parent_node->feasible_arrival_time;
                    for (int j = 0; j < temp_feasible_arrive_time.size(); j++) {
                        temp_feasible_arrive_time[j].first += curr_time_node->travel_time_from_parent;
                        if (temp_feasible_arrive_time[j].second != INF) {
                            temp_feasible_arrive_time[j].second += curr_time_node->travel_time_from_parent;
                        }
                    }
                    std::vector<int> curr_node_set;
                    curr_node_set = this->lock_node_list[curr_time_node->node_id];
                    temp_legal_time = this->getLegalInterVals4Node(curr_time_node->node_id);
                    for (int j = 0; j < curr_node_set.size(); j++) {
                        std::vector<std::pair<double, double> > curr_legal_time;
                        curr_legal_time = this->getLegalInterVals4Node(curr_node_set[j]);
                        temp_legal_time = this->merge_feasible_arrtime(temp_legal_time, curr_legal_time);
                    }

                    //2.18 change
                    if (curr_time_node->travel_time_from_parent > 1.01) {
                        std::vector<int> new_parent_node_set = this->lock_node_list[parent_node->node_id];
                        std::vector<std::pair<double, double> > new_temp_legal_time = this->getLegalInterVals4Node(parent_node->node_id);
                        for (int j = 0; j < new_parent_node_set.size(); j++) {
                            std::vector<std::pair<double, double> > curr_legal_time;
                            curr_legal_time = this->getLegalInterVals4Node(new_parent_node_set[j]);
                            new_temp_legal_time = this->merge_feasible_arrtime(new_temp_legal_time, curr_legal_time);
                        }
                        temp_legal_time = this->merge_feasible_arrtime(new_temp_legal_time, temp_legal_time);
                    }


                    temp_feasible_arrive_time = this->merge_feasible_arrtime(temp_feasible_arrive_time, temp_legal_time);
                    curr_time_node->feasible_arrival_time = temp_feasible_arrive_time;
                    if (curr_time_node->feasible_arrival_time.size() == 0) {
                        continue;
                    }
                    curr_time_node->GValue = curr_time_node->feasible_arrival_time[0].first - start_working_time[k];
                    curr_time_node->HValue = this->getManhatenDistance(curr_time_node->node_id, end_id);
                    curr_time_node->FValue = curr_time_node->GValue + curr_time_node->HValue;
                    curr_time_node->Punish = parent_node->Punish;
                    if (curr_time_node->travel_time_from_parent > 10) {
                        curr_time_node->Punish += curr_time_node->travel_time_from_parent;
                    }
                    curr_time_node->FValue += curr_time_node->Punish;
                    openList.insert(make_pair(curr_time_node->FValue, curr_time_node));
                    openListSet.insert(curr_time_node->node_id);
                }
            }
        }
        route temp_route;
        temp_route = this->printShortestPath(closeList[closeList.size() - 1]);
        this->LockTimeWindow(temp_route);
        this->TimeWindowReduction4Node();
        result.push_back(temp_route);
    }
    return result;
}

route LMap::printShortestPath(struct ruofan_time_node* current_node) {

    route result;

    std::vector<ruofan_time_node> nodeVec;
    ruofan_time_node * iter = current_node;
    while (iter != NULL) {
        ruofan_time_node tmp;
        tmp = *iter;
        nodeVec.insert(nodeVec.begin(), tmp);
        iter = iter->pre_node;
    }

    std::vector<std::pair<double,double> > myList;

    this->printShortestPathHelper(nodeVec, myList, 0);

    result = this->generateRoute(nodeVec, myList);


    return result;
}

int LMap::printShortestPathHelper(std::vector<ruofan_time_node> &nodeVec, std::vector<std::pair<double, double> > &myList, int depth) {
    if (depth == nodeVec.size()) {
        return 1;
    }
    ruofan_time_node curr_node = nodeVec[depth];
    std::vector<std::pair<double, double> > curr_feasible_arrive_time = curr_node.feasible_arrival_time;
    for (int i = 0; i < curr_feasible_arrive_time.size(); i++) {
        if (depth == 0) {
            myList.push_back(curr_feasible_arrive_time[i]);
            int result = printShortestPathHelper(nodeVec, myList, depth + 1);
            if (result == 1) {
                return 1;
            } else {
                myList.pop_back();
            }
        } else {
            std::pair<double, double> lastPair = myList[myList.size() - 1];
            std::pair<double, double> currentPair = curr_feasible_arrive_time[i];
            if (lastPair.second + curr_node.travel_time_from_parent < currentPair.first) {
                continue;
            } else {
                myList.push_back(curr_feasible_arrive_time[i]);
                int result = printShortestPathHelper(nodeVec, myList, depth + 1);
                if (result == 1) {
                    return 1;
                } else {
                    myList.pop_back();
                }
            }
        }
    }

    return -1;
}

route LMap::generateRoute(std::vector<ruofan_time_node> nodeVec, std::vector<std::pair<double, double> > myList) {
    route result;
    std::vector<int> vps;
    std::vector<double> tts;
    std::vector<double> wts;
    std:vector<std::pair<double, double>> arr_and_depar_time;

    for (int i = 0; i < nodeVec.size(); i++) {
        int curr_node = nodeVec[i].node_id;
        double travel_time;
        if (i < nodeVec.size() - 1) {
            travel_time = nodeVec[i + 1].travel_time_from_parent;
        } else {
            travel_time = 0;
        }
        double wait_time;
        if (i < nodeVec.size() - 1) {
            wait_time = myList[i + 1].first - myList[i].first - travel_time;
            if (abs(wait_time) < 0.0001) {
                wait_time = 0;
            }
        } else {
            wait_time = 0;
        }
        vps.push_back(curr_node);
        tts.push_back(travel_time);
        wts.push_back(wait_time);
        std::pair<double, double> temp_pair;
        if (i < nodeVec.size() - 1) {
            temp_pair.first = myList[i].first;
            temp_pair.second = temp_pair.first + wait_time;
        } else {
            temp_pair.first = myList[i].first;
            temp_pair.second = myList[i].second;
        }
        arr_and_depar_time.push_back(temp_pair);

    }
    result.vps = vps;
    result.tts = tts;
    result.wts = wts;
    result.arr_and_depar_time = arr_and_depar_time;


    std::string vps_str = "";
    std::string tts_str = "";
    std::string wts_str = "";
    std::string arr_and_depar_time_str = "";
    for (int i = 0; i < vps.size(); i++) {
        vps_str += this->int2str(vps[i]);
        tts_str += this->double2str(tts[i]);
        wts_str += this->double2str(wts[i]);
        arr_and_depar_time_str += this->double2str(arr_and_depar_time[i].first);
        arr_and_depar_time_str += ",";
        arr_and_depar_time_str += this->double2str(arr_and_depar_time[i].second);
        if (i < nodeVec.size() - 1) {
            vps_str += "|";
            tts_str += "|";
            wts_str += "|";
            arr_and_depar_time_str += "|";
        }
    }
    result.vps_str = vps_str;
    result.tts_str = tts_str;
    result.wts_str = wts_str;
    result.arr_and_depar_time_str = arr_and_depar_time_str;

    return result;
}

void LMap::LockTimeWindow(route input) {
    LockTimeWindow4Area(input);
}

void LMap::LockTimeWindow4Area(route input) {
    std::vector<int> vps = input.vps;
    std::vector<double> tts = input.tts;
    std::vector<double> wts = input.wts;
    std::vector<std::pair<double, double>> arr_and_depar_time = input.arr_and_depar_time;
    double curr_arr_time = arr_and_depar_time[0].first;
    double wait_time = 0;

    for (int i = 0; i < vps.size(); i++) {

        std::vector<int> curr_node_set;
        //curr_node_set.push_back(vps[i]);
        for (int j = 0; j < this->lock_node_list[vps[i]].size(); j++) {
            curr_node_set.push_back(this->lock_node_list[vps[i]][j]);
        }
        wait_time = wts[i];
        double begin_time = curr_arr_time;
        //double end_time = curr_arr_time + wait_time + this->time_stamp ;
        double end_time = curr_arr_time + wait_time + 2.0 * (1 + 1) ;


        if (i == vps.size() - 1) {
            end_time = INF;
        }
        for (int j = 0; j < curr_node_set.size(); j++) {
            LockTimeWindow4Node(curr_node_set[j], begin_time, end_time);
        }
        curr_arr_time += tts[i];
        curr_arr_time += wait_time;
    }



}

void LMap::LockTimeWindow4Node(int node_id, double begin_time, double end_time) {
    std::pair<double, double> tempPair;
    tempPair.first = begin_time;
    tempPair.second = end_time;
    this->node_locked_time_window[node_id].push_back(tempPair);
}


std::vector<std::pair<double, double> > LMap::merge_feasible_arrtime(std::vector<std::pair<double, double> > pair1_vec, std::vector<std::pair<double, double> > pair2_vec) {
    std::vector<std::pair<double, double> > result;
    for (int i = 0; i < pair1_vec.size(); i++) {
        for (int j = 0; j < pair2_vec.size(); j++) {
            std::pair<double, double> tempPair;
            tempPair = this->merge_feasible_arrtime_helper(pair1_vec[i],pair2_vec[j]);
            if (tempPair.first < tempPair.second) {
                result.push_back(tempPair);
            }
        }
    }
    result = this->combine_intervals(result);
    return result;
}

std::pair<double, double> LMap::merge_feasible_arrtime_helper(std::pair<double, double> pair1, std::pair<double, double> pair2) {
    double first = max(pair1.first, pair2.first);
    double second = min(pair1.second, pair2.second);
    std::pair<double, double> result;
    result.first = first;
    result.second = second;
    return result;
}

std::vector<std::pair<double, double> > LMap::combine_intervals(std::vector<std::pair<double, double> > origin_intervals) {
    std::vector<std::pair<double, double> > result;
    result = origin_intervals;
    for (int i = 0; i < result.size(); i++) {
        if (i < result.size() - 1) {
            if (result[i + 1].first <= result[i].second) {
                result[i].second = result[i + 1].second;
                result.erase(result.begin() + i);
                i--;
            }
        }
    }
    return result;
}

void LMap::TimeWindowReduction4Node() {
    for (int i = 0; i < this->node_locked_time_window.size(); i++) {
        std::vector<std::pair<double, double> > curr_pair_vec;
        curr_pair_vec = this->node_locked_time_window[i];
        curr_pair_vec = this->TimeWindowReduction4NodeHelper(curr_pair_vec);
        this->node_locked_time_window[i] = curr_pair_vec;
    }
}

std::vector<std::pair<double, double> > LMap::TimeWindowReduction4NodeHelper(std::vector<std::pair<double, double> > input) {
    std::vector<std::pair<double, double> > result = input;
    for (int i = 0; i < result.size(); i++) {
        if (i < result.size() - 1) {
            std::pair<double, double> currPair = result[i];
            std::pair<double, double> nextPair = result[i + 1];
            if (currPair.second >= nextPair.first) {
                std::pair<double, double> newPair;
                newPair.first = min(currPair.first, nextPair.first);
                newPair.second = max(nextPair.second, currPair.second);
                result.erase(result.begin() + i);
                result[i] = newPair;
                i--;
            }


        }
    }
    return result;
}

std::vector<std::pair<double, double> > LMap::getLegalInterVals4Node(int node_id) {
    std::vector<std::pair<double, double> > locked_Intervals = this->node_locked_time_window[node_id];
    std::vector<std::pair<double, double> > result;
    if (locked_Intervals.size() == 0) {
        std::pair<double, double> tempPair;
        tempPair.first = 0;
        tempPair.second = INF;
        result.push_back(tempPair);
    } else {
        if (locked_Intervals[0].first > 0) {
            std::pair<double, double> tempPair;
            tempPair.first = 0;
            tempPair.second = locked_Intervals[0].first;
            result.push_back(tempPair);
        }
        for (int i = 0; i < locked_Intervals.size(); i++) {
            double begin_time = locked_Intervals[i].second;
            double end_time;
            if (i == locked_Intervals.size() - 1) {
                if (locked_Intervals[i].second != INF) {
                    end_time = INF;
                } else {
                    continue;
                }
            } else {
                end_time = locked_Intervals[i + 1].first;
            }
            std::pair<double, double> tempPair;
            tempPair.first = begin_time;
            tempPair.second = end_time;
            result.push_back(tempPair);
        }
    }
    return result;
}

void LMap::GetRobotPositions() {
    std::vector<int> positions;
    for (int i = 1; i <= this->robot_number; i++) {
        redisReply * reply = (redisReply *) redisCommand(ctx, "HGET robot_position %d", i);
        int tempPos = std::stoi(reply->str);
        positions.push_back(tempPos);
        freeReplyObject(reply);
    }
    this->robot_positions = positions;
}


