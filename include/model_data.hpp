#pragma once
#include <string>
#include <vector>
#include "json.hpp"  // or forward declare nlohmann::json if you want

struct Node {
    int id;
    std::vector<int> leavingArcIds;
    std::vector<int> arrivingArcIds;
    double slackCost;
    double theta;
};

struct Arc {
    int id;
    int sourceNode;
    int targetNode;
    bool initial;
    std::vector<double> capacity;
    std::vector<double> buildCost;
    std::vector<std::vector<double>> flowCost;
};

class ModelData {
public:
    int timePeriods;
    int commodities;
    std::vector<Node> nodes;
    std::vector<Arc> arcs;
    std::vector<std::vector<double>> deltaXiLb;
    std::vector<std::vector<double>> deltaXiUb;
    std::vector<std::vector<double>> muBar;
    std::vector<std::vector<double>> epsilon;
    std::vector<std::vector<double>> revenue;
    std::vector<double> omega;
    

    bool loadFromFile(const std::string& filename);
};
