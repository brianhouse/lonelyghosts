#!/usr/bin/env python3

import time
import networkx as nx
import metis
from timeit import default_timer as timer
from colors import colors
from housepy import config

MAX_NEIGHBORS = config['max_neighbors']


def find_neighborhoods(G):

    # identify subgraphs
    subgraphs = list(nx.connected_component_subgraphs(G))

    # split subgraphs with size greater than MAX_NEIGHBORS
    remove_indexes = []
    for s in range(len(subgraphs)):
        subgraph = subgraphs[s]
        if len(subgraph) > (MAX_NEIGHBORS + 1):
            k = (len(subgraph) // MAX_NEIGHBORS) + 1
            cuts, parts = metis.part_graph(subgraph, k)
            for i in range(k):
                subgraphs.append(subgraph.subgraph([node for (n, node) in enumerate(subgraph.node) if parts[n] == i]))
            remove_indexes.append(s)
    subgraphs = [subgraph for (s, subgraph) in enumerate(subgraphs) if s not in remove_indexes]

    # create an index
    neighborhoods = []
    for i, node in enumerate(G):
        for j, subgraph in enumerate(subgraphs):
            if node in subgraph:
                neighborhoods.append(j)
                break    

    node_neighborhoods = dict(zip(G, neighborhoods))
    neighborhood_nodes = {}
    for node, neighborhood in node_neighborhoods.items():
        neighborhood_nodes[neighborhood] = neighborhood_nodes.get(neighborhood, [])
        neighborhood_nodes[neighborhood].append(node)

    return node_neighborhoods, neighborhood_nodes


if __name__ == "__main__":

    import matplotlib.pyplot as plt

    # MAX_NEIGHBORS = 3

    # adjlist = ('11211', '11233'), ('11211', '11283'), ('11283', '12909'), ('12909', '11211'), ('12977', '12909'), ('22222', '22222'), ('80220', '90210'), ('80220', '66666'), ('66666', '90210'), ('90210', '434343'), ('434343', '67879')
    # G = nx.Graph()
    # G.add_edges_from(adjlist)

    G = nx.read_gpickle("graph_state.pkl")

    start = timer()
    node_neighborhoods, neighborhood_nodes = find_neighborhoods(G)
    print(node_neighborhoods)
    print(neighborhood_nodes)
    print("TIME ELAPSED: %.2fms" % ((timer() - start) * 1000))

    color_map = [colors[n] for n in node_neighborhoods.values()]
    nx.draw(G, node_color=color_map)
    plt.show()

