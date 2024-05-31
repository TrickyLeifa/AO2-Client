#include "characterslotbutton.h"

#include "file_functions.h"
#include "options.h"

namespace kal
{
CharacterSlotButton::CharacterSlotButton(const QString &character, AOApplication *app, QWidget *parent)
    : QPushButton(parent)
    , ao_app(app)
    , m_character(character)
{
  setFixedSize(sizeHint());

  ui_taken_overlay = new QWidget(this);
  ui_taken_overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
  ui_taken_overlay->setFixedSize(sizeHint());
  ui_taken_overlay->setStyleSheet("background-color: rgba(0, 0, 0, 64);");
  ui_taken_overlay->setVisible(false);

  ui_selector_overlay = new QWidget(this);
  ui_selector_overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
  ui_selector_overlay->setFixedSize(sizeHint());
  ui_selector_overlay->setVisible(false);

  QString selector_path = ao_app->get_image("char_selector", Options::getInstance().theme(), Options::getInstance().subTheme(), ao_app->default_theme, "", "", "", true);
  ui_selector_overlay->setStyleSheet("image: url(" + selector_path + ");");

  QString icon_path = ao_app->get_image_suffix(ao_app->get_character_path(m_character, "char_icon"), true);
  if (file_exists(icon_path))
  {
    setIcon(QIcon(icon_path));
    setIconSize(sizeHint());
  }
  else
  {
    setText(character);
  }
}

CharacterSlotButton::~CharacterSlotButton()
{}

void CharacterSlotButton::displayTakenOverlay(bool enabled)
{
  ui_taken_overlay->setVisible(enabled);
}

QSize CharacterSlotButton::sizeHint() const
{
  return QSize(DEFAULT_HEIGHT, DEFAULT_WIDTH);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void CharacterSlotButton::enterEvent(QEvent *e)
#else
void CharacterSlotButton::enterEvent(QEnterEvent *e)
#endif
{
  QPushButton::enterEvent(e);
  ui_selector_overlay->setVisible(true);
}

void CharacterSlotButton::leaveEvent(QEvent *e)
{
  ui_selector_overlay->setVisible(false);
  QPushButton::leaveEvent(e);
}
} // namespace kal
