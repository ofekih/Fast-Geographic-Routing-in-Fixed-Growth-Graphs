#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <routingkit/contraction_hierarchy.h>

struct Ball
{
	unsigned distance;
	unsigned count;
};

class Graph
{
public:
	Graph(unsigned num_nodes, bool directed = false);

	void add_edge(unsigned u, unsigned v, unsigned weight);

	const std::unordered_map<unsigned, unsigned>& get_neighbors(unsigned u) const;
	unsigned size() const noexcept;
	unsigned num_edges() const noexcept;

	RoutingKit::ContractionHierarchy get_contraction_hierarchy() const;

	std::vector<Ball> get_balls(unsigned u) const;

	unsigned connected_component_size(unsigned u) const;

	static double tight_c(const std::vector<Ball>& balls, double alpha, unsigned num_to_skip = 0, unsigned min_distance = 0);

	static double minimize_tight_c(const std::vector<Ball>& balls, double guess, unsigned num_to_skip = 0, unsigned min_distance = 0, double fractional_difference = 5e-4);

	double estimate_optimal_dimension(double guess = 1.5, unsigned num_to_skip = 0, unsigned min_distance = 0, double tolerance = 2e-3) const;

private:
	std::vector<std::unordered_map<unsigned, unsigned>> _neighbors;
	bool _directed;
	unsigned _num_edges;
};