#pragma once

#include "graph.hpp"

#include <string>
#include <vector>

#include <routingkit/contraction_hierarchy.h>

static const std::string ROAD_NETWORK_DIRECTORY = "road_networks/";
static const std::string RAW_NETWORK_EXTENSION = ".raw";
static const std::string CONTRACTION_HIERARCHY_NETWORK_EXTENSION = ".ch";

std::vector<std::string> get_state_names();
std::vector<std::string> get_non_state_names();

Graph get_graph(const std::string& name);

RoutingKit::ContractionHierarchy get_contraction_hierarchy(const std::string& name);
