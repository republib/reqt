/*
 * ReTCPPeer.hpp
 *
 * (Un)License: Public Domain
 * You can use and modify this file without any restriction.
 * Do what you want.
 * No warranties and disclaimer of any damages.
 * More info: http://unlicense.org
 * The latest sources: https://github.com/republib
 */
#ifndef RPLTCPPEER_HPP
#define RPLTCPPEER_HPP

#ifndef REBASE_HPP
#include "base/rebase.hpp"
#include "math/remath.hpp"
#include "net/renet.hpp"
#endif

class ReTCPPeer: public QObject {
   Q_OBJECT
public:
   enum {
      ///> standard behaviour
      FLAG_UNDEF,
      ///> if set: the size field is 4 byte (max. 4 GByte) instead of 2 byte (64kByte)
      FLAG_4_BYTE_SIZE = 1,
      ///> if set the data are compressed by the gzip algorithm
      FLAG_GZIP = 2,
      ///> if set the data are encrypted. In front of the size field a 4 byte salt field exists.
      ///> In this case all data behind the salt field are encrypted.
      FLAG_ENCRYPT = 4,
      ///> connection initialization of
   } flag_t;
public:
   static ReTCPPeer* createPeer(ReConfigurator& configurator, QThread* thread,
                                ReTerminator* terminator, ReLogger* logger = NULL);
public:
   ReTCPPeer(ReConfigurator& configurator, QThread* thread,
             ReTerminator* terminator, bool isServer, ReLogger* logger = NULL);
   virtual ~ReTCPPeer();
private:
   // No copy constructor: no implementation!
   ReTCPPeer(const ReTCPPeer& source);
   // No assignment operator: no implementation!
   ReTCPPeer& operator =(const ReTCPPeer& source);
public:
   virtual bool send(qint8 flags, const char* command, const QByteArray& data);
   virtual bool receive(QByteArray& command, QByteArray& data);
   virtual bool sendAndReceive(uint8_t flags, char command[4],
                               QByteArray* data, QByteArray& answer, QByteArray& answerData);
   void setSocket(QAbstractSocket* socket);
   QAbstractSocket* getSocket() const;
   QByteArray getPeerAddress();
   void handleError(QTcpSocket::SocketError socketError);
   int getPort();
   QByteArray getIp();
   void setAddress(const char* ip, int port);
private:
   QByteArray readBytes(int bytes, time_t maxTime, int& loops);

public slots:
   void readTcpData();

private:
   QAbstractSocket* m_socket;
   // <ip>:<port>
   QByteArray m_address;
   ReLogger* m_logger;
   QByteArray m_received;
   int m_expected;
   QThread* m_thread;
   // Only used for salt generation:
   ReRandom m_random;
   ///> maximum allowed time (in seconds) for sending/receiving one info unit
   int m_timeout;
   ///> for controlled termination
   ReTerminator* m_terminator;
   ReConfigurator& m_configurator;
   bool m_isServer;
   QMutex m_dataLocker;
   QWaitCondition m_waitForData;
};

#endif // RPLTCPPEER_HPP
