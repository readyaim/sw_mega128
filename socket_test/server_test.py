# !/usr/bin/env python
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
    def connectionMade(self):
        clnt = self.clnt = self.transport.getPeer().host
        logger.info("...connected from: %s", clnt)
    def dataReceived(self, data):
        #self.transport.write(("[%s] %s"%(ctime(), data.decode())).encode())
        print(data.decode())
        str_data = data.decode()
        if (str_data.strip()[0:6]=='r+Time'):
            strHandler = time.localtime(time.time())
            strYear = '!'+str(strHandler.tm_year)
            strMon = str(strHandler.tm_mon) if strHandler.tm_mon>9 else '0'+str(strHandler.tm_mon)
            strDay = str(strHandler.tm_mday) if strHandler.tm_mday>9 else '0'+str(strHandler.tm_mday)
            strHour = str(strHandler.tm_hour) if strHandler.tm_hour>9 else '0'+str(strHandler.tm_hour)
            strMin = str(strHandler.tm_min) if strHandler.tm_min>9 else '0'+str(strHandler.tm_min)
            strTime = strYear +strMon+strDay+strHour+strMin
            self.transport.write(("%s"%strTime).encode('utf-8'))
        
        elif(str_data[0:4]=="DATA"):
            with open("db.txt",'w') as fw:
                fw.write(str_data[4:])
            self.transport.write(('ACK').encode('utf-8'))
        else:
            self.transport.write(("%s"%"\0").encode('utf-8'))

def twistedServer():
    factory = protocol.Factory()
    factory.protocol = TSServProtocol
    logger.debug("waiting for connection...")
    reactor.listenTCP(PORT, factory)
    reactor.run()
# function definition here

# test function here

if __name__=='__main__':
    #main()
    #main_queue()
#    tsTserv()
    #tsUserv()
#    tsTservSS()
      twistedServer()
   