/*
 * ReTCPPeer.cpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#include "base/rebase.hpp"
#include "math/remath.hpp"
#include "net/renet.hpp"

enum {
   LOC_SEND_1 = LOC_FIRST_OF(LOC_TCPPEER), // 10801
   LOC_READ_BYTES_1,
   LOC_READ_BYTES_2,
   LOC_READ_BYTES_3,
   LOC_READ_BYTES_4,
   LOC_HANDLE_ERROR_1,
   LOC_SEND_2,
};

static int s_dummy = 0;

/** @class ReTCPPeer ReTCPPeer.hpp "net/ReTCPPeer.hpp"
 *
 * @brief Implements the common things for TCP server and client.
 *
 * The communication is done with the following protocol:
 * <ul>
 *  <li>The data is transmitted via TCP.</li>
 *  <li>The data exchange is done with <b>info units</b>.
 *  <li>Each info unit contains a header and the data.</li>
 * </ul>
 * The format of the header:
 *<pre>FLAGS [SALT] COMMAND SIZE
 * </pre>
 * <ul>
 *  <li>FLAGS (1 byte): a XOR sum of the flags defined in <code>rpltcppeer::flag_t</code>.</li>
 *  <li>SALT (4 byte): a random value. Controls the encryption. Only available if <code>FLAG_ENCRYPT</code> is set.</li>
 *  <li>COMMAND (5 byte): define the task to do (client to server) or the answer (server to client).
 *  <li>SIZE (2 or 4 byte): the size of the data behind the header. 4 bytes if <code>FLAG_4_BYTE_SIZE</code> is set.</li>
 * </ul>
 *
 */

/**
 * @brief Creates an instance of a <code>ReTCPPeer</code>.
 *
 * @param configurator  delivers some connection parameters
 * @param thread        the current thread. Used for sleep()
 * @param terminator    NULL or for controlled thread termination
 * @param logger        logger. If Null the global logger will be taken (not thread safe!)
 * @return              an instance of <code>ReTCPPeer</code>
 */
ReTCPPeer* ReTCPPeer::createPeer(ReConfigurator& configurator, QThread* thread,
                                 ReTerminator* terminator, ReLogger* logger) {
   return new ReTCPPeer(configurator, thread, terminator, logger);
}

/**
 * @brief Constructor.
 *
 * @param configurator  delivers some connection parameters
 * @param thread        the current thread. Used for sleep()
 * @param terminator    NULL or for controlled thread termination
 * @param isServer      true: the receiving does have a timeout
 * @param logger        logger. If Null the global logger will be taken (not thread safe!)
 */
ReTCPPeer::ReTCPPeer(ReConfigurator& configurator, QThread* thread,
                     ReTerminator* terminator, bool isServer, ReLogger* logger) :
   m_socket(NULL),
   m_logger(logger == NULL ? ReLogger::globalLogger() : logger),
   m_thread(thread),
   m_random(),
   m_timeout(isServer ? 0 : configurator.asInt("connection.timeout", 60)),
   m_terminator(terminator),
   m_configurator(configurator),
   m_isServer(isServer),
   m_dataLocker(QMutex::NonRecursive),
   m_waitForData() {
   // Simulate a true random with time, and addresses from stack and code segment:
   m_random.setSeed(
      time(NULL) + ((int64_t) this << 8) + ((int64_t) &s_dummy << 16)
      + ((int64_t) &createPeer << 24));
}

/**
 * @brief Destructor.
 */
ReTCPPeer::~ReTCPPeer() {
}

/**
 * @brief Sends a message via TCP.
 *
 * @param flags     a sum of FLAGS_... constants
 * @param command   defines the content of the message
 * @param data      NULL or additional data
 * @return          true: success<br>
 *                  false: error occurred
 */
bool ReTCPPeer::send(qint8 flags, const char* command, const QByteArray& data) {
   bool rc = false;
   QByteArray header;
   QByteArray data2 = ReStringUtil::toCString(data.constData(), 20);
   m_logger->logv(LOG_INFO, LOC_SEND_1, "send: flags: %x %s %s (%d)", flags,
                  command, data2.constData(), data.length());
   header.reserve(16);
   header.append((char) flags);
   if (flags & FLAG_ENCRYPT) {
      header.append((char) m_random.nextByte());
      header.append((char) m_random.nextByte());
      header.append((char) m_random.nextByte());
      header.append((char) m_random.nextByte());
   }
   unsigned int length = data.length();
   header.append(char(length % 256));
   header.append(char((length >> 8) % 256));
   if (flags & FLAG_4_BYTE_SIZE) {
      header.append(char((length >> 16) % 256));
      header.append(char((length >> 24) % 256));
   }
   length = strlen(command);
   header.append(command, length < 5 ? length : 5);
   while (length++ < 5) {
      header.append(' ');
   }
   int64_t written = m_socket->write(header.constData(), header.length());
   int64_t written2 = m_socket->write(data);
   m_socket->flush();
   int count = 0;
   if (written != header.length() || written2 != data.length()) {
      int endTime = time(NULL) + m_timeout;
      // wait until the data are sent or timeout or external termination:
      while (m_socket->bytesToWrite() > 0) {
         m_thread->msleep(1);
         if (++count % 20 == 0) {
            if (m_terminator == NULL || m_terminator->isStopped()
                  || time(NULL) > endTime)
               break;
         }
      }
   }
   if (m_logger->isActive(LOG_DEBUG))
      m_logger->logv(LOG_DEBUG, LOC_SEND_1, "send %s: %s len=%d loops=%d %s",
                     m_address.constData(), command, data.length(), count,
                     ReStringUtil::hexDump((const void*) data.constData(), 16, 16)
                     .constData());
   return rc;
}

/**
 * @brief Reads an amount of bytes with a timeout.
 *
 * @param bytes     count of bytes to read
 * @param maxTime   IN/OUT: last time the read must be ready
 * @param loops     IN/OUT: number of sleeps
 *
 * @return          "": read not successful: timeout or termination or error<br>
 *                  otherwise: the read bytes
 */
QByteArray ReTCPPeer::readBytes(int bytes, time_t maxTime, int& loops) {
   QAbstractSocket* socket = getSocket();
   bool success = true;
   int64_t available;
   long msec = m_configurator.asInt(ReNetConfig::SLEEP_MILLISEC, 1);
   int divider = 1000L / (msec == 0 ? 1 : msec);
   if (divider < 1)
      divider = 1;
   QMutexLocker locker(&m_dataLocker);
   m_dataLocker.lock();
   while (!m_waitForData.wait(&m_dataLocker, 1000L)) {
      if (loops == 0 && !m_isServer)
         maxTime = time(NULL) + m_timeout;
      if (++loops % divider == 0 && !m_isServer) {
         if (time(NULL) > maxTime) {
            m_logger->logv(LOG_ERROR, LOC_READ_BYTES_1,
                           "receive: timeout (%d)", m_timeout);
            success = false;
            break;
         }
      }
      if (m_terminator != NULL && m_terminator->isStopped()) {
         m_logger->log(LOG_ERROR, LOC_READ_BYTES_2, "receive: stopped");
         success = false;
         break;
      }
   }
   available = socket->bytesAvailable();
   m_logger->logv(LOG_DEBUG, LOC_READ_BYTES_4,
                  "readBytes(): available: %ld/%ld", available, bytes);
   QByteArray rc;
   if (success) {
      rc = socket->read(bytes);
      if (rc.length() != bytes) {
         m_logger->logv(LOG_ERROR, LOC_READ_BYTES_3,
                        "receive: too few bytes: %d of %d", rc.length(), bytes);
      }
   }
   return rc;
}

int getInt(const QByteArray& data, int offset, int size) {
   int rc = ((int) (unsigned char) data.at(offset++));
   while (--size > 0) {
      rc = rc * 256 + (unsigned char) data.at(offset++);
   }
   return rc;
}

/**
 * @brief Receives a message via TCP.
 *
 * @param command   OUT: defines the content of the read message
 * @param data      OUT: "" or additional data
 * @return          true: success<br>
 *                  false: error occurred
 */
bool ReTCPPeer::receive(QByteArray& command, QByteArray& data) {
   bool rc = true;
   command.clear();
   data.clear();
   QByteArray header;
   header.reserve(16);
   int minHeaderSize = 8;
   int loops = 0;
   time_t maxTime = 0;
   uint8_t flags = 0;
   header = readBytes(minHeaderSize, maxTime, loops);
   if (header.length() > 0) {
      flags = header.at(0);
      int headerSize = minHeaderSize;
      if ((flags & FLAG_4_BYTE_SIZE) != 0)
         headerSize += 2;
      if ((flags & FLAG_ENCRYPT) != 0)
         headerSize += 4;
      if (headerSize != minHeaderSize) {
         QByteArray restHeader = readBytes(headerSize - minHeaderSize,
                                           maxTime, loops);
         if (restHeader.length() == 0)
            header.clear();
         else
            header.append(restHeader);
      }
   }
   rc = header.length() > 0;
   if (rc) {
      int offset = (flags & FLAG_ENCRYPT) == 0 ? 6 : 8;
      int size = (flags & FLAG_4_BYTE_SIZE) == 0 ? 4 : 2;
      int dataLength = getInt(header, offset, size);
      command = header.mid(offset - 5, 5);
      data = readBytes(dataLength, maxTime, loops);
      rc = data.length() == dataLength;
   }
   return rc;
}

/**
 * @brief Sends a message and receives an answer message via TCP.
 *
 * @param flags     a sum of FLAGS_... constants
 * @param command       defines the content of the message to send
 * @param data          NULL or additional data
 * @param answer        OUT: the command of the answer
 * @param answerData    OUT: "" or additional data of the answer
 * @return              true: success<br>
 *                      false: error occurred
 */
bool ReTCPPeer::sendAndReceive(uint8_t flags, char command[4], QByteArray* data,
                               QByteArray& answer, QByteArray& answerData) {
   answer.clear();
   answerData.clear();
   bool rc = send(flags, command, data == NULL ? QByteArray("") : *data);
   if (rc)
      rc = receive(answer, answerData);
   return rc;
}

/**
 * @brief Sets the socket.
 *
 * @param socket    the socket to set
 */
void ReTCPPeer::setSocket(QAbstractSocket* socket) {
   m_socket = socket;
   if (socket != NULL)
      connect(m_socket, SIGNAL(readyRead()), SLOT(readTcpData()));
}

/**
 * @brief Reads the (ready) data from the socket.
 */
void ReTCPPeer::readTcpData() {
   m_waitForData.wakeOne();
}

/**
 * @brief Handles a network error.
 *
 * @param socketError   the error code
 */
void ReTCPPeer::handleError(QTcpSocket::SocketError socketError) {
   m_logger->logv(LOG_ERROR, LOC_HANDLE_ERROR_1, "Network error %d",
                  socketError);
}

/**
 * @brief Returns a human readable peer address.
 * @return a string with the peer address: e.g. "192.16.2.3:44335"
 */
QByteArray ReTCPPeer::getPeerAddress() {
   QByteArray rc;
   if (m_socket == NULL)
      rc = "<not connected>";
   else
      rc = m_socket->peerAddress().toString().toLatin1();
   return rc;
}

/**
 * @brief Returns the socket.
 *
 * @return the socket
 */
QAbstractSocket* ReTCPPeer::getSocket() const {
   return m_socket;
}

/**
 * @brief Returns the port.
 * @return the port of the peer.
 */
int ReTCPPeer::getPort() {
   int port = m_configurator.asInt(ReNetConfig::PORT, 12345);
   return port;
}
/**
 * @brief Returns the ip address.
 * @return  "": all addresses (for listening)<br>
 *          otherwise: the address (e.g. 127.0.0.1)
 */
QByteArray ReTCPPeer::getIp() {
   QByteArray ip = m_configurator.asString(ReNetConfig::IP, "");
   return ip;
}
/**
 * @brief Sets the address (ip:port).
 * @param ip    the ip address
 * @param port  the port
 */
void ReTCPPeer::setAddress(const char* ip, int port) {
   m_address = QByteArray(ip) + ":" + QByteArray::number(port);
}
