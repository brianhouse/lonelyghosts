#!/usr/bin/env python3

import serial, threading, time, os, socket, queue
from housepy import config, log, timeutil


class Listener(threading.Thread):

    def __init__(self, port=23232, message_handler=None, blocking=False):
        super(Listener, self).__init__()
        self.daemon = True
        self.messages = queue.Queue()
        Handler(self.messages, message_handler)
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.socket.bind(('', port))
        except Exception as e:
            log.error(log.exc(e))
            return
        self.start()
        if blocking:
            try:
                while True:
                    time.sleep(1)
            except (KeyboardInterrupt, SystemExit):
                self.connection.close()
                pass

    def run(self):
        while True:
            try:
                message, address = self.socket.recvfrom(1024)
                ip, port = address
                data = message.decode('utf-8').split(',')
                data.append("")
                data = {'id': str(data[0]), 'rssi': int(data[1]), 'ip': ip, 'action': data[2], 'data': data[3]}
                self.messages.put(data)
            except Exception as e:
                log.error(log.exc(e))
                log.error(message)


class Handler(threading.Thread):

    def __init__(self, messages, message_handler):
        super(Handler, self).__init__()        
        if message_handler is None:
            return
        self.messages = messages
        self.message_handler = message_handler
        self.daemon = True
        self.start()

    def run(self):
        while True:
            try:
                message = self.messages.get()
                self.message_handler(message)
            except Exception as e:
                log.error(log.exc(e))


class Sender(threading.Thread):

    def __init__(self, blocking=False):
        super(Sender, self).__init__()
        self.daemon = True
        self.messages = queue.Queue()
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.settimeout(0.1)
        self.start()
        if blocking:
            try:
                while True:
                    time.sleep(1)
            except (KeyboardInterrupt, SystemExit):
                self.connection.close()
                pass

    def run(self):
        while True:
            try:
                message, address = self.messages.get()
                # log.info("SENDING [%s] to %s:%s" % (message, address[0], address[1]))
                self.socket.sendto(message.encode('ascii'), address)
            except Exception as e:
                log.error(log.exc(e))

    def send(self, message, address):
        self.messages.put((message, address))


if __name__ == "__main__":

    sender = Sender()
    ips = {}                    # ip addresses indexed by node id
    neighbor_id_sets = {}       # sets of neighbor ids indexed by node id

    def message_handler(node):

        # add the node if it's new
        if node['id'] not in ips:

            log.info("--> adding %s" % node['id'])
            ips[node['id']] = node['ip']
            neighbor_id_sets[node['id']] = set()    

            # set range
            try:                            
                sender.send("range%s" % abs(config['neighbor_range']), (node['ip'], 23232))                
                log.info("--> set range of %s to -%s" % (node['id'], config['neighbor_range']))
            except Exception as e:
                log.error(log.exc(e))

        if node['action'] == "scan":
            try:
                log.info("SCAN [ID %s] [RSSI %d] [%s]" % (node['id'], node['rssi'], node['data']))

                # set of the neighbors broadcasted for this node
                current_set = set([token.split(':')[0] for token in node['data'].strip().split(';') if len(token.strip())])

                # set the neighbors of this node to this list
                neighbor_id_sets[node['id']] = current_set

                # now go through every set of neighbor_ids -- add this node if they are in the current list, otherwise discard
                for neighbor_id, neighbor_id_set in neighbor_id_sets.items():
                    if neighbor_id == node['id']:
                        continue
                    elif neighbor_id in current_set:
                        neighbor_id_set.add(node['id'])
                    else:
                        neighbor_id_set.discard(node['id'])                        
            except Exception as e:
                log.error(log.exc(e))

        elif node['action'] == "fire":
            try:
                # bump all neighbors
                neighbor_ids = neighbor_id_sets[node['id']]
                log.info("FIRE [ID %s] [-> %s]" % (node['id'], ",".join(list(neighbor_ids))))
                for neighbor_id in neighbor_ids:
                    ip = ips[neighbor_id]
                    sender.send("bump", (ip, 23232))
                    # log.debug("%s sending to %s" % (node['id'], neighbor_id))
            except Exception as e:
                log.error(log.exc(e))

    Listener(message_handler=message_handler)
    while True:
        time.sleep(1)
