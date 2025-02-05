#include "src/data.hpp"
#include "src/highway.hpp"
#include "src/lattice.hpp"
#include "src/road_networks.hpp"

#include <cmath>

void find_for_names(const std::vector<std::string>& names, unsigned offset, unsigned total, unsigned Q = 1)
{
	WallTimer timer;

	// for (const auto& state : names)
	for (unsigned i = 0; i < names.size(); ++i)
	{
		if (i % total != offset)
		{
			continue;
		}

		std::string state = names[i];

		timer.start("Loading contraction hierarchy for " + state);

		auto ch = get_contraction_hierarchy(state);

		timer.print();

		unsigned k = std::lround(std::log2(ch.node_count()));

		if (has_optimal_clustering_exponent_data(k, Q).contains(state))
		{
			printf("Optimal clustering exponent for %s already exists\n", state.c_str());
			continue;
		}

		Highway h(state, ch, k, Q, 1.5);

		timer.start("Determining optimal clustering exponent for " + state);

		double clustering_exponent = h.estimate_optimal_clustering_exponent();

		timer.print();

		printf("Optimal clustering exponent for %s: %f\n", state.c_str(), clustering_exponent);

		save_optimal_clustering_exponent_data(state, k, Q, clustering_exponent);
	}
}

void find_for_lattices(unsigned dimension, bool wrap_around = true, unsigned Q = 1)
{
	WallTimer timer;

	for (unsigned side_length = 16; ; side_length *= 8)
	{
		std::string name = std::to_string(dimension) + "D_" + std::to_string(side_length);
		name += wrap_around ? "-wrap" : "";

		unsigned num_nodes = std::pow(side_length, dimension);
		unsigned k = std::lround(std::log2(num_nodes));

		if (has_optimal_clustering_exponent_data(k, Q).contains(name))
		{
			printf("Optimal clustering exponent for %s already exists\n", name.c_str());
			continue;
		}

		timer.start("Generating lattice for " + name);

		Lattice lattice(side_length, dimension, wrap_around);

		timer.print();

		timer.start("Determining optimal dimension for " + name);

		// because of wrap-around, every node is identical, so we can get the dimension estimate by looking at the balls of any arbitrary node
		auto balls = lattice.get_balls(0);
		double estimated_dimension = Graph::minimize_tight_c(balls, dimension);

		timer.print();

		estimated_dimension = std::round(estimated_dimension * 100) / 100;

		printf("Optimal dimension for %s: %f\n", name.c_str(), estimated_dimension);

		save_dimension_data(name, 0, 0, estimated_dimension);

		timer.start("Loading contraction hierarchy for " + name);

		auto ch = lattice.get_contraction_hierarchy();

		timer.print();

		Highway h(name, ch, k, Q, estimated_dimension);

		timer.start("Determining optimal clustering exponent for " + name);

		double clustering_exponent = h.estimate_optimal_clustering_exponent(estimated_dimension);

		timer.print();

		printf("Optimal clustering exponent for %s: %f\n", name.c_str(), clustering_exponent);

		save_optimal_clustering_exponent_data(name, k, Q, clustering_exponent);
	}
}

int main(int argc, char* argv[])
{
	// get offset and total as command line args

	unsigned offset = std::stoul(argv[1]);
	unsigned total = std::stoul(argv[2]);

	find_for_names(get_state_names(), offset, total);
	find_for_names(get_non_state_names(), offset, total);

	// find_for_lattices(3);

	return 0;
}