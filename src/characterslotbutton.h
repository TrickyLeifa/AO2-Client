#pragma once

#include "aoapplication.h"

#include <QPushButton>

namespace kal
{
class CharacterSlotButton : public QPushButton
{
  Q_OBJECT

public:
  static constexpr int DEFAULT_HEIGHT = 60;
  static constexpr int DEFAULT_WIDTH = 60;

  explicit CharacterSlotButton(const QString &character, AOApplication *app, QWidget *parent = nullptr);
  virtual ~CharacterSlotButton();

  void displayTakenOverlay(bool enabled);

  QSize sizeHint() const override;

protected:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  void enterEvent(QEvent *e) override;
#else
  void enterEvent(QEnterEvent *e) override;
#endif
  void leaveEvent(QEvent *e) override;

private:
  AOApplication *ao_app;

  QString m_character;

  QWidget *ui_taken_overlay;
  QWidget *ui_selector_overlay;
};
} // namespace kal
