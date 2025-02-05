#pragma once

#include <chrono>
#include <ctime>
#include <string>
#include <unordered_set>

static const std::string DATA_DIRECTORY = "data/";
static const std::string CSV_EXTENSION = ".csv";
static const std::string CLUSTERING_EXPONENT_DATA_FILENAME = "clustering-exponent" + CSV_EXTENSION;
static const std::string OPTIMAL_CLUSTERING_EXPONENT_DATA_FILENAME = "optimal-clustering-exponent-acc" + CSV_EXTENSION;
static const std::string DIMENSION_DATA_FILENAME = "dimension" + CSV_EXTENSION;
static const std::string OPTIMAL_VS_DIMENSION_DATA_FILENAME = "optimal-vs-dimension" + CSV_EXTENSION;

void save_clustering_exponent_data(const std::string& name, unsigned k, unsigned Q, double clustering_exponent, double average_greedy_path_length);

void save_optimal_clustering_exponent_data(const std::string& name, unsigned k, unsigned Q, double optimal_clustering_exponent);

void save_dimension_data(const std::string& name, unsigned num_to_skip, unsigned min_distance, double dimension);

void save_optimal_vs_dimension_data(const std::string& name, double optimal_dimension, double dimension, double result_with_optimal, double result_with_dimension);

std::unordered_set<std::string> has_optimal_clustering_exponent_data(unsigned k, unsigned Q);

std::unordered_set<std::string> has_dimension_data(unsigned num_to_skip, unsigned min_distance);

std::string get_hostname();
static const std::string HOSTNAME = get_hostname();

std::string pretty_print(size_t nanoseconds);

struct WallTimer
{
	std::chrono::high_resolution_clock::time_point start_time;

	WallTimer()
	{
	}

	size_t elapsed_nanoseconds() const noexcept
	{
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_time);
	
		return duration.count();
	}

	void start(const std::string& prompt = "", bool reset = true) noexcept
	{
		if (reset)
		{
			start_time = std::chrono::high_resolution_clock::now();
		}

		if (!prompt.empty())
		{
			printf("%s... \t", prompt.c_str());
		}
	}

	size_t print() noexcept
	{
		auto nanoseconds = elapsed_nanoseconds();
		printf("done (%s)\n", pretty_print(nanoseconds).c_str());
		return nanoseconds;
	}
};

struct CPUTimer
{
	std::clock_t start_time;

	CPUTimer()
	{
	}

	size_t elapsed_nanoseconds() const noexcept
	{
		auto end = std::clock();
		return (end - start_time) * 1e9 / CLOCKS_PER_SEC;
	}

	void start(const std::string& prompt = "", bool reset = true) noexcept
	{
		if (reset)
		{
			start_time = std::clock();
		}

		if (!prompt.empty())
		{
			printf("%s... \t", prompt.c_str());
		}
	}

	size_t print() noexcept
	{
		auto nanoseconds = elapsed_nanoseconds();
		printf("done (%s)\n", pretty_print(nanoseconds).c_str());
		fflush(stdout);
		return nanoseconds;
	}
};
