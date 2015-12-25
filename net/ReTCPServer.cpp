/*
 * ReTCPServer.cpp
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
   LOC_RUN_1 = LOC_FIRST_OF(LOC_TCPSERVER), // 10601
   LOC_TCP_TREAD_RUN_1,
   LOC_TCP_TREAD_RUN_2,
   LOC_TCP_INCOMING_CONNECTION_1,
};

/** @class ReTCPThread ReTCPServer.hpp "net/ReTCPServer.hpp"
 *
 * @brief Serves one connection of a multihreaded TCP server.
 *
 * Note: The real thing is done by the ReTaskHandler instance.
 */

/**
 * @brief Constructor.
 *
 * @param configurator  delivers some connection parameters
 * @param socketDescriptor  socket of the connection to handle
 * @param threadId          an unique id for the thread
 * @param handler           does the work
 */
ReTCPThread::ReTCPThread(ReConfigurator& configurator, qintptr socketDescriptor,
                         int threadId, ReTaskHandler* handler) :
   m_threadId(threadId),
   m_taskHandler(handler),
   m_socketDescriptor(socketDescriptor),
   m_configurator(configurator) {
}
/**
 * @brief Destructor.
 */
ReTCPThread::~ReTCPThread() {
}

/**
 * @brief Does the proper thread task.
 *
 * Initializes the socket and loops for incoming commands.
 */
void ReTCPThread::run() {
   QTcpSocket socket;
   if (!socket.setSocketDescriptor(getSocketDescriptor())) {
      emit error(socket.error());
   } else {
      ReTCPPeer peer(m_configurator, this, m_taskHandler->getTerminator(),
                     true, m_taskHandler->getLogger());
      peer.setSocket(&socket);
      QByteArray addr = peer.getPeerAddress();
      m_taskHandler->getLogger()->logv(LOG_DEBUG, LOC_TCP_TREAD_RUN_1,
                                       "ReTCPThread::run(): start Peer: %s", addr.constData());
      while (m_taskHandler->handle(&peer)) {
         // do nothing
      }
      socket.disconnectFromHost();
      socket.waitForDisconnected();
      m_taskHandler->getLogger()->logv(LOG_DEBUG, LOC_TCP_TREAD_RUN_1,
                                       "ReTCPThread::run(): end Peer: %s", addr.constData());
   }
}

/**
 * @brief Returns the thread id.
 *
 * @return the thread id
 */
int ReTCPThread::getThreadId() const {
   return m_threadId;
}
/**
 * @brief Returns the task handler.
 *
 * @return the task handler
 */
ReTaskHandler* ReTCPThread::getTaskHandler() const {
   return m_taskHandler;
}
/**
 * @brief Returns the tcp socket of the served connection.
 *
 * @return the socket
 */
qintptr ReTCPThread::getSocketDescriptor() const {
   return m_socketDescriptor;
}

/** @class ReTCPServer ReTCPServer.hpp "base/ReTCPServer.hpp"
 *
 * @brief Implements a multithreaded TCP server.
 */

/**
 * @brief Constructor.
 *
 * @param configurator  some parameters will be get from this configurator
 * @param taskHandler       this handler reads from the tcp and interprets the content
 * @param threadFactory     creates a thread for a new connection
 * @param logger            NULL or logger
 * @param parent            NULL or the parent which deletes the childen
 */
ReTCPServer::ReTCPServer(ReConfigurator& configurator,
                         ReTaskHandler* taskHandler, ReThreadFactory& threadFactory,
                         ReLogger* logger, QObject* parent) :
   QTcpServer(parent),
   m_taskHandler(taskHandler),
   m_threadId(0),
   m_threadFactory(threadFactory),
   m_configurator(configurator) {
}

/**
 * @brief The slot handling a new tcp connection.
 *
 * @param socketDescriptor  the tcp socket
 */
void ReTCPServer::incomingConnection(qintptr socketDescriptor) {
   ReTCPThread* thread = m_threadFactory.create(m_configurator,
                         socketDescriptor, ++m_threadId, m_taskHandler);
   m_taskHandler->getLogger()->log(LOG_DEBUG, LOC_TCP_INCOMING_CONNECTION_1,
                                   "Connection detected");
   QTcpServer::connect(thread, SIGNAL(finished()), thread,
                       SLOT(deleteLater()));
   thread->start();
}

/** @class ReTCPThread ReTCPServer.hpp "net/ReTCPServer.hpp"
 *
 * @brief Defines a function pointer type to create a <code>ReTCPThread</code> instance.
 *
 */

/** @class ReTaskHandler ReTCPServer.hpp "net/ReTCPServer.hpp"
 *
 * @brief An abstract base class for an handler processing  an data unit.
 *
 * The handler knows the stucture of the data unit and can interpret this.
 */
/**
 * @brief Constructor
 *
 * @param configurator  delivers some connection parameters
 * @param terminator    external controller for thread termination
 * @param logger        the logger
 */
ReTaskHandler::ReTaskHandler(ReConfigurator& configurator,
                             ReTerminator* terminator, ReLogger* logger) :
   m_answerFlags(0),
   m_logger(logger),
   m_terminator(terminator),
   m_configurator(configurator) {
}

/**
 * @brief Destructor.
 */
ReTaskHandler::~ReTaskHandler() {
}

/**
 * @brief Reads one data unit, processes it and sends the answer.
 *
 * @param peer      the communication partner
 * @return          false: the application should stop<br>
 *                  true: processing remains
 */
bool ReTaskHandler::handle(ReTCPPeer* peer) {
   QByteArray command;
   QByteArray data;
   QByteArray answer;
   QByteArray answerData;
   bool rc = true;
   if (peer->receive(command, data)) {
      rc = process(command, data, answer, answerData);
      if (answer.length() > 0) {
         peer->send(m_answerFlags, answer, answerData);
      }
   }
   return rc;
}

/**
 * @brief Sets the thead id.
 *
 * @param id    the thread id
 */
void ReTaskHandler::setThreadId(int id) {
   m_threadId = id;
}

/**
 * @brief Gets the thread id.
 *
 * @return the thread id
 */
int ReTaskHandler::getThreadId() const {
   return m_threadId;
}

/**
 * @brief Returns the logger.
 *
 * @return  the logger
 */
ReLogger* ReTaskHandler::getLogger() const {
   return m_logger;
}

/**
 * @brief Returns the termination controller.
 *
 * @return the termination controller
 */
ReTerminator* ReTaskHandler::getTerminator() const {
   return m_terminator;
}

