#include "src/graph.hpp"
#include "src/lattice.hpp"
#include "src/highway.hpp"

#include "src/road_networks.hpp"
#include "src/data.hpp"

#include <cmath>

#include <stdio.h>

int main()
{
	// // print non-state names
	// for (const auto& name : get_non_state_names())
	// {
	// 	printf("%s\n", name.c_str());
	// }

	// std::exit(0);
	// auto ch = get_contraction_hierarchy("HI");

	// WallTimer timer;

	// timer.start("Initializing highway");

	// // Highway h("HI", ch, 1, 1, 2.0);
	// Highway h("HI", ch, std::lround(std::log2(ch.node_count())), 1, 1.5);

	// h.initialize();

	// timer.print();

	// timer.start("Computing best clustering exponent");

	// unsigned batch_size = 600;

	// // double total_path_length = h.get_total_greedy_path_length(batch_size);
	// // double average_path_length = total_path_length / batch_size;

	// // double average_path_length = h.get_average_greedy_path_length(batch_size);
	// double clustering_exponent = h.estimate_optimal_clustering_exponent(1.5, batch_size);

	// timer.print();

	// // printf("Average greedy path length: %f\n", average_path_length);
	// printf("Optimal clustering exponent: %f\n", clustering_exponent);



	// Lattice g(150, 3, true);
	// Lattice g(150, 3, true);
	Graph g = get_graph("HI");
	// // // Graph g("CA");

	// // printf("Num nodes: %u\n", g.size());
	// // printf("Num edges: %u\n", g.num_edges());

	double alpha = g.estimate_optimal_dimension(1.5, 0);
	printf("Optimal alpha: %f\n", alpha);
}