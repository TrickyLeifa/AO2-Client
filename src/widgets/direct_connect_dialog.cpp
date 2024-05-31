#include "direct_connect_dialog.h"

#include "debug_functions.h"
#include "networkmanager.h"
#include "options.h"
#include "qtutil/qwidgetfinder.h"

#include <QStringBuilder>
#include <QUiLoader>
#include <QVBoxLayout>

const QString DirectConnectDialog::UI_FILE_PATH = "direct_connect_dialog.ui";
const QRegularExpression DirectConnectDialog::SCHEME_PATTERN{"^\\w+://.+$"};
const int DirectConnectDialog::CONNECT_TIMEOUT = 5 * 1000;

DirectConnectDialog::DirectConnectDialog(NetworkManager *netManager, QWidget *parent)
    : QDialog(parent)
    , net_manager(netManager)
{
  QFile file(Options::getInstance().getUIAsset(UI_FILE_PATH));
  if (!file.open(QFile::ReadOnly))
  {
    qFatal("Failed to open file %s: %s", qPrintable(file.fileName()), qPrintable(file.errorString()));
    return;
  }

  QUiLoader loader(this);
  ui_widget = loader.load(&file, this);

  auto l_layout = new QVBoxLayout(this);
  l_layout->addWidget(ui_widget);

  qtutil::QWidgetFinder finder(ui_widget);

  finder.findChild(ui_direct_hostname_edit, "direct_hostname_edit");
  finder.findChild(ui_direct_connection_status_lbl, "direct_connection_status_lbl");
  finder.findChild(ui_direct_connect_button, "direct_connect_button");
  finder.findChild(ui_direct_cancel_button, "direct_cancel_button");

  m_connect_timeout.setSingleShot(true);

  connect(ui_direct_cancel_button, &QPushButton::pressed, this, &DirectConnectDialog::close);

  connect(ui_direct_connect_button, &QPushButton::pressed, this, &DirectConnectDialog::onConnectPressed);

  connect(net_manager, &NetworkManager::server_connected, this, &DirectConnectDialog::onServerConnected);

  connect(&m_connect_timeout, &QTimer::timeout, this, &DirectConnectDialog::onConnectTimeout);
}

void DirectConnectDialog::onConnectPressed()
{
  QString l_hostname = ui_direct_hostname_edit->text();
  if (!SCHEME_PATTERN.match(l_hostname).hasMatch())
  {
    l_hostname = "tcp://" % l_hostname;
  }
  QUrl l_url(l_hostname);
  if (!l_url.isValid())
  {
    call_error(tr("Invalid URL."));
    return;
  }
  if (!SERVER_CONNECTION_TYPE_STRING_MAP.contains(l_url.scheme()))
  {
    call_error(tr("Scheme not recognized. Must be either of the following: ") % QStringList::fromVector(SERVER_CONNECTION_TYPE_STRING_MAP.keys().toVector()).join(", "));
    return;
  }
  if (l_url.port() == -1)
  {
    call_error(tr("Invalid server port."));
    return;
  }
  ServerInfo l_server;
  l_server.socket_type = SERVER_CONNECTION_TYPE_STRING_MAP[l_url.scheme()];
  l_server.ip = l_url.host();
  l_server.port = l_url.port();
  l_server.name = "Direct Connection";

  net_manager->connect_to_server(l_server);
  ui_direct_connect_button->setEnabled(false);
  ui_direct_connection_status_lbl->setText("Connecting...");
  ui_direct_connection_status_lbl->setStyleSheet("color : rgb(0,64,156)");
  m_connect_timeout.start(CONNECT_TIMEOUT);
}

void DirectConnectDialog::onServerConnected()
{
  net_manager->join_to_server();
  ui_direct_connection_status_lbl->setText("Connected!");
  ui_direct_connection_status_lbl->setStyleSheet("color: rgb(0,128,0)");
  close();
}

void DirectConnectDialog::onConnectTimeout()
{
  ui_direct_connect_button->setEnabled(true);
  ui_direct_connection_status_lbl->setText("Connection Timeout!");
  ui_direct_connection_status_lbl->setStyleSheet("color: rgb(255,0,0)");
}
