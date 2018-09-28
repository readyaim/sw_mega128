# !/usr/bin/env python
'''This is the test excecises for chapter 16 of CORE_PYTHON_PROGRAMMING
Network Programming
'''

import sys
import os
from datetime import datetime
from time import ctime, sleep
import threading
from random import randint
from queue import Queue
import logging
from socket import *


logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s: %(message)s')
handler = logging.FileHandler('trace.log',mode='a')
handler.setLevel(logging.DEBUG)
handler.setFormatter(formatter)
# create a logging format
formatterconsole = logging.Formatter('%(message)s')
handlerconsole = logging.StreamHandler()
handlerconsole.setFormatter(formatterconsole)
handlerconsole.setLevel(logging.DEBUG)
if logger.hasHandlers() is False:
    logger.addHandler(handler)
    logger.addHandler(handlerconsole)
    logger.info("HasHandlers() is False, add two new handlers")
else:
    logger.info("HasHandlers() is True, previous handlers existed, don't add new ones")

# Global variable definition here
debug =True

#HOST = '10.240.17.60'
HOST = '123.206.115.17'
#HOST = '192.168.1.102'
PORT = 21567
ADDR = (HOST, PORT)
BUFSIZ = 1024

def tsTclinet():
    #HOST = 'localhost' #  or 'localhost'
    tcpCliSock = socket(AF_INET,SOCK_STREAM)
    tcpCliSock.connect(ADDR)
    
    while True:
        data = input('> ')
        #print('data=',data);
        if not data:
            break
        tcpCliSock.send(data.encode())
        data = tcpCliSock.recv(BUFSIZ).decode()
        if not data:
            break
        print(data)
    
    tcpCliSock.close()


def tsUclinet():
    udpCliSock = socket(AF_INET,SOCK_DGRAM)
    while True:
        data = input('> ')
        #print('data=',data);
        if not data:
            break
        udpCliSock.sendto(data.encode(), ADDR)
        data, ADDR2 = udpCliSock.recvfrom(BUFSIZ)   #.decode()
        if not data:
            break
        print(data)
    
    udpCliSock.close()

def tsTclientt():
    while True:
        tcpCliSock = socket(AF_INET, SOCK_STREAM)
        tcpCliSock.connect(ADDR)
        data = input('>')
        if not data:
            break
        tcpCliSock.send(('%s\r\n' % data).encode('utf-8'))
        data = tcpCliSock.recv(BUFSIZ)
        if not data:
            break
        print(data.strip().decode('utf-8'))
        tcpCliSock.close()


# Class definition here
class MyThread (threading.Thread):
    def __init__(self, func, name='', *kargs):
        threading.Thread.__init__(self)
        self.name = name
        self.func = func
        self.args = kargs
        logger.debug('kargs=%s',kargs)
    def getResult(self):
        pass
        return self.res
    def run(self):
        logger.debug ('starting %s at %s', self.name, ctime())
        #self.res = apply(self.func, self.args)
        self.res = self.func(*self.args)
        logger.debug("%s %s %s", self.name, 'finished at:', ctime())

from twisted.internet import protocol, reactor
class TSClntProtocol(protocol.Protocol):
    def sendData(self):
        data = input("> ")
        if data:
            logger.info( '...sending %s...', data)
            #print(data)
            self.transport.write(data.encode())
        else:
            self.transport.loseConnection()
    def connectionMade(self):
        self.sendData()
    def dataReceived(self,data):
        dataList = list(map(ord, data.decode()))
        
        logger.debug("%s", dataList)
        self.sendData()
    
class TSClntFactory(protocol.ClientFactory):
    protocol = TSClntProtocol
    clientConnectionLost = clientConnectionFailed = lambda self, connector, reason: reactor.stop()

def twistedClient():    
    reactor.connectTCP(HOST, PORT, TSClntFactory())
    reactor.run()
   
# function definition here

# test function here

if __name__=='__main__':
    #main()
    #main_queue()
#    tsTclinet()
    #tsUclinet()
    #tsTclientt()
    twistedClient()
        
   