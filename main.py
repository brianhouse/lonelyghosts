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
                data = {'id': str(data[0]), 'rssi': int(data[1]), 'bat': int(data[2]), 'ip': ip, 'action': data[3], 'neighbors': [token for token in data[4].strip().split(';') if len(token.strip())]}
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
        self.socket.settimeout(1)
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
    nodes = {}
    def message_handler(node):
        log.info("[ID %s] [IP %s] [RSSI %d] [BAT %d] [%s] %s" % (node['id'], node['ip'], node['rssi'], node['bat'], node['action'], node['neighbors']))
        nodes[node['id']] = node['ip']
        if node['action'] == "fire":
            try:
                for node_id in [node.split('-')[0] for node in node['neighbors']]:
                    if node_id in nodes:
                        ip = nodes[node_id]
                        sender.send("bump", (ip, 23232))
            except Exception as e:
                log.error(log.exc(e))
    Listener(message_handler=message_handler)
    while True:
        time.sleep(1)
