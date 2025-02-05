#pragma once

#include <routingkit/contraction_hierarchy.h>

#include <string>
#include <thread>
#include <functional>
#include <vector>

// static const unsigned NUM_THREADS = 1;
static const unsigned NUM_THREADS = std::thread::hardware_concurrency();

class Highway
{
public:
	Highway(const std::string& name, const RoutingKit::ContractionHierarchy& ch, unsigned k, unsigned Q, double clustering_exponent);

	const std::string& name() const noexcept;

	void initialize() noexcept;

	void for_each_long_distance_contact(unsigned u, const std::function<void(unsigned)>& callback) const noexcept;

	unsigned get_distance(unsigned s, unsigned t) const noexcept;

	unsigned get_greedy_path_length(unsigned start, unsigned end) const noexcept;

	double get_total_greedy_path_length(unsigned num_trials) const noexcept;

	double get_average_greedy_path_length(unsigned batch_size = 1000, double fractional_difference = 5e-4) noexcept;

	double estimate_optimal_clustering_exponent(double guess = 1.5, unsigned batch_size = NUM_THREADS * 100, double tolerance = 5e-3) noexcept;

private:
	const std::string& _name;
	const RoutingKit::ContractionHierarchy& _contraction_hierarchy;
	unsigned _k;
	unsigned _Q;
	double _clustering_exponent;

	unsigned _num_nodes;

	std::vector<unsigned> _highway_nodes; 
	std::vector<bool> _is_highway_node;
};
