﻿# !/usr/bin/env python
'''This is the test excecises for chapter 16 of CORE_PYTHON_PROGRAMMING
Network Programming
'''
import time
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
#HOST = '192.168.1.105'
HOST = ''
PORT = 21567
ADDR = (HOST, PORT)
BUFSIZ=1024

def tsTserv():
    #from socket import *
    tcpSerSock=socket(AF_INET,SOCK_STREAM) #创服务器套接字
    tcpSerSock.bind(ADDR) #套接字与地址绑定
    tcpSerSock.listen(5)  #监听连接,传入连接请求的最大数
    

    while True:
        print('waiting for connection...')
        tcpCliSock,addr =tcpSerSock.accept()
        print('...connected from:',addr)
        while True:
            data =tcpCliSock.recv(BUFSIZ).decode()
            print('date=',data)
            if not data:
                break
            tcpCliSock.send(('[%s] %s' %(ctime(),data)).encode())
        tcpCliSock.close()
    tcpSerSock.close()

def tsUserv():
    udpSerSock = socket(AF_INET,SOCK_DGRAM)
    udpSerSock.bind(ADDR)
    
    while True:
        print('waiting for message...')
        data,addr=udpSerSock.recvfrom(BUFSIZ)
        data=data.decode()
        udpSerSock.sendto(('[%s] %s'%(ctime(),data)).encode(),addr)
        print('...received from and returned to:',addr)
    
    udpSerSock.close()

from socketserver import (TCPServer as TCP, StreamRequestHandler as SRH)
from socketserver import BaseServer as BSC
from time import ctime
 
# heritage SRH father class
class MyRequestHandler(SRH):
    # overide handle() method
    def handle(self):
        # StreamRequestHandler类把输入和输出的套接字看做类似文件的对象
        # readline(): read client info，write(): write data to client
        #print('---Connected from: ' ,self.client_address, self.wfile.write(('[%s]%s'%(ctime(), self.rfile.readline().decode('utf-8'))).encode('utf-8')))
        logger.info('---Connected from: %s' ,self.client_address)
        data= self.rfile.readline().decode("utf-8")
        if data.strip() is not '':
            logger.debug('data received is %s', data)
            result = data
            self.wfile.write(('[%s]%s'%(ctime(), result)).encode('utf-8'))
 
def tsTservSS():
    # 使用给定的地址和请求处理类创建TCP服务器
    tcpServ = TCP(ADDR, MyRequestHandler)
    logger.info('Waiting for connection...')
    # 无限的等待并服务于客户端
    tcpServ.serve_forever()

from twisted.internet import protocol, reactor
from time import ctime

# Class definition here
class TSServProtocol(protocol.Protocol):
    def __init__(self, factory):
        self.factory = factory
    def connectionMade(self):
        clnt = self.clnt = self.transport.getPeer()
        self.factory.users[1] = self
        logger.info("...connected from: %s", clnt)
    def dataReceived(self, data):
        #self.transport.write(("[%s] %s"%(ctime(), data.decode())).encode())
        logger.debug("%s",data)
#        str_data = data.decode('utf-8')
        str_data = data
#        str_data = struct.unpack('>c*',data)
        if (str_data.strip()[0:6]==b'r+Time'):
            #logger.debug("run here")
            strHandler = time.localtime(time.time())
            
            strYear = strHandler.tm_year-2000
            strMon = strHandler.tm_mon
            strDay = (strHandler.tm_mday)
            strHour =(strHandler.tm_hour)
            strMin = (strHandler.tm_min)
            strTime ='!'+chr(20)+chr(strYear)+chr(strMon)+chr(strDay)+chr(strHour)+chr(strMin)
            #strTime = '!'+datetime.fromtimestamp(time.time()).strftime("%Y%m%d%H%M")
            #strTime ='!'+chr(0x05)+chr(0x0a)+chr(0x05)+chr(0x0A)+chr(0x05)+chr(0x0A)+chr(0x05) +chr(0x0A)+chr(strDay)+chr(strHour)+chr(strMin)+'\n'
            bTime= struct.pack('B'*7, 33,20, strHandler.tm_year-2000, strHandler.tm_mon, strHandler.tm_mday, strHandler.tm_hour, strHandler.tm_min)
            
            #strTime ='!'+chr(20)+chr(strYear)+chr(strMon)
            logger.debug("%s",bTime)
            #self.transport.write(strTime.encode())
            self.transport.write(bTime)
            
        elif(str_data.strip()[0:4]==b"DATA"):
            boardID = (int(str_data[6])<<8) + int(str_data[7])
            length = str_data[4:6]
#            print(length, ' ', str_data[6:8])
#            print(str_data[7])
            print(boardID)
            filename = 'db'+('%5s'%str(boardID)).replace(' ','0')+'.bin'
            print(filename)
            with open(filename,'ab') as fw:
#                fw.write(str_data[9:])
                fw.write(str_data[0:])
                logger.debug("write %s",filename)
            self.transport.write(b'\0')
        else:
            #self.transport.write(("%s"%"\0").encode('utf-8'))
            self.transport.write(str_data)
    
class ServerFactory(protocol.Factory):
    def __init__(self):
        self.users = {}
        pass

    def buildProtocol(self, addr):
        #return Chat(self.users)     #Chat()
        return TSServProtocol(self)     #Chat(ChatFactory)
    
    def getCMD(self, *args):
        while True:
            cmd = input('>')
            print(cmd)
            self.users[1].transport.write(cmd.encode())
    def check_users_online(self):
        """
        隔一段时间，服务器整体轮询一次，如果发现某一个客户端很长时间没有接受到心跳包，就判定它为断线，这时候主动切断这个客户端
        """
        for key, value in self.users.items():
            if value.last_heartbeat_time != 0 and int(time.time()) - value.last_heartbeat_time > 4:
                log.msg("[%s]没有检测到心跳包,主动切断" % key.encode('utf-8'))
                value.transport.abortConnection()
            else:
#                log.msg("[ %s] heart beating" % (key))
                pass
def twistedServer():
    factory = protocol.Factory()
    factory.protocol = TSServProtocol
    logger.debug("waiting for connection...")
    reactor.listenTCP(PORT, factory)
    reactor.run()



def test1_twistedServer():
    sf = ServerFactory()
    reactor.callInThread(sf.getCMD,0,0)
    logger.debug("waiting for connection...")
    reactor.listenTCP(21567, sf)
    
    reactor.run()


# function definition here

# test function here

if __name__=='__main__':
    #main()
    #main_queue()
#    tsTserv()
    #tsUserv()
#    tsTservSS()
#      twistedServer()
      test1_twistedServer()
   