#include <string>
#include <vector>
#include "Singleton.h"
#include <map>
#include <regex>
#include "hiredis/hiredis.h"
#include <set>
#include <iomanip>
#include <time.h>
#include "mapreader.h"

using namespace std;


struct rf_task
{
    int from;
    int to;
    int isFinished = 1;
};

class TS
{
public:
    TS();
    ~TS();
    void ReadMap();
    void Init();
    void GetRobotPositions();
    void StoreRobotTask(int robot_id, rf_task t);
    void checkDeadLock();
    void setDeadLock(int a);

    std::vector<std::string> split(const string& str, const string& delim);
    std::string int2str(int num);
    int str2int(std::string num);

public:
    redisContext* ctx;
    MapReader myMapreader;
    std::vector<physical_node> physical_node_list;
    std::vector<int> yardPoints;
    std::vector<rf_task> RobotTasks;
    std::vector<int> robot_positions;
    std::vector<int> robot_with_box;
    int isDeadLock;

};
