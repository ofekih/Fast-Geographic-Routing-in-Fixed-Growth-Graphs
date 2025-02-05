#include <cmath>

#include "src/data.hpp"
#include "src/highway.hpp"
#include "src/road_networks.hpp"

int main(int argc, char* argv[] )
{
	// should get num_to_skip and min_distance from command line
	if (argc != 3)
	{
		printf("Usage: %s <num_to_skip> <min_distance>\n", argv[0]);
		return 1;
	}

	unsigned num_to_skip = std::stoul(argv[1]);
	unsigned min_distance = std::stoul(argv[2]);

	WallTimer timer;

	auto already_computed = has_dimension_data(num_to_skip, min_distance);

	for (const auto& state : get_non_state_names())
	{
		if (already_computed.contains(state))
		{
			printf("Dimension estimate for %s already exists\n", state.c_str());
			continue;
		}

		timer.start("Loading graph for " + state);

		auto g = get_graph(state);

		timer.print();

		timer.start("Determining optimal dimension for " + state);

		double dimension = g.estimate_optimal_dimension(1.5, num_to_skip, min_distance);

		timer.print();

		// round to 2 decimal places
		dimension = std::round(dimension * 100) / 100;

		printf("Optimal dimension for %s: %f\n", state.c_str(), dimension);

		save_dimension_data(state, num_to_skip, min_distance, dimension);
	}

	return 0;
}