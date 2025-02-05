#include "highway.hpp"

#include "data.hpp"

#include <routingkit/contraction_hierarchy.h>

#include <future>
#include <random>
#include <vector>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_min.h>

Highway::Highway(const std::string& name, const RoutingKit::ContractionHierarchy& ch, unsigned k, unsigned Q, double clustering_exponent) :
	_name(name), _contraction_hierarchy(ch), _k(k), _Q(Q), _clustering_exponent(clustering_exponent), _num_nodes(ch.node_count())
{
	_is_highway_node.resize(_num_nodes, false);
}

const std::string& Highway::name() const noexcept
{
	return _name;
}

void Highway::initialize() noexcept
{
	static std::mt19937 rng(std::random_device{}());
	static std::uniform_real_distribution<double> dist(0.0, 1.0);

	_highway_nodes.clear();
	_highway_nodes.reserve(_num_nodes / _k * 2);

	for (unsigned node = 0; node < _num_nodes; ++node)
	{
		if (dist(rng) < (1.0 / _k))
		{
			_highway_nodes.push_back(node);
			_is_highway_node[node] = true;
			continue;
		}

		_is_highway_node[node] = false;
	}
}

void Highway::for_each_long_distance_contact(unsigned u, const std::function<void(unsigned)>& callback) const noexcept
{
	if (!_is_highway_node[u])
	{
		return;
	}

	thread_local std::mt19937 rng(std::random_device{}());
	thread_local RoutingKit::ContractionHierarchyQuery ch_query(_contraction_hierarchy);

	auto distances = ch_query.reset().add_source(u).pin_targets(_highway_nodes).run_to_pinned_targets().get_distances_to_targets();

	std::vector<double> probabilities;

	for (unsigned i = 0; i < _highway_nodes.size(); ++i)
	{
		unsigned node = _highway_nodes[i];
		if (node == u)
		{
			probabilities.push_back(0.0);
			continue;
		}

		unsigned distance = distances[i];
		double probability = std::pow(distance, -_clustering_exponent);
		probabilities.push_back(probability);
	}

	std::discrete_distribution<unsigned> dist(probabilities.begin(), probabilities.end());
	
	for (unsigned i = 0; i < _k * _Q; ++i)
	{
		callback(_highway_nodes[dist(rng)]);
	}
}

unsigned Highway::get_distance(unsigned s, unsigned t) const noexcept
{
	thread_local RoutingKit::ContractionHierarchyQuery ch_query(_contraction_hierarchy);
	return ch_query.reset().add_source(s).add_target(t).run().get_distance();
}

unsigned Highway::get_greedy_path_length(unsigned start, unsigned end) const noexcept
{
	thread_local RoutingKit::ContractionHierarchyQuery ch_query(_contraction_hierarchy);
	
	ch_query.reset_target().add_target(end);

	unsigned path_length = 0;

	while (start != end)
	{
		++path_length;

		unsigned min_distance = std::numeric_limits<unsigned>::max();
		unsigned min_node = 0;

		for_each_long_distance_contact(start, [&](unsigned contact)
		{
			unsigned distance = get_distance(contact, end);
			if (distance < min_distance)
			{
				min_distance = distance;
				min_node = contact;
			}
		});

		unsigned local_contact = ch_query.reset_source().add_source(start).run().get_node_path()[1];
		unsigned local_distance = get_distance(local_contact, end);

		if (min_distance < local_distance)
		{
			// take a long distance contact
			start = min_node;
			continue;
		}

		// take a local contact
		start = local_contact;
	}

	return path_length;
}

double Highway::get_total_greedy_path_length(unsigned num_trials) const noexcept
{
	double total_path_length = 0.0;

	std::vector<std::future<double>> futures;

	for (unsigned i = 0; i < NUM_THREADS; ++i)
	{
		futures.emplace_back(std::async(std::launch::async, [this, num_trials, i]() noexcept
		{
			thread_local std::mt19937 rng(std::random_device{}());
			thread_local std::uniform_int_distribution<unsigned> dist(0, _num_nodes - 1);

			double local_total_path_length = 0.0;

			for (unsigned j = i; j < num_trials; j += NUM_THREADS)
			{
				unsigned start = dist(rng);
				unsigned end = dist(rng);

				local_total_path_length += get_greedy_path_length(start, end);
			}

			return local_total_path_length;
		}));
	}

	for (auto& future : futures)
	{
		total_path_length += future.get();
	}

	return total_path_length;
}

double Highway::get_average_greedy_path_length(unsigned batch_size, double fractional_difference) noexcept
{
	double total_path_length = 0.0;
	unsigned iteration = 0;
	unsigned iterations_since_last_change = 0;
	double min_in_range = std::numeric_limits<double>::max();
	double max_in_range = std::numeric_limits<double>::min();
	double average;

	printf("Testing exponent: %f\n", _clustering_exponent);

	do
	{
		initialize();
		total_path_length += get_total_greedy_path_length(batch_size);
		++iteration;

		printf("Iteration: %u, Average path length: %f\n", iteration, total_path_length / (iteration * batch_size));

		++iterations_since_last_change;
		average = total_path_length / (iteration * batch_size);
		min_in_range = std::min(min_in_range, average);
		max_in_range = std::max(max_in_range, average);

		if ((max_in_range - min_in_range) / min_in_range > fractional_difference)
		{
			iterations_since_last_change = 0;
			min_in_range = max_in_range = average;
		}

	} while (iterations_since_last_change < std::max(10u, iteration / 2));

	double tolerance = (max_in_range - min_in_range) / min_in_range;
	double rounded_average = std::round(average / tolerance) * tolerance;

	save_clustering_exponent_data(_name, _k, _Q, _clustering_exponent, rounded_average);

	return average;
}

double Highway::estimate_optimal_clustering_exponent(double guess, unsigned batch_size, double tolerance) noexcept
{
	struct Params
	{
		const std::string& name;
		const RoutingKit::ContractionHierarchy& contraction_hierarchy;
		unsigned k;
		unsigned Q;
		unsigned batch_size;
	};

	Params params = { _name, _contraction_hierarchy, _k, _Q, batch_size };

	auto get_average_greedy_path_length_wrapper = [](double clustering_exponent, void* params) -> double {
		Params* p = static_cast<Params*>(params);

		Highway h(p->name, p->contraction_hierarchy, p->k, p->Q, clustering_exponent);
		return h.get_average_greedy_path_length(p->batch_size);
	};

	gsl_function F;
	F.function = get_average_greedy_path_length_wrapper;
	F.params = &params;

	const gsl_min_fminimizer_type* T = gsl_min_fminimizer_brent;
	gsl_min_fminimizer* s = gsl_min_fminimizer_alloc(T);

	double lower_bound = 0.01;
	double upper_bound = 2.5;
	double clustering_exponent = guess;

	gsl_min_fminimizer_set(s, &F, clustering_exponent, lower_bound, upper_bound);

	int status;
	unsigned iter = 0;
	do
	{
		++iter;
		status = gsl_min_fminimizer_iterate(s);
		clustering_exponent = gsl_min_fminimizer_x_minimum(s);
		lower_bound = gsl_min_fminimizer_x_lower(s);
		upper_bound = gsl_min_fminimizer_x_upper(s);

		status = gsl_min_test_interval(lower_bound, upper_bound, tolerance, 0.0);
	} while (status == GSL_CONTINUE && iter < 100);

	gsl_min_fminimizer_free(s);

	return clustering_exponent;
}
