import csv
from functools import cache
import math
from pathlib import Path
from typing import Any, Callable, NamedTuple
import matplotlib.pyplot as plt
import numpy as np
plt.rcParams['text.usetex'] = True
plt.rcParams["figure.autolayout"] = True

from state_data import NUM_NODES_PER_STATE

DATA_DIRECTORY = Path('openlab-data')

FIGURE_DIRECTORY = Path('figures')
FIGURE_DIRECTORY.mkdir(exist_ok=True)

DIMENSIONS_FILE = DATA_DIRECTORY / 'dimensions.csv'
CLUSTERING_EXPONENTS_FILE = DATA_DIRECTORY / 'clustering-exponents.csv'
GREEDY_ROUTING_FILE = DATA_DIRECTORY / 'optimal-vs-dimension.csv'

DPI = 600
# DPI = 300
OKABE_COLORS = ['#000000', '#E69F00', '#56B4E9', '#009E73', '#F0E442', '#0072B2', '#D55E00', '#CC79A7']
plt.rcParams['axes.prop_cycle'] = plt.cycler(color = OKABE_COLORS)

POINT_SIZE = 16

def plot(figure_name: str, save: bool, ylabel: str, xlabel: str, legend_loc: str | None = 'best'):
	# plt.title(title)
	plt.xlabel(xlabel, fontsize='x-large')
	plt.ylabel(ylabel, fontsize='x-large')
	# plt.axis('square')

	if legend_loc:
		# size = 'xx-large' if legend_loc == 'best' else 'x-large'
		plt.legend(loc = legend_loc, fontsize='xx-large')

	if save:
		path = (FIGURE_DIRECTORY / figure_name).with_suffix('.png')
		plt.savefig(path)
	else:
		plt.show()

class FitLine(NamedTuple):
	m: float
	b: float
	x: list[int | float]
	y: list[float]
	r2: float

def binary_search(x: list[int], key: int) -> int:
	low = 0
	high = len(x)

	while low < high:
		mid = (high + low) // 2

		if x[mid] < key:
			low = mid + 1
		else:
			high = mid

	return low

def fit_y_eq_x(x: list[float], y: list[float]) -> FitLine:
	fit = np.poly1d((1, 0))
	
	expected_y = fit(x)
	average_y = np.sum(y) / len(y)
	r2 = np.sum((expected_y - average_y) ** 2) / np.sum((y - average_y) ** 2)

	min_x_index = np.argmin(x)
	max_x_index = np.argmax(x)

	x_endpoints = [x[min_x_index], x[max_x_index]]
	expected_y_endpoints = [expected_y[min_x_index], expected_y[max_x_index]]

	return FitLine(m = 1, b = 0, x = x_endpoints, y = expected_y_endpoints, r2 = r2)

def fit_log_line(x: list[int], y: list[float], skip_until: int = 0, all_points: bool = False, add_next: list[int] = [], force_eq: tuple[int, int] | None = None) -> FitLine:
	if skip_until > 0:
		skip = binary_search(x, skip_until)
	else:
		skip = 0
	
	logx, logy = np.log2(x[skip:]), np.log2(y[skip:])
	m, b = force_eq if force_eq else np.polyfit(logx, logy, 1)

	# XX = np.vstack((logx, np.ones_like(logx))).T
	# p_no_offset = np.linalg.lstsq(XX[:, :-1], logy)[0]
	# m, b = p_no_offset[0], 0.0

	fit = np.poly1d((m, b))

	for next_x in add_next:
		next_logx = np.log2(next_x)
		x.append(next_x)
		logx = np.append(logx, next_logx)


	expected_y = fit(logx)
	average_y = np.sum(logy) / len(logy)
	r2 = np.sum((expected_y[:len(expected_y) - len(add_next)] - average_y) ** 2) / np.sum((logy - average_y) ** 2)

	return FitLine(m = m, b = b,
		x = x[skip::] if all_points else x[skip::len(x) - skip - 1],
		y = 2 ** (expected_y if all_points else expected_y[::len(expected_y) - 1]),
		r2 = r2)

def fit_line(x: list[int], y: list[float], skip_until: int = 0, all_points: bool = False, add_next: list[int] = [], force_eq: tuple[int, int] | None = None) -> FitLine:
	if skip_until > 0:
		skip = binary_search(x, skip_until)
	else:
		skip = 0
	
	x_slice, y_slice = x[skip:], y[skip:]
	m, b = force_eq if force_eq else np.polyfit(x_slice, y_slice, 1)
	fit = np.poly1d((m, b))
	
	for next_x in add_next:
		x.append(next_x)
		x_slice = np.append(x_slice, next_x)
	
	expected_y = fit(x_slice)
	average_y = np.sum(y_slice) / len(y_slice)
	r2 = np.sum((expected_y[:len(expected_y) - len(add_next)] - average_y) ** 2) / np.sum((y_slice - average_y) ** 2)
	
	return FitLine(m = m, b = b,
		x = x[skip::] if all_points else x[skip::len(x) - skip - 1],
		y = expected_y if all_points else expected_y[::len(expected_y) - 1],
		r2 = r2)

def load_dimensions(num_to_skip: int = 0, min_distance: int = 0) -> dict[str, float]:
	dimensions = dict[str, float]()

	with DIMENSIONS_FILE.open() as file:
		for row in csv.reader(file):
			if int(row[1]) == num_to_skip and int(row[2]) == min_distance:
				dimensions[row[0]] = float(row[3])

	return dimensions

def rounded_log2(x: int) -> int:
	return round(math.log2(x))

def load_clustering_exponents(k: Callable[[int], int] = rounded_log2, Q: int = 1) -> dict[str, float]:
	clustering_exponents = dict[str, float]()

	with CLUSTERING_EXPONENTS_FILE.open() as file:
		for row in csv.reader(file):
			if not row[0] in NUM_NODES_PER_STATE:
				continue

			if int(row[1]) == k(NUM_NODES_PER_STATE[row[0]]) and int(row[2]) == Q:
				clustering_exponents[row[0]] = float(row[3])

	return clustering_exponents

def load_greedy_routing() -> dict[str, tuple[float, float]]:
	greedy_routing = dict[str, tuple[float, float]]()

	with GREEDY_ROUTING_FILE.open() as file:
		for row in csv.reader(file):
			greedy_routing[row[0]] = (float(row[3]), float(row[4]))

	return greedy_routing


class StateData(NamedTuple):
	name: str
	num_nodes: int
	dimension: float
	clustering_exponent: float
	greedy_routing_dimension: float
	greedy_routing_2: float

@cache
def load_state_data(num_to_skip: int = 0, min_distance: int = 0, k: Callable[[int], int] = rounded_log2, Q: int = 1) -> list[StateData]:
	state_data = list[StateData]()

	dimensions = load_dimensions(num_to_skip, min_distance)
	clustering_exponents = load_clustering_exponents(k, Q)
	greedy_routing = load_greedy_routing()

	for state, num_nodes in NUM_NODES_PER_STATE.items():
		if not state in dimensions or not state in clustering_exponents or not state in greedy_routing:
			continue

		if len(state) != 2:
			# not really a state
			continue

		greedy_routing_dimension, greedy_routing_2 = greedy_routing[state]

		state_data.append(StateData(name = state, num_nodes = num_nodes,
			dimension = dimensions[state], clustering_exponent = clustering_exponents[state],
			greedy_routing_dimension = greedy_routing_dimension, greedy_routing_2 = greedy_routing_2))
		
	return state_data

def compare_states(savefig: bool, exact_fit: bool = True):
	plt.rcParams['axes.prop_cycle'] = plt.cycler(color=OKABE_COLORS[1:])
	plt.figure(num = 0 if exact_fit else 2, figsize = (8 if exact_fit else 5, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	ax = plt.gca()

	annotations = list[tuple[float, float, str, Any]]()
	states_to_annotate = {'CA', 'AK', 'DC', 'HI', 'ME', 'WY', 'NV', 'ID', 'NJ', 'IN', 'MT', 'VA', 'DE', 'GA'}
	states_to_annotate_below = {'WY', 'HI', 'GA', 'IN'}

	# line of best fit
	x = [state.dimension for state in load_state_data()]
	y = [state.clustering_exponent for state in load_state_data()]
	x, y = zip(*sorted(zip(x, y)))

	if exact_fit:
		fit = fit_y_eq_x(x, y)
		plt.plot(fit.x, fit.y, label = f'$s = \\alpha, R^2 = {fit.r2:.2f}$', color=OKABE_COLORS[0], linestyle='--', alpha=0.5, linewidth=POINT_SIZE/4, zorder=1)
	else:
		fit = fit_line(x, y)
		plt.plot(fit.x, fit.y, label = f'$s = {fit.m:.2f} \\alpha {"+" if fit.b > 0 else ""} {fit.b:.2f}, R^2 = {fit.r2:.2f}$', color=OKABE_COLORS[0], linestyle='--', alpha=0.5, linewidth=POINT_SIZE/4, zorder=1)

	for state in load_state_data():
		marker = 's' if state.name in states_to_annotate else 'o'

		p = plt.scatter(state.dimension, state.clustering_exponent, s = POINT_SIZE, marker = marker, zorder=2)
		color = p.get_facecolor()

		if state.name in states_to_annotate:
			annotations.append((state.dimension, state.clustering_exponent, state.name, color))

	# label some states
	for x, y, label, color in annotations:
		yoffset = -20 if label in states_to_annotate_below else 2
		ax.annotate(label, (x, y), textcoords = 'offset points', xytext = (0, yoffset), color = color, ha='center', va='bottom', fontsize='xx-large')

	plot_name = 'clustering_exponent_vs_dimensionality_exact' if exact_fit else 'clustering_exponent_vs_dimensionality'

	plot(plot_name, savefig, 'Optimal Clustering Exponent $s$', 'Approximate Dimensionality $\\alpha$')

def compare_states_size(savefig: bool):
	plt.rcParams['axes.prop_cycle'] = plt.cycler(color=OKABE_COLORS[1:])
	plt.figure(num = 1, figsize = (5, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	ax = plt.gca()

	annotations = list[tuple[float, float, str, Any]]()
	states_to_annotate = {'CA', 'AK', 'DC', 'HI', 'ME', 'ID', 'NJ', 'MT', 'VA', 'DE', 'TX', 'RI', 'FL', 'NH'}
	states_to_annotate_below = {'MT', 'HI', 'GA', 'IN	', 'ME', 'TX', 'FL'}

	x = [state.num_nodes for state in load_state_data()]
	y = [state.clustering_exponent for state in load_state_data()]
	x, y = zip(*sorted(zip(x, y)))

	x = np.log2(x)
	fit = fit_line(x, y)

	plt.plot(2 ** fit.x, fit.y, label = f'$s = {fit.m:.2f} \\log n {"+" if fit.b > 0 else ""} {fit.b:.2f}, R^2 = {fit.r2:.2f}$', color=OKABE_COLORS[0], linestyle='--', alpha=0.5, linewidth=POINT_SIZE/4, zorder=1)

	for state in load_state_data():
		marker = 's' if state.name in states_to_annotate else 'o'

		p = plt.scatter(state.num_nodes, state.clustering_exponent, s = POINT_SIZE, marker = marker, zorder=2)
		color = p.get_facecolor()

		if state.name in states_to_annotate:
			annotations.append((state.num_nodes, state.clustering_exponent, state.name, color))

	for x, y, label, color in annotations:
		yoffset = -20 if label in states_to_annotate_below else 2
		ax.annotate(label, (x, y), textcoords = 'offset points', xytext = (0, yoffset), color = color, ha='center', va='bottom', fontsize='xx-large')

	ax.set_xscale('log')
	plot('clustering_exponent_vs_size', savefig, 'Optimal Clustering Exponent $s$', 'Number of Nodes $n$')

def compare_greedy_routing(savefig: bool):
	# plt.rcParams['axes.prop_cycle'] = plt.cycler(color=OKABE_COLORS[2:])
	
	plt.figure(num = 4, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	ax = plt.gca()

	annotations = list[tuple[float, float, str, Any]]()
	states_to_annotate = {'AK', 'DC', 'HI', 'CA', 'MT', 'DE'}
	states_to_annotate_below = {'AK'}

	# x = [state.num_nodes for state in load_state_data()]
	# y = [state.clustering_exponent for state in load_state_data()]
	# x, y = zip(*sorted(zip(x, y)))

	# x = np.log2(x)
	# fit = fit_line(x, y)

	# plt.plot(2 ** fit.x, fit.y, label = f'$s = {fit.m:.2f} \\log n {"+" if fit.b > 0 else ""} {fit.b:.2f}, R^2 = {fit.r2:.2f}$', color=OKABE_COLORS[0], linestyle='--', alpha=0.5, linewidth=POINT_SIZE/4, zorder=1)

	for state in load_state_data():
		plt.scatter(state.num_nodes, state.greedy_routing_dimension, s = POINT_SIZE, marker = 's', zorder=2, color=OKABE_COLORS[1], label='$s = \\alpha$' if state.name == 'TX' else '')

		plt.scatter(state.num_nodes, state.greedy_routing_2, s = POINT_SIZE, marker = 'o', zorder=2, color=OKABE_COLORS[2], label='$s = 2$' if state.name == 'TX' else '')

		# also plot the difference of 2 - dimension
		plt.scatter(state.num_nodes, state.greedy_routing_2 - state.greedy_routing_dimension, s = POINT_SIZE, marker = '*', zorder=2, color=OKABE_COLORS[3], label='difference' if state.name == 'TX' else '')

		if state.name in states_to_annotate:
			annotations.append((state.num_nodes, state.greedy_routing_2, state.name, OKABE_COLORS[2]))

	for x, y, label, color in annotations:
		yoffset = -20 if label in states_to_annotate_below else 2
		ax.annotate(label, (x, y), textcoords = 'offset points', xytext = (0, yoffset), color = color, ha='center', va='bottom', fontsize='xx-large')

	ax.set_xscale('log')
	plot('greedy_routing_comp', savefig, 'Average Greedy Routing Hops', 'Number of Nodes $n$')

if __name__ == '__main__':
	savefig = False
	savefig = True

	compare_states(savefig)
	compare_states(savefig, False)
	compare_states_size(savefig)
	compare_greedy_routing(savefig)

	# print(load_state_data())

	# state_data = load_state_data()
	# for state in state_data:
	# 	print(f'{{"{state.name}", {state.dimension} }},')
