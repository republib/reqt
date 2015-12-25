/*
 * ReTcpClient.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#ifndef RETCPCLIENT_HPP
#define RETCPCLIENT_HPP

#ifndef REBASE_HPP
#include "base/rebase.hpp"
#include "math/remath.hpp"
#include "net/renet.hpp"
#endif

class ReTCPPeer;

class RplTcpClient: public QObject {
   Q_OBJECT
public:
   RplTcpClient(ReConfigurator& configurator, QThread* thread,
                ReTerminator* terminator, ReLogger* logger = NULL);
   virtual ~RplTcpClient();
private:
   // No copy constructor: no implementation!
   RplTcpClient(const RplTcpClient& source);
   // Prohibits assignment operator: no implementation!
   RplTcpClient& operator =(const RplTcpClient& source);
public:
   ReTCPPeer* getPeer() const;
private:
   void setRemoteAddress(const char* ip, int port);
public slots:
   void handleError(QAbstractSocket::SocketError socketError);
private:
   ReTCPPeer* m_peer;
   ReLogger* m_logger;
   ReConfigurator& m_configurator;
};

class RplClientThread: public QThread {
public:
   RplClientThread(ReConfigurator& configurator, ReLogger* logger = NULL);
   virtual ~RplClientThread();
private:
   // No copy constructor: no implementation!
   RplClientThread(const RplClientThread& source);
   // Prohibits the assignment operator. Not implemented!
   RplClientThread& operator=(const RplClientThread& source);
public:
   /**
    * @brief Does the main task of the thread.
    *
    * Will be called from <code>QThread::run()</code>.
    * The implementations of this abstract method should be call <code>getPeer()</code>
    * to send and receive messages.
    */
   virtual void doIt() = 0;
   ReTCPPeer* getPeer() const;
   ReLogger* getLogger() const;
private:
   virtual void run();
protected:
   RplTcpClient* m_client;
   ReLogger* m_logger;
   ReConfigurator& m_configurator;
private:
   bool m_ownsLogger;
};

#endif // RETCPCLIENT_HPP
