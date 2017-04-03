#!/usr/bin/env python3

import serial, threading, time, os, socket, queue
from housepy import config, log, util


class FeatherListener(threading.Thread):

    def __init__(self, port=23232, message_handler=None, blocking=False):
        super(FeatherListener, self).__init__()
        self.daemon = True
        self.messages = queue.Queue()
        FeatherHandler(self.messages, message_handler)
        self.events = {}
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
                data = message.decode('utf-8').split(',')
                data = {'id': str(address[0]), 'rssi': int(data[1]), 'ip': address, 't_utc': util.timestamp(ms=True), 'data': [v for v in data[2:]]}
                self.messages.put(data)
                if data['id'] not in self.events:
                    self.events[data['id']] = queue.Queue()
                self.events[data['id']].put(1)                    
            except Exception as e:
                log.error(log.exc(e))


class FeatherHandler(threading.Thread):

    def __init__(self, messages, message_handler):
        super(FeatherHandler, self).__init__()        
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


class FeatherSender(threading.Thread):

    def __init__(self, blocking=False):
        super(FeatherSender, self).__init__()
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
                log.info("SENDING [%s] to %s:%s" % (message, address[0], address[1]))
                self.socket.sendto(message.encode('ascii'), address)
            except Exception as e:
                log.error(log.exc(e))

    def send(self, message, address):
        self.messages.put((message, address))


if __name__ == "__main__":
    fs = FeatherSender()
    feather_ids = []
    def message_handler(response):
        # log.info("%f [ID %s] [IP %s] [RSSI %d]:\t%s" % (response['t_utc'], response['id'], response['ip'][0], response['rssi'], "".join(["%s " % f for f in response['data']])))
        if response['id'] not in feather_ids:
            feather_ids.append(response['id'])
        if response['data'][0] == "fire":
            log.info("%s fire!" % response['id'])
            for feather_id in feather_ids:
                if feather_id == response['id']:
                    continue
                fs.send("bump", (feather_id, 23232))
            log.info("")
    fl = FeatherListener(message_handler=message_handler)
    while True:
        time.sleep(1)
        # time.sleep(2)
        # fs.send("bump", ("192.168.1.6", 23232))
