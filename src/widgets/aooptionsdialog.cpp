#include "aooptionsdialog.h"

#include "QDesktopServices"
#include "aoapplication.h"
#include "file_functions.h"
#include "networkmanager.h"
#include "options.h"
#include "qtutil/qwidgetfinder.h"

#include <bass.h>

#include <QCollator>
#include <QGroupBox>
#include <QResource>
#include <QUiLoader>
#include <QVBoxLayout>

AOOptionsDialog::AOOptionsDialog(AOApplication *p_ao_app, QWidget *parent)
    : QDialog(parent)
    , ao_app(p_ao_app)
{
  setupUI();
}

void AOOptionsDialog::populateAudioDevices()
{
  ui_audio_device_combobox->clear();
  if (needsDefaultAudioDevice())
  {
    ui_audio_device_combobox->addItem("default", "default");
  }

  BASS_DEVICEINFO info;
  for (int a = 0; BASS_GetDeviceInfo(a, &info); a++)
  {
    ui_audio_device_combobox->addItem(info.name, info.name);
  }
}

template <>
void AOOptionsDialog::setWidgetData(QCheckBox *widget, const bool &value)
{
  widget->setChecked(value);
}

template <>
bool AOOptionsDialog::widgetData(QCheckBox *widget) const
{
  return widget->isChecked();
}

template <>
void AOOptionsDialog::setWidgetData(QLineEdit *widget, const QString &value)
{
  widget->setText(value);
}

template <>
QString AOOptionsDialog::widgetData(QLineEdit *widget) const
{
  return widget->text();
}

template <>
void AOOptionsDialog::setWidgetData(QLineEdit *widget, const uint16_t &value)
{
  widget->setText(QString::number(value));
}

template <>
uint16_t AOOptionsDialog::widgetData(QLineEdit *widget) const
{
  return widget->text().toUShort();
}

template <>
void AOOptionsDialog::setWidgetData(QPlainTextEdit *widget, const QStringList &value)
{
  widget->setPlainText(value.join('\n'));
}

template <>
QStringList AOOptionsDialog::widgetData(QPlainTextEdit *widget) const
{
  return widget->toPlainText().trimmed().split('\n');
}

template <>
void AOOptionsDialog::setWidgetData(QSpinBox *widget, const int &value)
{
  widget->setValue(value);
}

template <>
int AOOptionsDialog::widgetData(QSpinBox *widget) const
{
  return widget->value();
}

template <>
void AOOptionsDialog::setWidgetData(QComboBox *widget, const QString &value)
{
  for (auto i = 0; i < widget->count(); i++)
  {
    if (widget->itemData(i).toString() == value)
    {
      widget->setCurrentIndex(i);
      return;
    }
  }
  qWarning() << "value" << value << "not found for widget" << widget->objectName();
}

template <>
QString AOOptionsDialog::widgetData(QComboBox *widget) const
{
  return widget->currentData().toString();
}

template <>
void AOOptionsDialog::setWidgetData(QGroupBox *widget, const bool &value)
{
  widget->setChecked(value);
}

template <>
bool AOOptionsDialog::widgetData(QGroupBox *widget) const
{
  return widget->isChecked();
}

template <>
void AOOptionsDialog::setWidgetData(QListWidget *widget, const QStringList &value)
{
  widget->addItems(value);
}

template <>
QStringList AOOptionsDialog::widgetData(QListWidget *widget) const
{
  QStringList paths;
  for (auto i = 1; i < widget->count(); i++)
  {
    paths.append(widget->item(i)->text());
  }
  return paths;
}

template <typename T, typename V>
void AOOptionsDialog::registerOption(const QString &widgetName, V (Options::*getter)() const, void (Options::*setter)(V))
{
  auto *widget = findChild<T *>(widgetName);
  if (!widget)
  {
    qWarning() << "could not find widget" << widgetName;
    return;
  }

  OptionEntry entry;
  entry.load = [=] {
    setWidgetData<T, V>(widget, (Options::getInstance().*getter)());
  };
  entry.save = [=] {
    (Options::getInstance().*setter)(widgetData<T, V>(widget));
  };

  optionEntries.append(entry);
}

void AOOptionsDialog::updateValues()
{
  QSet<QString> themes;
  QStringList bases = Options::getInstance().mountPaths();
  bases.push_front(get_base_path());

  for (const QString &base : bases)
  {
    QStringList l_themes = QDir(base + "/themes").entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // Resorts list to match numeric sorting found in Windows.
    QCollator l_sorting;
    l_sorting.setNumericMode(true);
    std::sort(l_themes.begin(), l_themes.end(), l_sorting);

    for (const QString &l_theme : qAsConst(l_themes))
    {
      if (!themes.contains(l_theme))
      {
        ui_theme_combobox->addItem(l_theme, l_theme);
        themes.insert(l_theme);
      }
    }
  }

  QStringList l_subthemes = QDir(ao_app->get_real_path(ao_app->get_theme_path(""))).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  for (const QString &l_subtheme : qAsConst(l_subthemes))
  {
    if (l_subtheme.toLower() != "server" && l_subtheme.toLower() != "default" && l_subtheme.toLower() != "effects" && l_subtheme.toLower() != "misc")
    {
      ui_subtheme_combobox->addItem(l_subtheme, l_subtheme);
    }
  }

  ao_app->net_manager->request_document(MSDocumentType::PrivacyPolicy, [this](QString document) {
    if (document.isEmpty())
    {
      document = tr("Couldn't get the privacy policy.");
    }
    ui_privacy_policy->setHtml(document);
  });

  for (const OptionEntry &entry : qAsConst(optionEntries))
  {
    entry.load();
  }
}

void AOOptionsDialog::savePressed()
{
  bool l_reload_theme_required = (ui_theme_combobox->currentText() != Options::getInstance().theme()) || (ui_theme_scaling_factor_sb->value() != Options::getInstance().themeScalingFactor());
  for (const OptionEntry &entry : qAsConst(optionEntries))
  {
    entry.save();
  }

  if (l_reload_theme_required)
  {
    Q_EMIT reloadThemeRequest();
  }
  close();
}

void AOOptionsDialog::discardPressed()
{
  close();
}

void AOOptionsDialog::buttonClicked(QAbstractButton *button)
{
  if (ui_settings_buttons->buttonRole(button) == QDialogButtonBox::ResetRole)
  {
    if (QMessageBox::question(this, "", "Restore default settings?\nThis can't be undone!", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
      // Destructive operation.
      Options::getInstance().clearConfig();
      updateValues();
    }
  }
}

void AOOptionsDialog::onReloadThemeClicked()
{
  Options::getInstance().setTheme(ui_theme_combobox->currentText());
  Options::getInstance().setSettingsSubTheme(ui_subtheme_combobox->currentText());
  Options::getInstance().setAnimatedThemeEnabled(ui_animated_theme_cb->isChecked());
  Q_EMIT reloadThemeRequest();
  delete layout();
  delete ui_settings_widget;
  optionEntries.clear();
  setupUI();
}

void AOOptionsDialog::themeChanged(int i)
{
  ui_subtheme_combobox->clear();
  // Fill the combobox with the names of the themes.
  ui_subtheme_combobox->addItem("server", "server");
  ui_subtheme_combobox->addItem("default", "server");

  QStringList l_subthemes = QDir(ao_app->get_real_path(ao_app->get_theme_path("", ui_theme_combobox->itemText(i)))).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

  for (const QString &l_subthemes : qAsConst(l_subthemes))
  {
    if (l_subthemes.toLower() != "server" && l_subthemes.toLower() != "default" && l_subthemes.toLower() != "effects" && l_subthemes.toLower() != "misc")
    {
      ui_subtheme_combobox->addItem(l_subthemes, l_subthemes);
    }
  }

  QString l_ressource_name = Options::getInstance().theme() + ".rcc";
  QString l_resource = ao_app->get_asset("themes/" + ui_theme_combobox->currentText() + ".rcc");
  if (l_resource.isEmpty())
  {
    QResource::unregisterResource(ao_app->get_asset("themes/" + l_ressource_name));
    qDebug() << "Unable to locate ressource file" << l_ressource_name;
    return;
  }
  QResource::registerResource(l_resource);
}

void AOOptionsDialog::setupUI()
{
  QFile file(Options::getInstance().getUIAsset("options_dialog.ui"));
  if (!file.open(QFile::ReadOnly))
  {
    qFatal("Failed to open file %s: %s", qPrintable(file.fileName()), qPrintable(file.errorString()));
    return;
  }

  QUiLoader loader;
  ui_settings_widget = loader.load(&file, this);

  qtutil::QWidgetFinder finder(ui_settings_widget);

  auto layout = new QVBoxLayout(this);
  layout->addWidget(ui_settings_widget);

  // General dialog element.
  finder.findChild(ui_settings_buttons, "settings_buttons");

  connect(ui_settings_buttons, &QDialogButtonBox::accepted, this, &AOOptionsDialog::savePressed);
  connect(ui_settings_buttons, &QDialogButtonBox::rejected, this, &AOOptionsDialog::discardPressed);
  connect(ui_settings_buttons, &QDialogButtonBox::clicked, this, &AOOptionsDialog::buttonClicked);

  // Gameplay Tab
  finder.findChild(ui_theme_combobox, "theme_combobox");
  connect(ui_theme_combobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AOOptionsDialog::themeChanged);

  registerOption<QComboBox, QString>("theme_combobox", &Options::theme, &Options::setTheme);

  finder.findChild(ui_subtheme_combobox, "subtheme_combobox");
  registerOption<QComboBox, QString>("subtheme_combobox", &Options::settingsSubTheme, &Options::setSettingsSubTheme);

  finder.findChild(ui_theme_reload_button, "theme_reload_button");
  connect(ui_theme_reload_button, &QPushButton::clicked, this, &::AOOptionsDialog::onReloadThemeClicked);

  finder.findChild(ui_theme_folder_button, "theme_folder_button");
  connect(ui_theme_folder_button, &QPushButton::clicked, this, [=] {
    QString p_path = ao_app->get_real_path(ao_app->get_theme_path("", ui_theme_combobox->itemText(ui_theme_combobox->currentIndex())));
    if (!dir_exists(p_path))
    {
      return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(p_path));
  });

  finder.findChild(ui_theme_scaling_factor_sb, "theme_scaling_factor_sb");
  finder.findChild(ui_animated_theme_cb, "animated_theme_cb");
  finder.findChild(ui_stay_time_spinbox, "stay_time_spinbox");
  finder.findChild(ui_instant_objection_cb, "instant_objection_cb");
  finder.findChild(ui_text_crawl_spinbox, "text_crawl_spinbox");
  finder.findChild(ui_chat_ratelimit_spinbox, "chat_ratelimit_spinbox");
  finder.findChild(ui_username_textbox, "username_textbox");
  finder.findChild(ui_showname_cb, "showname_cb");
  finder.findChild(ui_default_showname_textbox, "default_showname_textbox");
  finder.findChild(ui_ms_textbox, "ms_textbox");
  finder.findChild(ui_discord_cb, "discord_cb");
  finder.findChild(ui_language_combobox, "language_combobox");
  finder.findChild(ui_scaling_combobox, "scaling_combobox");
  finder.findChild(ui_shake_cb, "shake_cb");
  finder.findChild(ui_effects_cb, "effects_cb");
  finder.findChild(ui_framenetwork_cb, "framenetwork_cb");
  finder.findChild(ui_colorlog_cb, "colorlog_cb");
  finder.findChild(ui_stickysounds_cb, "stickysounds_cb");
  finder.findChild(ui_stickyeffects_cb, "stickyeffects_cb");
  finder.findChild(ui_stickypres_cb, "stickypres_cb");
  finder.findChild(ui_customchat_cb, "customchat_cb");
  finder.findChild(ui_sticker_cb, "sticker_cb");
  finder.findChild(ui_continuous_cb, "continuous_cb");
  finder.findChild(ui_category_stop_cb, "category_stop_cb");
  finder.findChild(ui_sfx_on_idle_cb, "sfx_on_idle_cb");
  finder.findChild(ui_evidence_double_click_cb, "evidence_double_click_cb");
  finder.findChild(ui_slides_cb, "slides_cb");

  registerOption<QSpinBox, int>("theme_scaling_factor_sb", &Options::themeScalingFactor, &Options::setThemeScalingFactor);
  registerOption<QCheckBox, bool>("animated_theme_cb", &Options::animatedThemeEnabled, &Options::setAnimatedThemeEnabled);
  registerOption<QSpinBox, int>("stay_time_spinbox", &Options::textStayTime, &Options::setTextStayTime);
  registerOption<QCheckBox, bool>("instant_objection_cb", &Options::objectionSkipQueueEnabled, &Options::setObjectionSkipQueueEnabled);
  registerOption<QSpinBox, int>("text_crawl_spinbox", &Options::textCrawlSpeed, &Options::setTextCrawlSpeed);
  registerOption<QSpinBox, int>("chat_ratelimit_spinbox", &Options::chatRateLimit, &Options::setChatRateLimit);
  registerOption<QLineEdit, QString>("username_textbox", &Options::username, &Options::setUsername);
  registerOption<QCheckBox, bool>("showname_cb", &Options::customShownameEnabled, &Options::setCustomShownameEnabled);
  registerOption<QLineEdit, QString>("default_showname_textbox", &Options::shownameOnJoin, &Options::setShownameOnJoin);
  registerOption<QLineEdit, QString>("ms_textbox", &Options::alternativeMasterserver, &Options::setAlternativeMasterserver);
  registerOption<QCheckBox, bool>("discord_cb", &Options::discordEnabled, &Options::setDiscordEnabled);
  registerOption<QComboBox, QString>("language_combobox", &Options::language, &Options::setLanguage);

  ui_language_combobox->addItem("English", "en");
  ui_language_combobox->addItem("Deutsch", "de");
  ui_language_combobox->addItem("Español", "es");
  ui_language_combobox->addItem("Português", "pt");
  ui_language_combobox->addItem("Polski", "pl");
  ui_language_combobox->addItem("日本語", "jp");
  ui_language_combobox->addItem("Русский", "ru");

  registerOption<QComboBox, QString>("scaling_combobox", &Options::defaultScalingMode, &Options::setDefaultScalingMode);

  // Populate scaling dropdown. This is necessary as we need the user data
  // embeeded into the entry.
  ui_scaling_combobox->addItem(tr("Pixel"), "fast");
  ui_scaling_combobox->addItem(tr("Smooth"), "smooth");

  registerOption<QCheckBox, bool>("shake_cb", &Options::shakeEnabled, &Options::setShakeEnabled);
  registerOption<QCheckBox, bool>("effects_cb", &Options::effectsEnabled, &Options::setEffectsEnabled);
  registerOption<QCheckBox, bool>("framenetwork_cb", &Options::networkedFrameSfxEnabled, &Options::setNetworkedFrameSfxEnabled);
  registerOption<QCheckBox, bool>("colorlog_cb", &Options::colorLogEnabled, &Options::setColorLogEnabled);
  registerOption<QCheckBox, bool>("stickysounds_cb", &Options::clearSoundsDropdownOnPlayEnabled, &Options::setClearSoundsDropdownOnPlayEnabled);
  registerOption<QCheckBox, bool>("stickyeffects_cb", &Options::clearEffectsDropdownOnPlayEnabled, &Options::setClearEffectsDropdownOnPlayEnabled);
  registerOption<QCheckBox, bool>("stickypres_cb", &Options::clearPreOnPlayEnabled, &Options::setClearPreOnPlayEnabled);
  registerOption<QCheckBox, bool>("customchat_cb", &Options::customChatboxEnabled, &Options::setCustomChatboxEnabled);
  registerOption<QCheckBox, bool>("sticker_cb", &Options::characterStickerEnabled, &Options::setCharacterStickerEnabled);
  registerOption<QCheckBox, bool>("continuous_cb", &Options::continuousPlaybackEnabled, &Options::setContinuousPlaybackEnabled);
  registerOption<QCheckBox, bool>("category_stop_cb", &Options::stopMusicOnCategoryEnabled, &Options::setStopMusicOnCategoryEnabled);
  registerOption<QCheckBox, bool>("sfx_on_idle_cb", &Options::playSelectedSFXOnIdle, &Options::setPlaySelectedSFXOnIdle);
  registerOption<QCheckBox, bool>("evidence_double_click_cb", &Options::evidenceDoubleClickEdit, &Options::setEvidenceDoubleClickEdit);
  registerOption<QCheckBox, bool>("slides_cb", &Options::slidesEnabled, &Options::setSlidesEnabled);

  // Callwords tab. This could just be a QLineEdit, but no, we decided to allow
  // people to put a billion entries in.
  finder.findChild(ui_callwords_textbox, "callwords_textbox");
  registerOption<QPlainTextEdit, QStringList>("callwords_textbox", &Options::callwords, &Options::setCallwords);

  // Audio tab.
  finder.findChild(ui_audio_device_combobox, "audio_device_combobox");
  populateAudioDevices();
  registerOption<QComboBox, QString>("audio_device_combobox", &Options::audioOutputDevice, &Options::setAudioOutputDevice);

  finder.findChild(ui_suppress_audio_spinbox, "suppress_audio_spinbox");
  finder.findChild(ui_bliprate_spinbox, "bliprate_spinbox");
  finder.findChild(ui_blank_blips_cb, "blank_blips_cb");
  finder.findChild(ui_loopsfx_cb, "loopsfx_cb");
  finder.findChild(ui_objectmusic_cb, "objectmusic_cb");
  finder.findChild(ui_disablestreams_cb, "disablestreams_cb");

  registerOption<QSpinBox, int>("suppress_audio_spinbox", &::Options::defaultSuppressAudio, &Options::setDefaultSupressedAudio);
  registerOption<QSpinBox, int>("bliprate_spinbox", &::Options::blipRate, &Options::setBlipRate);
  registerOption<QCheckBox, bool>("blank_blips_cb", &Options::blankBlip, &Options::setBlankBlip);
  registerOption<QCheckBox, bool>("loopsfx_cb", &Options::loopingSfx, &Options::setLoopingSfx);
  registerOption<QCheckBox, bool>("objectmusic_cb", &Options::objectionStopMusic, &Options::setObjectionStopMusic);
  registerOption<QCheckBox, bool>("disablestreams_cb", &Options::streamingEnabled, &Options::setStreamingEnabled);

  // Asset tab
  finder.findChild(ui_mount_list, "mount_list");
  auto *defaultMount = new QListWidgetItem(tr("%1 (default)").arg(get_base_path()));
  defaultMount->setFlags(Qt::ItemFlag::NoItemFlags);
  ui_mount_list->addItem(defaultMount);
  registerOption<QListWidget, QStringList>("mount_list", &Options::mountPaths, &Options::setMountPaths);

  finder.findChild(ui_mount_add, "mount_add");
  connect(ui_mount_add, &QPushButton::clicked, this, [this] {
    QString path = QFileDialog::getExistingDirectory(this, tr("Select a base folder"), QApplication::applicationDirPath(), QFileDialog::ShowDirsOnly);
    if (path.isEmpty())
    {
      return;
    }
    QDir dir(QApplication::applicationDirPath());
    QString relative = dir.relativeFilePath(path);
    if (!relative.contains("../"))
    {
      path = relative;
    }
    QListWidgetItem *dir_item = new QListWidgetItem(path);
    ui_mount_list->addItem(dir_item);
    ui_mount_list->setCurrentItem(dir_item);

    // quick hack to update buttons
    Q_EMIT ui_mount_list->itemSelectionChanged();
  });

  finder.findChild(ui_mount_remove, "mount_remove");
  connect(ui_mount_remove, &QPushButton::clicked, this, [this] {
    auto selected = ui_mount_list->selectedItems();
    if (selected.isEmpty())
    {
      return;
    }
    delete selected[0];
    Q_EMIT ui_mount_list->itemSelectionChanged();
    asset_cache_dirty = true;
  });

  finder.findChild(ui_mount_up, "mount_up");
  connect(ui_mount_up, &QPushButton::clicked, this, [this] {
    auto selected = ui_mount_list->selectedItems();
    if (selected.isEmpty())
    {
      return;
    }
    auto *item = selected[0];
    int row = ui_mount_list->row(item);
    ui_mount_list->takeItem(row);
    int new_row = qMax(1, row - 1);
    ui_mount_list->insertItem(new_row, item);
    ui_mount_list->setCurrentRow(new_row);
    asset_cache_dirty = true;
  });

  finder.findChild(ui_mount_down, "mount_down");
  connect(ui_mount_down, &QPushButton::clicked, this, [this] {
    auto selected = ui_mount_list->selectedItems();
    if (selected.isEmpty())
    {
      return;
    }
    auto *item = selected[0];
    int row = ui_mount_list->row(item);
    ui_mount_list->takeItem(row);
    int new_row = qMin(ui_mount_list->count() + 1, row + 1);
    ui_mount_list->insertItem(new_row, item);
    ui_mount_list->setCurrentRow(new_row);
    asset_cache_dirty = true;
  });

  finder.findChild(ui_mount_clear_cache, "mount_clear_cache");
  connect(ui_mount_clear_cache, &QPushButton::clicked, this, [this] {
    asset_cache_dirty = true;
    ui_mount_clear_cache->setEnabled(false);
  });

  connect(ui_mount_list, &QListWidget::itemSelectionChanged, this, [this] {
    auto selected_items = ui_mount_list->selectedItems();
    bool row_selected = !ui_mount_list->selectedItems().isEmpty();
    ui_mount_remove->setEnabled(row_selected);
    ui_mount_up->setEnabled(row_selected);
    ui_mount_down->setEnabled(row_selected);

    if (!row_selected)
    {
      return;
    }

    int row = ui_mount_list->row(selected_items[0]);
    if (row <= 1)
    {
      ui_mount_up->setEnabled(false);
    }
    if (row >= ui_mount_list->count() - 1)
    {
      ui_mount_down->setEnabled(false);
    }
  });

  // Logging tab
  finder.findChild(ui_downwards_cb, "downwards_cb");
  finder.findChild(ui_length_spinbox, "length_spinbox");
  finder.findChild(ui_log_newline_cb, "log_newline_cb");
  finder.findChild(ui_log_margin_spinbox, "log_margin_spinbox");
  finder.findChild(ui_log_timestamp_format_lbl, "log_timestamp_format_lbl");
  finder.findChild(ui_log_timestamp_format_combobox, "log_timestamp_format_combobox");

  registerOption<QCheckBox, bool>("downwards_cb", &Options::logDirectionDownwards, &Options::setLogDirectionDownwards);
  registerOption<QSpinBox, int>("length_spinbox", &Options::maxLogSize, &Options::setMaxLogSize);
  registerOption<QCheckBox, bool>("log_newline_cb", &Options::logNewline, &Options::setLogNewline);
  registerOption<QSpinBox, int>("log_margin_spinbox", &Options::logMargin, &Options::setLogMargin);

  finder.findChild(ui_log_timestamp_cb, "log_timestamp_cb");
  registerOption<QCheckBox, bool>("log_timestamp_cb", &Options::logTimestampEnabled, &Options::setLogTimestampEnabled);
  connect(ui_log_timestamp_cb, &QCheckBox::stateChanged, this, &::AOOptionsDialog::timestampCbChanged);
  ui_log_timestamp_format_lbl->setText(tr("Log timestamp format:\n") + QDateTime::currentDateTime().toString(Options::getInstance().logTimestampFormat()));

  finder.findChild(ui_log_timestamp_format_combobox, "log_timestamp_format_combobox");
  registerOption<QComboBox, QString>("log_timestamp_format_combobox", &Options::logTimestampFormat, &Options::setLogTimestampFormat);
  connect(ui_log_timestamp_format_combobox, &QComboBox::currentTextChanged, this, &::AOOptionsDialog::onTimestampFormatEdited);

  QString l_current_format = Options::getInstance().logTimestampFormat();

  ui_log_timestamp_format_combobox->setCurrentText(l_current_format);

  ui_log_timestamp_format_combobox->addItem(l_current_format, l_current_format);
  ui_log_timestamp_format_combobox->addItem("h:mm:ss AP", "h:mm:ss AP");
  ui_log_timestamp_format_combobox->addItem("hh:mm:ss", "hh:mm:ss");
  ui_log_timestamp_format_combobox->addItem("h:mm AP", "h:mm AP");
  ui_log_timestamp_format_combobox->addItem("hh:mm", "hh:mm");

  if (!Options::getInstance().logTimestampEnabled())
  {
    ui_log_timestamp_format_combobox->setDisabled(true);
  }

  finder.findChild(ui_log_ic_actions_cb, "log_ic_actions_cb");
  finder.findChild(ui_desync_logs_cb, "desync_logs_cb");
  finder.findChild(ui_log_text_cb, "log_text_cb");

  registerOption<QCheckBox, bool>("log_ic_actions_cb", &Options::logIcActions, &Options::setLogIcActions);
  registerOption<QCheckBox, bool>("desync_logs_cb", &Options::desynchronisedLogsEnabled, &Options::setDesynchronisedLogsEnabled);
  registerOption<QCheckBox, bool>("log_text_cb", &Options::logToTextFileEnabled, &Options::setLogToTextFileEnabled);
  registerOption<QCheckBox, bool>("log_demo_cb", &Options::logToDemoFileEnabled, &Options::setLogToDemoFileEnabled);

  // DSGVO/Privacy tab

  finder.findChild(ui_privacy_policy, "privacy_policy");
  ui_privacy_policy->setPlainText(tr("Getting privacy policy..."));
  finder.findChild(ui_privacy_optout_cb, "privacy_optout_cb");
  registerOption<QCheckBox, bool>("privacy_optout", &Options::playerCountOptout, &Options::setPlayerCountOptout);

  updateValues();
}

void AOOptionsDialog::onTimestampFormatEdited()
{
  const QString format = ui_log_timestamp_format_combobox->currentText();
  const int index = ui_log_timestamp_format_combobox->currentIndex();

  ui_log_timestamp_format_combobox->setItemText(index, format);
  ui_log_timestamp_format_combobox->setItemData(index, format);
  ui_log_timestamp_format_lbl->setText(tr("Log timestamp format:\n") + QDateTime::currentDateTime().toString(format));
}

void AOOptionsDialog::timestampCbChanged(int state)
{
  ui_log_timestamp_format_combobox->setDisabled(state == 0);
}

#if (defined(_WIN32) || defined(_WIN64))
bool AOOptionsDialog::needsDefaultAudioDevice()
{
  return true;
}
#elif (defined(LINUX) || defined(__linux__))
bool AOOptionsDialog::needsDefaultAudioDevice()
{
  return false;
}
#elif defined __APPLE__
bool AOOptionsDialog::needsDefaultAudioDevice()
{
  return true;
}
#else
#error This operating system is not supported.
#endif
