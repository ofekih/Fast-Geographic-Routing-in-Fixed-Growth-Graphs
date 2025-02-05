#pragma once

#include "graph.hpp"

class Lattice : public Graph
{
public:
	Lattice(unsigned side_length, unsigned dimension, bool wrap_around = false);

	unsigned side_length() const noexcept;

	unsigned dimension() const noexcept;

private:
	void index_to_coords(unsigned index, std::vector<unsigned>& coords) const noexcept;

	unsigned coords_to_index(const std::vector<unsigned>& coords) const noexcept;

	unsigned _side_length;
	unsigned _dimension;
};