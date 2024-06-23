#pragma once

#include "aopacket.h"
#include "datatypes.h"

#include <QWebSocket>

class NetConnection : public QObject
{
  Q_OBJECT

public:
  explicit NetConnection(QObject *parent = nullptr);
  virtual ~NetConnection();

  bool isConnected();

  void connectToServer(ServerInfo &server);
  void disconnectFromServer();

  void sendPacket(AOPacket packet);

Q_SIGNALS:
  void connectedToServer();
  void disconnectedFromServer();
  void errorOccurred(QString error);

  void receivedPacket(AOPacket packet);

private:
  QWebSocket *m_socket;
  QAbstractSocket::SocketState m_last_state;

private Q_SLOTS:
  void onError();
  void onStateChanged(QAbstractSocket::SocketState state);
  void onTextMessageReceived(QString message);
};
