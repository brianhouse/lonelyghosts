#!/usr/bin/env python3

import time
import networkx as nx
import metis
import matplotlib.pyplot as plt

adjlist = (1, 2), (2, 3), (3, 1), (4, 3), (5, 5), (6, 7), (6, 8), (8, 7)

G = nx.Graph()
G.add_edges_from(adjlist)

edgecuts, parts = metis.part_graph(G, 3)
print(parts)


colors = ['red','blue','green', 'yellow']
color_map = []
for i, p in enumerate(parts):
    color_map.append(colors[p])

nx.draw(G, node_color=color_map)
plt.show()




"""

ok, so if you cant directly parse by size:

- figure out unconnected all subgraphs
- if any are greater than size 10, divide into parts N//10 + 1.

"""