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
import struct

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
#HOST = '123.206.115.17'
#HOST = '192.168.1.103'
HOST= 'localhost'
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
            if data.strip()[0:4] != 'DATA':
                logger.info( '...sending %s...', data)
                #print(data)
                self.transport.write(data.encode())
            else:
                #'DATA'
                dataBytes = []
                boardID=randint(1,3)
                for i in range(248):
                    dataBytes.append(randint(1,255))
                binData = struct.pack('>4B2H248B', ord('D'),ord('A'),ord('T'),ord('A'),len(dataBytes)+2,boardID,*dataBytes)
#                binData = struct.pack('B'*16, ord('D'),ord('A'),ord('T'),ord('A'),0,16,0,1,
#                                   randint(1,255), randint(1,255), randint(1,255), randint(1,255),
#                                   randint(1,255), randint(1,255), randint(1,255), randint(1,255))
                logger.info('...%s...', binData)
                self.transport.write(binData)
        else:
            self.transport.loseConnection()
    def connectionMade(self):
        self.sendData()
    def dataReceived(self,data):
        dataList = list(map(ord, data.decode()))
        logger.debug("%s", data)
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
    strData = 'DATA****'
    for i in range(256):
        strData +=str(randint(1,256)) 
    
#    binData=struct.pack('B'*264, strData)
    binData = struct.pack('B'*16, ord('D'),ord('A'),ord('T'),ord('A'),0,16,0,1,
                                   randint(1,255), randint(1,255), randint(1,255), randint(1,255),
                                   randint(1,255), randint(1,255), randint(1,255), randint(1,255))
    #main()
    #main_queue()
#    tsTclinet()
    #tsUclinet()
    #tsTclientt()
    twistedClient()
        
   