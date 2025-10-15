#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include "json.hpp"
#include "model_data.hpp"

using json = nlohmann::json;

bool ModelData::loadFromFile(const std::string& filename) {
    std::ifstream input(filename);
    if (!input.is_open()) {
        std::cerr << "Could not open file " << filename << "\n";
        return false;
    }
    json j;
    input >> j;

    timePeriods = j["time_periods"];
    commodities = j["commodities"];

    nodes.clear();
    arcs.clear();

    for (const auto& c : j["nodes"]) {
        Node node;
        node.id = c["id"];
        node.leavingArcIds = c["leaving_arc_ids"].get<std::vector<int>>();
        node.arrivingArcIds = c["arriving_arc_ids"].get<std::vector<int>>();
        node.slackCost = c["c_v"];
        node.theta = c["theta"];
        nodes.push_back(node);
    }

    for (const auto& v : j["arcs"]) {
        Arc arc;
        arc.id = v["id"];
        arc.sourceNode = v["source_node"];
        arc.targetNode = v["target_node"];
        arc.initial = v["initial"];
        arc.capacity = v["C"].get<std::vector<double>>();
        arc.buildCost = v["c_x"].get<std::vector<double>>();
        arc.flowCost = v["c_u"].get<std::vector<std::vector<double>>>();
        arcs.push_back(arc);
    }


    deltaXiLb = j["delta_xi_lb"].get<std::vector<std::vector<double>>>();
    deltaXiUb = j["delta_xi_ub"].get<std::vector<std::vector<double>>>();
    muBar = j["mu_bar"].get<std::vector<std::vector<double>>>();
    epsilon = j["epsilon"].get<std::vector<std::vector<double>>>();
    revenue = j["R"].get<std::vector<std::vector<double>>>();
    omega = j["omega"].get<std::vector<double>>();

    return true;
}
