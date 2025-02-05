#include <cmath>
#include <unordered_map>
#include <string>

#include "src/data.hpp"
#include "src/road_networks.hpp"
#include "src/highway.hpp"

static const std::unordered_map<std::string, double> DIMENSION_ESTIMATES = {
	{"AK", 1.18 },
	{"AL", 1.6 },
	{"AR", 1.58 },
	{"AZ", 1.51 },
	{"CA", 1.53 },
	{"DC", 1.69 },
	{"DE", 1.48 },
	{"FL", 1.54 },
	{"GA", 1.64 },
	{"HI", 1.47 },
	{"IA", 1.55 },
	{"ID", 1.44 },
	{"IL", 1.58 },
	{"IN", 1.6 },
	{"KS", 1.57 },
	{"KY", 1.58 },
	{"LA", 1.56 },
	{"MA", 1.6 },
	{"ME", 1.5 },
	{"MI", 1.54 },
	{"MN", 1.57 },
	{"MO", 1.57 },
	{"MS", 1.6 },
	{"MT", 1.47 },
	{"NC", 1.62 },
	{"ND", 1.54 },
	{"NE", 1.5 },
	{"NH", 1.5 },
	{"NJ", 1.63 },
	{"NM", 1.53 },
	{"NV", 1.43 },
	{"NY", 1.54 },
	{"OH", 1.6 },
	{"OK", 1.55 },
	{"OR", 1.54 },
	{"PA", 1.61 },
	{"RI", 1.57 },
	{"SC", 1.62 },
	{"SD", 1.52 },
	{"TN", 1.57 },
	{"TX", 1.63 },
	{"UT", 1.45 },
	{"VA", 1.58 },
	{"VT", 1.5 },
	{"WA", 1.54 },
	{"WI", 1.55 },
	{"WV", 1.49 },
	{"WY", 1.52 },
	{"CO", 1.51 },
	{"CT", 1.65 },
	{"MD", 1.58 },
};

int main(int argc, char* argv[])
{
	unsigned offset = std::stoul(argv[1]);
	unsigned total = std::stoul(argv[2]);

	WallTimer timer;

	unsigned i = -1;
	for (const auto& [state, dimension] : DIMENSION_ESTIMATES)
	{
		++i;
		if (i % total != offset)
		{
			continue;
		}

		timer.start("Loading contraction hierarchy for " + state);

		auto ch = get_contraction_hierarchy(state);

		timer.print();

		unsigned k = std::lround(std::log2(ch.node_count()));

		Highway h2(state, ch, k, 1, 2);

		timer.start("Determining greedy path length when alpha = 2");

		double path_length_2 = h2.get_average_greedy_path_length(1000, 1e-2);

		timer.print();

		printf("Greedy path length for %s when alpha = 2: %f\n", state.c_str(), path_length_2);

		// now when alpha = dimension
		Highway h_dimension(state, ch, k, 1, dimension);

		timer.start("Determining greedy path length when alpha = " + std::to_string(dimension));

		double path_length_dimension = h_dimension.get_average_greedy_path_length(1000, 1e-2);

		timer.print();

		printf("Greedy path length for %s when alpha = %f: %f\n", state.c_str(), dimension, path_length_dimension);

		save_optimal_vs_dimension_data(state, dimension, 2, path_length_dimension, path_length_2);
	}
}
