
#include "aoapplication.h"

#include "courtroom.h"
#include "datatypes.h"
#include "lobby.h"
#include "networkmanager.h"

#include <QDebug>
#include <QDirIterator>
#include <QLibraryInfo>
#include <QPluginLoader>
#include <QResource>
#include <QTranslator>

int main(int argc, char *argv[])
{
  qSetMessagePattern("%{type}: %{if-category}%{category}: %{endif}%{message}");

  qRegisterMetaType<AOPacket>();

  QApplication app(argc, argv);

#ifdef ANDROID
  if (QtAndroid::checkPermission("android.permission.READ_EXTERNAL_STORAGE") == QtAndroid::PermissionResult::Denied)
  {
    QtAndroid::requestPermissionsSync({"android.permission.READ_EXTERNAL_STORAGE", "android.permission.WRITE_EXTERNAL_STORAGE"});
  }
#endif

  QApplication::addLibraryPath(QApplication::applicationDirPath() + "/lib");

  QStringList formats;
  {
    QStringList expected_formats{"WEBP", "APNG", "GIF"};
    for (const QByteArray &i_format : QImageReader::supportedImageFormats())
    {
      if (expected_formats.contains(i_format))
      {
        expected_formats.removeAll(i_format.toLower());
        formats.append(QString(".%1").arg(i_format.toLower()));
      }
    }

    if (!expected_formats.isEmpty())
    {
      call_error("Missing image formats: " + expected_formats.join(", ") + ".<br /><br /> Please make sure you have installed the application properly.");
    }
  }
  AOApplication main_app(formats);

  QApplication::setApplicationVersion(main_app.get_version_string());
  QApplication::setApplicationDisplayName(QObject::tr("Attorney Online %1").arg(QApplication::applicationVersion()));

  QResource::registerResource(main_app.get_asset("themes/" + Options::getInstance().theme() + ".rcc"));

  QFont main_font = app.font();
  main_app.default_font = main_font;

  QFont new_font = main_font;
  int new_font_size = main_app.default_font.pointSize() * Options::getInstance().themeScalingFactor();
  new_font.setPointSize(new_font_size);
  app.setFont(new_font);

  QDirIterator it(get_base_path() + "fonts", QDirIterator::Subdirectories);
  while (it.hasNext())
  {
    QFontDatabase::addApplicationFont(it.next());
  }

  QString p_language = Options::getInstance().language();
  if (p_language.trimmed().isEmpty())
  {
    p_language = QLocale::system().name();
  }

  QTranslator qtTranslator;
  qtTranslator.load("qt_" + p_language, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);

  QTranslator appTranslator;
  qDebug() << ":/resource/translations/ao_" + p_language;
  appTranslator.load("ao_" + p_language, ":/resource/translations/");
  app.installTranslator(&appTranslator);

  main_app.construct_lobby();
  main_app.net_manager->get_server_list();
  main_app.net_manager->send_heartbeat();
  main_app.w_lobby->show();

  return app.exec();
}
