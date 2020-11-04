#ifndef MAPREADER_H
#define MAPREADER_H

#include <vector>
#include "hiredis/hiredis.h"
#include "cJSON.h"
#include <iostream>

struct physical_node {
    int physical_node_id;
    double x;
    double y;
    int type;
    std::vector<int> phy_adjacent_nodes;
    std::vector<int> logicl_nodes;
    std::vector<int> lock_nodes;
};

struct physical_link {
    int physical_link_id;
    int from_physical_node;
    int to_physical_node;
    std::vector<int> lock_nodes;
};

struct logical_node {
    int logical_node_id;
    int physical_node_id;
    std::vector<int> adjacent_nodes;
};

class MapReader {
public:
    MapReader(){
        ctx = redisConnect("127.0.0.1", 6379);
        if(ctx->err)
        {
            std::cout<<"No connection with ctx_get"<<std::endl;
        } else {
            std::cout << "Successfully connected to Redis!" << std::endl;
        }
    };
    ~MapReader(){};
public:
    std::vector<physical_node> readPhysicalNodes();
    std::vector<physical_link> readPhysicalLinks();
    std::vector<logical_node> readLogicalNodes();
    redisContext* ctx;
};






#endif // MAPREADER_H
