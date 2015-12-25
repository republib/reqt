/*
 * ReTcpClient.cpp
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
   LOC_1 = LOC_FIRST_OF(LOC_TCPCLIENT), // 10701
   LOC_HANDLE_ERROR_1,
   LOC_SET_REMOTE_ADDRESS_1,
};

/** @class RplTcpClient ReTcpClient.hpp "net/ReTcpClient.hpp"
 *
 * @brief Implements a TCP client.
 *
 * Use the protocol defined at <code>ReTCPServer</code>.
 */
/**
 * @brief Constructor.
 *
 * @param configurator  some parameters will be get from this configurator
 * @param thread        current thread. Used for <code>sleep()</code>
 * @param terminator    NULL or for controlled termination
 * @param logger        a logger
 */
RplTcpClient::RplTcpClient(ReConfigurator& configurator, QThread* thread,
                           ReTerminator* terminator, ReLogger* logger) :
   m_peer(new ReTCPPeer(configurator, thread, terminator, false, logger)),
   m_logger(logger),
   m_configurator(configurator) {
   QByteArray ip = configurator.asString(ReNetConfig::IP, "localhost");
   int port = configurator.asInt(ReNetConfig::PORT, 12345);
   if (!ip.isEmpty() && port != 0)
      setRemoteAddress(ip.constData(), port);
}

/**
 * @brief Destructor.
 */
RplTcpClient::~RplTcpClient() {
   delete m_peer;
   m_peer = NULL;
}

/**
 * @brief Defines the remote address for a client.
 * @param ip    NULL or the ip to connect
 * @param port  0 or the port to connect
 */
void RplTcpClient::setRemoteAddress(const char* ip, int port) {
   QTcpSocket* socket = (QTcpSocket*) m_peer->getSocket();
   delete socket;
   if (ip == NULL || port == 0)
      m_peer->setSocket(NULL);
   else {
      socket = new QTcpSocket();
      connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
              SLOT(handleError(QAbstractSocket::SocketError)));
      m_peer->setSocket(socket);
      m_peer->setAddress(ip, port);
      m_logger->logv(LOG_INFO, LOC_SET_REMOTE_ADDRESS_1, "connect with %s:%d",
                     ip, port);
      socket->connectToHost(ReString(ip), port);
      socket->waitForConnected();
   }
}

/**
 * @brief Returns the peer info.
 * @return the peer info
 */
ReTCPPeer* RplTcpClient::getPeer() const {
   return m_peer;
}

/**
 * @brief Handles a network error.
 *
 * @param socketError   the error code
 */
void RplTcpClient::handleError(QAbstractSocket::SocketError socketError) {
   if (m_logger != NULL)
      m_logger->logv(LOG_ERROR, LOC_HANDLE_ERROR_1, "Network error %d",
                     socketError);
}

/** @class RplClientThread ReTcpClient.hpp "net/ReTcpClient.hpp"
 *
 * @brief Implements a thread usable for a tcp client.
 *
 * Each <code>ReTCPPeer</code> needs a thread. Therefore this class provides all things
 * needed for a <code>RplTcpClient</code> which uses a <code>ReTCPPeer</code>.
 */

/**
 * @brief Constructor.
 *
 * @param configurator  delivers some connection parameters
 * @param logger        the logger. If NULL a default logger will be used
 */
RplClientThread::RplClientThread(ReConfigurator& configurator, ReLogger* logger) :
   m_client(NULL),
   m_logger(logger),
   m_configurator(configurator),
   m_ownsLogger(false) {
   m_client = new RplTcpClient(configurator, this, NULL, logger);
}
/**
 * @brief Destructor.
 */
RplClientThread::~RplClientThread() {
   delete m_client;
   m_client = NULL;
   if (m_ownsLogger) {
      delete m_logger;
      m_logger = NULL;
   }
}
/**
 * @brief Returns the peer which can be used for sending and receiving messages.
 *
 * @return the peer
 */
ReTCPPeer* RplClientThread::getPeer() const {
   return m_client->getPeer();
}

/**
 * @brief Returns the logger of the thread.
 * @return the logger
 */
ReLogger* RplClientThread::getLogger() const {
   return m_logger;
}

/**
 * @brief Contains the main method of the thread.
 *
 * Calls <code>doIt()</code> for the real things.
 */
void RplClientThread::run() {
   doIt();
}
