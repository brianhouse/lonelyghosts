#!/usr/bin/env python3

import time
import networkx as nx
import metis
import matplotlib.pyplot as plt
from timeit import default_timer as timer
from colors import colors

MAX_SUBGRAPH_SIZE = 3

start = timer()

# adjlist = (1, 2), (2, 3), (3, 1), (4, 3), (5, 5), (6, 7), (6, 8), (8, 7), (7, 9), (9, 10)
adjlist = (11211, 11233), (11211, 11283), (11283, 12909), (12909, 11211), (12977, 12909), (22222, 22222), (80220, 90210), (80220, 66666), (66666, 90210), (90210, 434343), (434343, 67879)


# create graph and identify subgraphs
G = nx.Graph()
G.add_edges_from(adjlist)
G = G.to_undirected()
subgraphs = list(nx.connected_component_subgraphs(G))

# split subgraphs with size greater than MAX_SUBGRAPH_SIZE
remove_indexes = []
for s in range(len(subgraphs)):
    subgraph = subgraphs[s]
    if len(subgraph) > MAX_SUBGRAPH_SIZE:
        k = (len(subgraph) // MAX_SUBGRAPH_SIZE) + 1
        cuts, parts = metis.part_graph(subgraph, k)
        for i in range(k):
            subgraphs.append(subgraph.subgraph([node for (n, node) in enumerate(subgraph.node) if parts[n] == i]))
        remove_indexes.append(s)
subgraphs = [subgraph for (s, subgraph) in enumerate(subgraphs) if s not in remove_indexes]

# create color map
color_map = []
for i, node in enumerate(G):
    for j, subgraph in enumerate(subgraphs):
        if node in subgraph:
            color_map.append(colors[j])
            break

print("TIME ELAPSED: %.2fms" % ((timer() - start) * 1000))

nx.draw(G, node_color=color_map)
plt.show()




"""

ok, so if you cant directly parse by size:

- figure out unconnected all subgraphs
- if any are greater than size N, divide into parts size//N + 1.

"""