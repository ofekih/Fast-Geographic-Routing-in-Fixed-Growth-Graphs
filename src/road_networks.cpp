#include "road_networks.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <routingkit/contraction_hierarchy.h>

namespace
{
	std::vector<std::string> get_names(bool is_state)
	{
		std::vector<std::string> names;

		for (const auto& entry : std::filesystem::directory_iterator(ROAD_NETWORK_DIRECTORY))
		{
			if (!entry.is_regular_file())
			{
				continue;
			}

			if (entry.path().extension().compare(RAW_NETWORK_EXTENSION))
			{
				continue;
			}

			std::string stem = entry.path().stem().string();
			if (is_state != (stem.size() == 2))
			{
				continue;
			}

			names.emplace_back(stem);
		}

		return names;
	}
}

std::vector<std::string> get_state_names()
{
	return get_names(true);
}

std::vector<std::string> get_non_state_names()
{
	return get_names(false);
}

Graph get_graph(const std::string& name)
{
	std::ifstream file(ROAD_NETWORK_DIRECTORY + name + RAW_NETWORK_EXTENSION);

	unsigned num_nodes;
	file >> num_nodes;

	Graph g(num_nodes);

	unsigned u, v;
	double weight;
	while (file >> u >> v >> weight)
	{
		g.add_edge(u, v, std::lround(weight));
	}

	return g;
}

RoutingKit::ContractionHierarchy get_contraction_hierarchy(const std::string& name)
{
	// check if contraction hierarchy is cached
	std::string ch_file = ROAD_NETWORK_DIRECTORY + name + CONTRACTION_HIERARCHY_NETWORK_EXTENSION;
	if (std::filesystem::exists(ch_file))
	{
		return RoutingKit::ContractionHierarchy::load_file(ch_file);
	}

	// if not, build it
	std::string raw_file = ROAD_NETWORK_DIRECTORY + name + RAW_NETWORK_EXTENSION;
	std::ifstream file(raw_file);

	std::vector<unsigned> tail, head, dist;
	unsigned u, v, num_nodes;
	double weight;

	file >> num_nodes;

	while (file >> u >> v >> weight)
	{
		head.push_back(u);
		tail.push_back(v);
		dist.push_back(std::lround(weight));

		head.push_back(v);
		tail.push_back(u);
		dist.push_back(std::lround(weight));
	}

	auto ch = RoutingKit::ContractionHierarchy::build(num_nodes, tail, head, dist);

	ch.save_file(ch_file);

	return ch;
}
