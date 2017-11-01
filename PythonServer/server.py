import socket
from threading import Thread
import sys


# Handles all incoming TCP connections with threads
class server:

    # Set up basic information for use when the server is started
    # parserCallback is the function that the handling thread should parse on its received data to.
    def __init__(self, logging, listenHost, listenPort, parserCallbackList ):
        self.logging = logging
        self.listenHost = listenHost
        self.listenPort = listenPort
        self.parserCallbackList = parserCallbackList

        self.incomingData = ""


    # Validates the size of the incoming packet. If ok it's sent on the the parser
    def clientThread(self, conn, ip, port):
        self.incomingData = conn.recv(4096)
        packetSize = sys.getsizeof(self.incomingData)

        conn.close()
        self.logging.debug("Client disconnect")

        if packetSize >= 4096 or packetSize < 9:
            self.logging.error("Invalid packet length: {}".format(siz))
        else:
            self.printHex()
            # Find the ID of the device talking and call the parser for it.
            deviceID = self.incomingData[7]
            if deviceID in self.parserCallbackList:
                self.parserCallbackList[deviceID](self.incomingData) # We pass on the packet to the paser of the deviceID. This threads job is over
            else:
                self.logging.error("Unknow device ID: {}".format(deviceID))


    # This will be in an eternal loop that spawns a new thread every time it gets a new tcp connection to the listening port
    # All new connections will be handled in method clientThread.
    # This method will never leave after called
    def startServer(self):
        soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        soc.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.logging.info("Socket created")

        try:
            soc.bind((self.listenHost, self.listenPort))
            self.logging.info("Socket bind complete")
        except socket.error as msg:
            self.logging.error("Bind failed. Error : " + str(sys.exc_info()))
            sys.exit()

        soc.listen(10)
        self.logging.info("Socket now listening")

        while True:
            conn, addr = soc.accept()
            ip, port = str(addr[0]), str(addr[1])
            self.logging.info("Accepting connection from " + ip + ":" + port)
            try:
                Thread(target=self.clientThread, args=(conn, ip, port)).start()
            except:
                self.logging.error("Terible error!")
        soc.close()


    # Debug function that will print out a received datapacket byte by byte
    def printHex(self):
        new_str = "Received raw data = "
        for i in self.incomingData:
            new_str += str(i) + " "
        self.logging.info( new_str )