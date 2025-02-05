#include "graph.hpp"
#include "data.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <queue>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_min.h>

#include <routingkit/contraction_hierarchy.h>

#include <stdio.h>

Graph::Graph(unsigned num_nodes, bool directed) :
	_neighbors(num_nodes), _directed(directed), _num_edges(0)
{
}

void Graph::add_edge(unsigned u, unsigned v, unsigned weight)
{
	++_num_edges;
	_neighbors[u][v] = weight;

	if (!_directed)
	{
		_neighbors[v][u] = weight;
	}
}

const std::unordered_map<unsigned, unsigned>& Graph::get_neighbors(unsigned u) const
{
	return _neighbors[u];
}

unsigned Graph::size() const noexcept
{
	return _neighbors.size();
}

unsigned Graph::num_edges() const noexcept
{
	return _num_edges;
}

RoutingKit::ContractionHierarchy Graph::get_contraction_hierarchy() const
{
	std::vector<unsigned> tail, head, dist;

	for (unsigned u = 0; u < _neighbors.size(); ++u)
	{
		for (const auto& [v, weight] : _neighbors[u])
		{
			head.push_back(u);
			tail.push_back(v);
			dist.push_back(weight);
		}
	}

	return RoutingKit::ContractionHierarchy::build(_neighbors.size(), tail, head, dist);
}

std::vector<Ball> Graph::get_balls(unsigned u) const
{
	std::vector<bool> visited(_neighbors.size(), false);

	std::vector<Ball> balls;
	std::priority_queue<std::pair<unsigned, unsigned>> pq;
	pq.push({0, u});

	unsigned visited_count = 0;
	while (!pq.empty())
	{
		auto [distance, node] = pq.top();
		distance = -distance;
		pq.pop();

		if (visited[node])
		{
			continue;
		}

		visited[node] = true;
		if (node != u)
		{
			++visited_count;
			balls.push_back({distance, visited_count});
		}

		for (const auto& [neighbor, weight] : _neighbors[node])
		{
			if (!visited[neighbor])
			{
				pq.push({-distance - weight, neighbor});
			}
		}
	}

	return balls;
}

unsigned Graph::connected_component_size(unsigned u) const
{
	std::vector<bool> visited(_neighbors.size(), false);

	std::queue<unsigned> q;
	q.push(u);

	unsigned visited_count = 0;
	while (!q.empty())
	{
		unsigned node = q.front();
		q.pop();

		if (visited[node])
			continue;

		visited[node] = true;
		++visited_count;

		for (const auto& [neighbor, weight] : _neighbors[node])
		{
			if (!visited[neighbor])
			{
				q.push(neighbor);
			}
		}
	}

	return visited_count;
}

// 10000.0 is the min distance
double Graph::tight_c(const std::vector<Ball>& balls, double alpha, unsigned num_to_skip, unsigned min_distance)
{
	double min = std::numeric_limits<double>::max();
	double max = std::numeric_limits<double>::min();
	for (const auto& ball : balls)
	{
		if (ball.count <= num_to_skip)
			continue;

		if (ball.distance < min_distance)
			continue;

		double value = ball.count / std::pow(ball.distance, alpha);
		min = std::min(min, value);
		max = std::max(max, value);
	}

	return max / min;
}

double Graph::minimize_tight_c(const std::vector<Ball>& balls, double guess, unsigned num_to_skip, unsigned min_distance, double tolerance)
{
	struct Params
	{
		const std::vector<Ball>* balls;
		unsigned num_to_skip;
		unsigned min_distance;
	};

	auto tight_c_wrapper = [](double alpha, void* params) -> double {
		Params* p = static_cast<Params*>(params);
		return Graph::tight_c(*(p->balls), alpha, p->num_to_skip, p->min_distance);
	};

	Params params_struct = { &balls, num_to_skip, min_distance };

	gsl_function F;
	F.function = tight_c_wrapper;
	F.params = &params_struct;

	const gsl_min_fminimizer_type* T = gsl_min_fminimizer_brent;
	gsl_min_fminimizer* s = gsl_min_fminimizer_alloc(T);

	double lower_bound = 0.001;
	double upper_bound = 5.0;
	double alpha_min = guess;

	// the following avoids the case where the minimum is at the boundary
	double l = F.function(lower_bound, &params_struct);
	double u = F.function(upper_bound, &params_struct);
	double m = F.function(alpha_min, &params_struct);

	if (m > l || m > u)
	{
		printf("l: %f, u: %f, m: %f\n", l, u, m);
		return -1.0;
	}
	// end of the fix

	gsl_min_fminimizer_set(s, &F, alpha_min, lower_bound, upper_bound);

	int status;
	unsigned iter = 0;
	do
	{
		++iter;
		status = gsl_min_fminimizer_iterate(s);
		alpha_min = gsl_min_fminimizer_x_minimum(s);
		lower_bound = gsl_min_fminimizer_x_lower(s);
		upper_bound = gsl_min_fminimizer_x_upper(s);

		status = gsl_min_test_interval(lower_bound, upper_bound, tolerance, 0.0);
	} while (status == GSL_CONTINUE && iter < 100);

	gsl_min_fminimizer_free(s);

	return alpha_min;
}

double Graph::estimate_optimal_dimension(double guess, unsigned num_to_skip, unsigned min_distance, double tolerance) const
{
	double current_alpha = guess;
	unsigned iteration = 0;
	unsigned iterations_since_last_change = 0;
	double min_in_range = std::numeric_limits<double>::max();
	double max_in_range = std::numeric_limits<double>::min();
	std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<unsigned> dist(0, size() - 1);
	std::vector<double> alpha_values;

	do
	{
		unsigned random_node = dist(rng);
		auto balls = get_balls(random_node);

		double alpha = minimize_tight_c(balls, current_alpha, num_to_skip, min_distance, tolerance);
		if (alpha < 0.0)
		{
			continue;
		}
		
		alpha_values.push_back(alpha);

		std::vector<double> sorted_alpha_values = alpha_values;
		std::sort(sorted_alpha_values.begin(), sorted_alpha_values.end());
		size_t n = sorted_alpha_values.size();
		if (n % 2 == 0)
		{
			current_alpha = (sorted_alpha_values[n / 2 - 1] + sorted_alpha_values[n / 2]) / 2.0;
		}
		else
		{
			current_alpha = sorted_alpha_values[n / 2];
		}

		++iteration;
		printf("Iteration: %d, Alpha: %f\n", iteration, current_alpha);

		++iterations_since_last_change;
		min_in_range = std::min(min_in_range, current_alpha);
		max_in_range = std::max(max_in_range, current_alpha);

		if (max_in_range - min_in_range > tolerance)
		{
			iterations_since_last_change = 0;
			min_in_range = max_in_range = current_alpha;
		}
	} while (iterations_since_last_change < 100u);

	return current_alpha;
}
