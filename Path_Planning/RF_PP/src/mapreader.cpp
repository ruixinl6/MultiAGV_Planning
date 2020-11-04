#include "mapreader.h"

std::vector<physical_node> MapReader::readPhysicalNodes() {
    std::vector<physical_node> result;
    redisReply * replyId = (redisReply *)redisCommand(ctx, "lrange physical_node_list 0 -1");
    for(int i = 0 ; i < replyId->elements; i++)
    {
        std::string ids = replyId->element[i]->str;
        const char * data = ids.c_str();
        cJSON * json = cJSON_Parse(data);
        physical_node node;

        std::vector<int> lock_nodes;
        std::vector<int> phy_adjacent_nodes;
        std::vector<int> logical_nodes;

        int physical_node_id;
        double y;
        double x;

        cJSON * ptr = json->child;
        while (ptr != NULL) {
            std::string tempString = ptr->string;
            if (tempString == "x") {
                x = ptr->valuedouble;
            } else if (tempString == "physical_node_id") {
                physical_node_id = ptr->valueint;
            } else if (tempString == "phy_adjacent_nodes") {
                cJSON * tempPtr = ptr->child;
                while (tempPtr != NULL) {
                    phy_adjacent_nodes.push_back(tempPtr->valueint);
                    tempPtr = tempPtr->next;
                }
            } else if (tempString == "logical_nodes") {
//                cJSON * tempPtr = json->child;
//                while (tempPtr != NULL) {
//                    logical_nodes.push_back(tempPtr->valueint);
//                    tempPtr = tempPtr->next;
//                }
            } else if (tempString == "type") {

            } else if (tempString == "lock_nodes") {
                cJSON * tempPtr = ptr->child;
                while (tempPtr != NULL) {
                    lock_nodes.push_back(tempPtr->valueint);
                    tempPtr = tempPtr->next;
                }
            } else if (tempString == "y") {
                y = ptr->valuedouble;
            }
            ptr = ptr->next;
        }

//        int type;
//        type = std::stoi(json->child->next->next->next->next->next->next->valuestring);

        node.lock_nodes = lock_nodes;
        logical_nodes.push_back(physical_node_id);
        node.logicl_nodes = logical_nodes;
        node.physical_node_id = physical_node_id;
        node.phy_adjacent_nodes = phy_adjacent_nodes;
        node.x = x;
        node.y = y;
//        node.type = type;
        result.push_back(node);

    }
    freeReplyObject(replyId);
    return result;
}

std::vector<physical_link> MapReader::readPhysicalLinks() {
    std::vector<physical_link> result;
    redisReply * reply = (redisReply *)redisCommand(ctx, "lrange physical_link_list 0 -1");
    for (int i = 0; i < reply->elements; i++) {
        const char * data = reply->element[i]->str;
        cJSON * json = cJSON_Parse(data);
        physical_link link;

        int physical_link_id;
        int to_physical_node;
        int from_physical_node;
        std::vector<int> lock_nodes;

        cJSON * ptr = json->child;
        while (ptr != NULL) {
            std::string tempString = ptr->string;
            if (tempString == "from_physical_node") {
                from_physical_node = ptr->valueint;
            } else if (tempString == "physical_link_id") {
                physical_link_id = ptr->valueint;
            } else if (tempString == "to_physical_node") {
                to_physical_node = ptr->valueint;
            } else if (tempString == "lock_nodes") {
                cJSON * tempPtr = ptr->child;
                while (tempPtr != NULL) {
                    lock_nodes.push_back(tempPtr->valueint);
                    tempPtr = tempPtr->next;
                }
            }
            ptr = ptr->next;
        }


        link.from_physical_node = from_physical_node;
        link.to_physical_node = to_physical_node;
        link.lock_nodes = lock_nodes;
        link.physical_link_id = physical_link_id;
        result.push_back(link);
    }
    freeReplyObject(reply);
    return result;
}

std::vector<logical_node> MapReader::readLogicalNodes() {
    std::vector<logical_node> result;
    redisReply * reply = (redisReply *) redisCommand(ctx, "lrange logical_node_list 0 -1");
    for (int i = 0; i < reply->elements; i++) {
        const char * data = reply->element[i]->str;
        cJSON * json = cJSON_Parse(data);
        logical_node node;
        std::vector<int> adjacent_nodes;
        cJSON * ptr = json->child->child;
        while (ptr != NULL) {
            adjacent_nodes.push_back(ptr->valueint);
            ptr = ptr->next;
        }
        int physical_node_id = json->child->next->valueint;
        int logical_node_id = json->child->next->next->valueint;
        node.adjacent_nodes = adjacent_nodes;
        node.logical_node_id = logical_node_id;
        node.physical_node_id = physical_node_id;
        result.push_back(node);
    }
    freeReplyObject(reply);
    return result;
}
