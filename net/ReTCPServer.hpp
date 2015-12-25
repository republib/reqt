/*
 * ReTCPServer.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#ifndef RPLTCPSERVER_HPP
#define RPLTCPSERVER_HPP

#ifndef REBASE_HPP
#include "base/rebase.hpp"
#include "math/remath.hpp"
#include "net/renet.hpp"
#endif

// the sources generated from QT include this file directly:
#ifndef RPLNET_HPP
#include "renet.hpp"
#endif

class ReTCPPeer;
class ReTaskHandler {
public:
   ReTaskHandler(ReConfigurator& configurator, ReTerminator* terminator,
                 ReLogger* logger);
   virtual ~ReTaskHandler();
public:

   virtual bool handle(ReTCPPeer* peer);
   /**
    * @brief Processes one data unit from the socket.
    *
    * @param command       defines the meaning of the information unit
    * @param data          "" or the data of the information unit
    * @param answer        OUT: "" or the answer to send back
    * @param answerData    OUT: "" or the answer data to send back
    * @return              true: the receiving loop should be continued<br>
    *                      false: the process should be stopped
    */
   virtual bool process(const QByteArray& command, const QByteArray& data,
                        QByteArray& answer, QByteArray& answerData) = 0;
   void setThreadId(int id);
   int getThreadId() const;
   ReLogger* getLogger() const;
   ReTerminator* getTerminator() const;
protected:
   uint8_t m_answerFlags;
private:
   int m_threadId;
   ReLogger* m_logger;
   ReTerminator* m_terminator;
   ReConfigurator& m_configurator;
};

class ReTCPThread: public QThread {
   Q_OBJECT
public:
   ReTCPThread(ReConfigurator& m_configurator, qintptr socketDescriptor,
               int threadId, ReTaskHandler* handler);
   virtual ~ReTCPThread();
private:
   // No copy constructor: no implementation!
   ReTCPThread(const ReTCPThread& source);
   // No assignment operator: no implementation!
   ReTCPThread& operator=(const ReTCPThread& source);
public:
   void run();
   int getThreadId() const;
   ReTaskHandler* getTaskHandler() const;
   qintptr getSocketDescriptor() const;

signals:
   void error(QTcpSocket::SocketError socketError);

private:
   // a unique id for the thread
   int m_threadId;
   // this handler interprets the info from the TCP connection
   ReTaskHandler* m_taskHandler;
   // the assigned socket
   qintptr m_socketDescriptor;
   ReConfigurator& m_configurator;
};
class ReThreadFactory {
public:
   virtual ReTCPThread* create(ReConfigurator& configurator,
                               qintptr socketDescriptor, int threadId, ReTaskHandler* handler) = 0;
};

class ReTCPPeer;
class ReTCPServer: public QTcpServer, public ReTerminator {
   Q_OBJECT
public:
   explicit ReTCPServer(ReConfigurator& configurator,
                        ReTaskHandler* taskHandler, ReThreadFactory& threadFactory,
                        ReLogger* logger = NULL, QObject* parent = 0);
private:
   // No copy constructor: no implementation!
   ReTCPServer(const ReTCPServer& source);
   // No assignment operator: no implementation!
   ReTCPServer& operator=(const ReTCPServer& source);
public:
   ReTCPPeer* getPeer() const;
   bool handleTask();

protected slots:
   void incomingConnection(qintptr socketDescriptor);

private:
   ReTaskHandler* m_taskHandler;
   int m_threadId;
   ReTCPPeer* m_peer;
   ReThreadFactory& m_threadFactory;
   ReConfigurator& m_configurator;
};

#endif // RPLTCPSERVER_HPP
