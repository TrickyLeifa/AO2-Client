#include "netconnection.h"

#include "networkmanager.h"

#include <QNetworkRequest>
#include <QUrl>

NetConnection::NetConnection(QObject *parent)
    : QObject(parent)
    , m_socket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
    , m_last_state(QAbstractSocket::UnconnectedState)
{
  connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &NetConnection::onError);
  connect(m_socket, &QWebSocket::stateChanged, this, &NetConnection::onStateChanged);
  connect(m_socket, &QWebSocket::textMessageReceived, this, &NetConnection::onTextMessageReceived);
}

NetConnection::~NetConnection()
{
  m_socket->disconnect(this);
  disconnectFromServer();
}

bool NetConnection::isConnected()
{
  return m_last_state == QAbstractSocket::ConnectedState;
}

void NetConnection::connectToServer(ServerInfo &server)
{
  disconnectFromServer();

  QUrl url;
  url.setScheme("ws");
  url.setHost(server.ip);
  url.setPort(server.port);

  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("SpriteChat"));

  m_socket->open(req);
}

void NetConnection::disconnectFromServer()
{
  if (isConnected())
  {
    m_socket->close(QWebSocketProtocol::CloseCodeGoingAway);
  }
}

void NetConnection::sendPacket(AOPacket packet)
{
  m_socket->sendTextMessage(packet.toString(true));
}

void NetConnection::onError()
{
  Q_EMIT errorOccurred(m_socket->errorString());
}

void NetConnection::onStateChanged(QAbstractSocket::SocketState state)
{
  m_last_state = state;
  switch (state)
  {
  default:
    break;

  case QAbstractSocket::ConnectedState:
    Q_EMIT connectedToServer();
    break;

  case QAbstractSocket::UnconnectedState:
    Q_EMIT disconnectedFromServer();
    break;
  }
}

void NetConnection::onTextMessageReceived(QString message)
{
  if (!message.endsWith("#%"))
  {
    return;
  }
  message.chop(2);

  QStringList raw_content = message.split('#');
  const QString header = raw_content.takeFirst();
  for (QString &data : raw_content)
  {
    data = AOPacket::decode(data);
  }

  Q_EMIT receivedPacket(AOPacket(header, raw_content));
}
