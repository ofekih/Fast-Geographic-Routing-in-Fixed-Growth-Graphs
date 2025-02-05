#include "data.hpp"

#include <fstream>
#include <sstream>
#include <string>

void save_clustering_exponent_data(const std::string& name, unsigned k, unsigned Q, double clustering_exponent, double average_greedy_path_length)
{
	std::ofstream file(DATA_DIRECTORY + CLUSTERING_EXPONENT_DATA_FILENAME, std::ios::app);
	file << name << "," << k << "," << Q << "," << clustering_exponent << "," << average_greedy_path_length << std::endl;
}

void save_optimal_clustering_exponent_data(const std::string& name, unsigned k, unsigned Q, double optimal_clustering_exponent)
{
	std::ofstream file(DATA_DIRECTORY + OPTIMAL_CLUSTERING_EXPONENT_DATA_FILENAME, std::ios::app);
	file << name << "," << k << "," << Q << "," << optimal_clustering_exponent << std::endl;
}

void save_dimension_data(const std::string& name, unsigned num_to_skip, unsigned min_distance, double dimension)
{
	std::ofstream file(DATA_DIRECTORY + DIMENSION_DATA_FILENAME, std::ios::app);
	file << name << "," << num_to_skip << "," << min_distance << "," << dimension << std::endl;
}

void save_optimal_vs_dimension_data(const std::string& name, double optimal_dimension, double dimension, double result_with_optimal, double result_with_dimension)
{
	std::ofstream file(DATA_DIRECTORY + OPTIMAL_VS_DIMENSION_DATA_FILENAME, std::ios::app);
	file << name << "," << optimal_dimension << "," << dimension << "," << result_with_optimal << "," << result_with_dimension << std::endl;
}

std::unordered_set<std::string> has_optimal_clustering_exponent_data(unsigned k, unsigned Q)
{
	std::unordered_set<std::string> states;

	std::ifstream file(DATA_DIRECTORY + OPTIMAL_CLUSTERING_EXPONENT_DATA_FILENAME);

	std::string line;
	while (std::getline(file, line))
	{
		std::string state;
		unsigned k_, Q_;

		std::istringstream iss(line);
		std::string k_str, Q_str;

		std::getline(iss, state, ',');
		std::getline(iss, k_str, ',');
		std::getline(iss, Q_str, ',');

		k_ = std::stoul(k_str);
		Q_ = std::stoul(Q_str);

		if (k_ == k && Q_ == Q)
		{
			states.insert(state);
		}
	}

	return states;
}

std::unordered_set<std::string> has_dimension_data(unsigned num_to_skip, unsigned min_distance)
{
	std::unordered_set<std::string> states;

	std::ifstream file(DATA_DIRECTORY + DIMENSION_DATA_FILENAME);

	std::string line;
	while (std::getline(file, line))
	{
		std::string state;
		unsigned num_skip_, min_distance_;

		std::istringstream iss(line);
		std::string num_skip_str, min_distance_str;

		std::getline(iss, state, ',');
		std::getline(iss, num_skip_str, ',');
		std::getline(iss, min_distance_str, ',');

		num_skip_ = std::stoul(num_skip_str);
		min_distance_ = std::stoul(min_distance_str);

		if (num_skip_ == num_to_skip && min_distance_ == min_distance)
		{
			states.insert(state);
		}
	}

	return states;
}

std::string get_hostname()
{
	std::string hostname;
	std::ifstream file("/proc/sys/kernel/hostname");
	std::getline(file, hostname);
	return hostname;
}

std::string pretty_print(size_t nanoseconds)
{
	size_t milliseconds = nanoseconds / 1e6;
	size_t seconds = nanoseconds / 1e9;
	size_t minutes = seconds / 60;
	size_t hours = minutes / 60;

	if (hours > 0)
	{
		return std::to_string(hours) + "h " + std::to_string(minutes % 60) + "m " + std::to_string(seconds % 60) + "s " + std::to_string(milliseconds % 1000) + "ms";
	}
	else if (minutes > 0)
	{
		return std::to_string(minutes) + "m " + std::to_string(seconds % 60) + "s " + std::to_string(milliseconds % 1000) + "ms";
	}
	else if (seconds > 0)
	{
		return std::to_string(seconds) + "s " + std::to_string(milliseconds % 1000) + "ms";
	}
	else
	{
		return std::to_string(milliseconds) + "ms";
	}
}