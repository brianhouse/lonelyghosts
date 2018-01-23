#!/usr/bin/env python3

import queue, time
import networkx as nx
from housepy import config, log
from messenger import Listener, Handler, Sender
from graph import MAX_NEIGHBORS, find_neighborhoods

sender = Sender()
ips = {}                    # ip addresses indexed by node id
graph = nx.Graph()
node_neighborhoods = {}
neighborhood_nodes = {}
check_ins = queue.Queue()


def message_handler(node):

    global node_neighborhoods, neighborhood_nodes

    node_id = node['id']

    # add the node if it's new
    if node_id not in ips:

        log.info("--> adding %s" % node_id)
        ips[node_id] = node['ip']

        # set behaviors
        try:                            
            sender.send("rang%s" % abs(config['neighbor_range']), (node['ip'], 23232))                
            log.info("--> set range of %s to -%s" % (node_id, config['neighbor_range']))
            sender.send("sens%s" % config['bump_amount'], (node['ip'], 23232))                
            log.info("--> set sensitivity amount of %s to %s" % (node_id, config['bump_amount']))
        except Exception as e:
            log.error(log.exc(e))


    # bump all neighbors of a node that has fired
    if node['action'] == "fire":
        try:
            check_ins.put(node_id)
            try:
                neighbor_ids = neighborhood_nodes[node_neighborhoods[node_id]]
            except KeyError:
                pass
            else:
                log.info("FIRE [ID %s] [-> %s]" % (node_id, ",".join(list(neighbor_ids))))
                for neighbor_id in neighbor_ids:
                    if neighbor_id == node_id:
                        continue
                    if neighbor_id in ips:
                        ip = ips[neighbor_id]
                        sender.send("bump", (ip, 23232))
                    # log.debug("%s sending to %s" % (node_id, neighbor_id))
        except Exception as e:
            log.error(log.exc(e))


    # disrupt all neighbors
    if node['action'] == "tilt":
        try:
            try:
                neighbor_ids = neighborhood_nodes[node_neighborhoods[node_id]]
            except KeyError:
                pass
            else:
                log.info("DISRUPT [ID %s] [-> %s]" % (node_id, ",".join(list(neighbor_ids))))
                for neighbor_id in neighbor_ids:
                    if neighbor_id == node_id:
                        continue     
                    if neighbor_id in ips:                               
                        ip = ips[neighbor_id]
                    sender.send("disr", (ip, 23232))
        except Exception as e:
            log.error(log.exc(e))                            


    # receive network graph information
    if node['action'] == "scan":
        try:
            log.info("SCAN [ID %s] [RSSI %d] [%s]" % (node['id'], node['rssi'], node['data']))

            # remove this node from the graph
            if node_id in graph:
                graph.remove_node(node_id)

            # get the closest MAX_NEIGHBORS neighbors broadcasted for this node
            neighbors = [token for token in node['data'].strip().split(';') if len(token.strip())]
            neighbors.sort(key=lambda x: abs(int(x.split(":")[1])))
            neighbors = neighbors[:MAX_NEIGHBORS]

            # add the adjacency list to the graph, and recompute neighborhoods
            adjlist = list(zip(node_id * len(neighbors), neighbors))
            graph.add_edges_from(adjlist)
            node_neighborhoods, neighborhood_nodes = find_neighborhoods(graph)

        except Exception as e:
            log.error(log.exc(e))


Listener(message_handler=message_handler)


while True:
    time.sleep(1.2)
    present = []
    while True:
        try:
            node_id = check_ins.get_nowait()
            present.append(node_id)
        except queue.Empty:
            break            
    log.info("PRESENT: %s" % len(set(present)))

    # nx.write_gpickle(graph, "graph_state.pkl")
