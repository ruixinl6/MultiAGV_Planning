/**
 * 
 * 
 * 
*/
#ifndef _MAP_H_
#define _MAP_H_
#include <string>
#include <vector>
#include "Singleton.h"
#include <map>
#include <regex>
#include "hiredis/hiredis.h"
#include <set>
#include <iomanip>
#include <time.h>

using namespace std;

/**
 * @brief The XOY struct
 * This structure is used to store the map's [x,y] information.
 */
struct XOY {
    double x;
    double y;
};

/**
 * @brief The ruofanNode struct
 * This structure is used to create single_path_planning(barely A-Star).
 * If you don't know what F,G,H means, you can search A-Star algorithm in baidu to understand it.
 */
struct ruofanNode {
    int node_id;
    int parent_node_id;
    double FValue = 0;
    double GValue = 0;
    double HValue = 0;

};

/**
 * @brief The route struct
 * It contains the vps, wts, tts result for multi-path-planning
 * vps is vector of point
 * wts is vector of waittime
 * tts is vector of traveltime
 */
struct route {
    std::string vps_str = "";
    std::string wts_str = "";
    std::string tts_str = "";
    std::vector<int> vps;
    std::vector<double> wts;
    std::vector<double> tts;
    std::vector<std::pair<double, double>> arr_and_depar_time;
    std::string arr_and_depar_time_str = "";
};

/**
 * @brief The ruofan_time_node struct
 * This structure is used in multi-path-planning but not used in single-path-planning
 * @param parent_node : Like the parent node in A-Star
 * @param robot_id : For now, this is not used, don't be confused by that.
 * @param departure_time : For now, this is not used, don't be confused by that.
 * @param earliest_arrive_time : For now, this is equal to arrive_time, don't be confused by that.
 */
struct ruofan_time_node {
    int  node_id;
    double FValue = 0;
    double GValue = 0;
    double HValue = 0;
    double Punish = 0;
    std::vector<std::pair<double, double> > feasible_arrival_time;
    double travel_time_from_parent;
    struct ruofan_time_node* pre_node = NULL;
};

class LMap :public Singleton<LMap>
{
public:
    LMap();
    void ReadMap();
    ~LMap();

    double time_stamp = 1;
    double speed = 10;
    int robot_number = 3;


    std::vector<XOY> node_list;
    std::vector<std::vector<int> > neighbour_node_list;
    std::vector<std::vector<double> > travel_time_list;
    std::vector<std::vector<int> > lock_node_list;
    std::vector<std::vector<int> > link_lock_node_list;
    map<int, map<int, int>> link_id_map;

    std::vector<std::vector<double> > fastmap;

    std::vector<double> getDoubleInStringOfNeighborList(std::string NeighbornodeOfList);

    std::vector<string> split(const string& str, const string& delim);

    redisContext* ctx;

    std::string int2str(int num);
    int str2int(std::string num);

    std::vector<int> getIntInStringOfNeighborList(std::string NeighbornodeOfList);
    std::vector<std::vector<int>> RetriveNeighborNodeList();
    std::vector<std::vector<double>> RetriveTravelTimeList();
    std::vector<std::vector<int>> RetriveLockNodeList();
    std::vector<std::vector<double> > ReadFastMap();

    void GetRobotPositions();

    std::string double2str(double num);
    double str2double(std::string num);

    double getDistance(int node1, int node2);
    double getManhatenDistance(int node1, int node2);
    double getFastMapDistance(int node1, int node2);
    void StoreRobotPath(int robot_id, route curr_route);
    void PrintRobotPath(int robot_id, route curr_route);

    route RetriveRobotPath(int robot_id);


    std::vector<int> from_ids;
    std::vector<int> to_ids;

    std::vector<int> robot_positions;


    std::vector<std::vector<std::pair<double, double> > > node_locked_time_window;

    map<int, map<int, std::vector<std::pair<double, double> > > > edge_lock_time_window;

public:
    std::vector<route> serve(std::vector<int> robot_numbers, std::vector<int> from_ids, std::vector<int> to_ids, std::vector<double> start_working_time);
    std::vector<route> multi_pp(std::vector<int> robot_numbers, std::vector<int> from_ids, std::vector<int> to_ids, std::vector<double> start_working_time);
    void FunctionTest();
    void init();
    void initLockArea(std::vector<int> needToInit);


private:
    friend class Singleton<LMap>;
    double findTravelTimeBetweenTwoNodes(int node1, int node2);


    route printShortestPath(struct ruofan_time_node* current_node);
    int printShortestPathHelper(std::vector<ruofan_time_node> &nodeVec, std::vector<std::pair<double, double> > &myList, int depth);
    route generateRoute(std::vector<ruofan_time_node> nodeVec, std::vector<std::pair<double, double> > myList);
    void LockTimeWindow(route input);
    void LockTimeWindow4Area(route input);
    void LockTimeWindow4Node(int node_id, double begin_time, double end_time);


    std::vector<std::pair<double, double> > getLegalInterVals4FarNodes(int curr_node_id, int prev_node_id);

    std::vector<std::pair<double, double> > merge_feasible_arrtime(std::vector<std::pair<double, double> > pair1_vec, std::vector<std::pair<double, double> > pair2_vec);
    std::pair<double, double> merge_feasible_arrtime_helper(std::pair<double, double> pair1, std::pair<double, double> pair2);
    std::vector<std::pair<double, double> > combine_intervals(std::vector<std::pair<double, double> > origin_intervals);

    void TimeWindowReduction4Node();
    std::vector<std::pair<double, double> > TimeWindowReduction4NodeHelper(std::vector<std::pair<double, double> > input);

    std::vector<std::pair<double, double> > getLegalInterVals4Node(int node_id);

};

#endif
