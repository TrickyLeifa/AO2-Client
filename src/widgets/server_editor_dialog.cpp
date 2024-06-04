#include "server_editor_dialog.h"

#include "datatypes.h"
#include "options.h"
#include "qtutil/qwidgetfinder.h"

#include <QDebug>
#include <QFile>
#include <QUiLoader>
#include <QVBoxLayout>

const QString ServerEditorDialog::UI_FILE_PATH = "favorite_server_dialog.ui";

ServerEditorDialog::ServerEditorDialog(QWidget *parent)
    : QDialog(parent)
{
  QFile file(Options::getInstance().getUIAsset(UI_FILE_PATH));
  if (!file.open(QFile::ReadOnly))
  {
    qFatal("Failed to open file %s: %s", qPrintable(file.fileName()), qPrintable(file.errorString()));
    return;
  }

  QUiLoader loader(this);
  ui_body = loader.load(&file, this);

  auto layout = new QVBoxLayout(this);
  layout->addWidget(ui_body);

  qtutil::QWidgetFinder finder(ui_body);

  finder.findChild(ui_name, "name");
  finder.findChild(ui_hostname, "hostname");
  finder.findChild(ui_port, "port");
  finder.findChild(ui_protocol, "protocol");
  finder.findChild(ui_description, "description");
  finder.findChild(ui_button_box, "button_box");
  finder.findChild(ui_legacy_edit, "legacy_edit");
  finder.findChild(ui_parse_legacy, "parse_legacy");

  connect(ui_parse_legacy, &QPushButton::released, this, &ServerEditorDialog::parseLegacyEntry);

  connect(ui_button_box, &QDialogButtonBox::accepted, this, &ServerEditorDialog::accept);
  connect(ui_button_box, &QDialogButtonBox::rejected, this, &ServerEditorDialog::reject);
}

ServerInfo ServerEditorDialog::currentServerInfo() const
{
  ServerInfo server;
  server.name = ui_name->text();
  server.ip = ui_hostname->text();
  server.port = ui_port->value();
  server.description = ui_description->toPlainText();
  server.socket_type = ServerConnectionType(ui_protocol->currentIndex());
  return server;
}

void ServerEditorDialog::loadServerInfo(ServerInfo server)
{
  ui_name->setText(server.name);
  ui_hostname->setText(server.ip);
  ui_port->setValue(server.port);
  ui_description->setPlainText(server.description);
  ui_protocol->setCurrentIndex(server.socket_type);
}

void ServerEditorDialog::parseLegacyEntry()
{
  QStringList entry = ui_legacy_edit->text().split(":");
  ServerInfo l_server_entry;
  if (entry.isEmpty())
  {
    qDebug() << "Legacy entry empty.";
    return;
  }

  int item_count = entry.size();
  if (item_count >= 3)
  {
    ui_hostname->setText(entry.at(0));
    ui_port->setValue(entry.at(1).toInt());
    ui_name->setText(entry.at(2));
    if (item_count >= 4)
    {
      if (entry.at(3) == "ws")
      {
        ui_protocol->setCurrentIndex(1);
      }
      else
      {
        ui_protocol->setCurrentIndex(0);
      }
    }
  }
}
