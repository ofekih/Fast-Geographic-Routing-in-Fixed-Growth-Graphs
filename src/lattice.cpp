#include "lattice.hpp"

#include <cmath>
#include <fstream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

Lattice::Lattice(unsigned side_length, unsigned dimension, bool wrap_around) :
	Graph(static_cast<unsigned>(std::pow(side_length, dimension)), true),
	_side_length(side_length), _dimension(dimension)
{
	std::vector<unsigned> coords(dimension, 0);
	std::vector<unsigned> new_coords(dimension, 0);

	for (unsigned index = 0; index < size(); ++index)
	{
		index_to_coords(index, coords);
		for (unsigned d = 0; d < dimension; ++d)
		{
			for (int sign : {-1, 1})
			{
				if (!wrap_around && ((coords[d] == 0 && sign == -1) || (coords[d] == side_length - 1 && sign == 1)))
				{
					continue;
				}

				new_coords = coords;
				new_coords[d] = (new_coords[d] + sign + side_length) % side_length;
				add_edge(index, coords_to_index(new_coords), 1.0);
			}
		}
	}
}

void Lattice::index_to_coords(unsigned index, std::vector<unsigned>& coords) const noexcept
{
	for (unsigned d = 0; d < _dimension; ++d)
	{
		coords[d] = index % _side_length;
		index /= _side_length;
	}
}

unsigned Lattice::coords_to_index(const std::vector<unsigned>& coords) const noexcept
{
	unsigned index = 0;
	for (unsigned d = _dimension - 1; d < _dimension; --d)
	{
		index = index * _side_length + coords[d];
	}
	return index;
}

unsigned Lattice::side_length() const noexcept
{
	return _side_length;
}

unsigned Lattice::dimension() const noexcept
{
	return _dimension;
}