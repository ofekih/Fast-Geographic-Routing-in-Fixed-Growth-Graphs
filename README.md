# Fast Geographic Routing in Fixed-Growth Graphs

This code is supplementary material for the paper "Fast Geographic Routing in Fixed-Growth Graphs" by Gila, Goodrich, Illickan, and Sridhar.

## Running the code

First, fetch the code from the RoutingKit submodule:

```bash
git submodule update --init
```

Then, compile its code:

```bash
cd RoutingKit
make
cd ..
```

Finally, compile the code in this directory:

```bash
make
```

This will give you several executables, including `bin/find_best_clustering_coefficient`, `bin/find_matching_dimensions`, and `bin/run_optimal_vs_dimension`.
