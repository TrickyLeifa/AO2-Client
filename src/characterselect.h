#pragma once

#include "aoapplication.h"
#include "characterselectlayout.h"
#include "characterslotbutton.h"
#include "datatypes.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalMapper>
#include <QTreeWidget>
#include <QWidget>

namespace kal
{
class CharacterSelect : public QWidget
{
  Q_OBJECT

public:
  explicit CharacterSelect(AOApplication *app, QWidget *parent = nullptr);
  virtual ~CharacterSelect();

  void setCharacterList(const QStringList &characterList);
  void setTakenCharacterList(const QList<bool> &takenCharacterList);

Q_SIGNALS:
  void exitServer();
  void characterSelected(int characterId);
  void spectatorSelected();

protected:
  void resizeEvent(QResizeEvent *event) override;

private:
  AOApplication *m_app = nullptr;

  class CharacterSlotDetails
  {
  public:
    QString name;
    QString category;
    QString path;
    QString ini_path;
    bool exists = false;
    bool is_taken = false;
    CharacterSlotButton *button;
    QTreeWidgetItem *item;
  };
  QVector<CharacterSlotDetails> m_slot_list;
  QSignalMapper *m_button_mapper = nullptr;

  QPushButton *ui_exit_server = nullptr;
  QPushButton *ui_spectate = nullptr;
  QWidget *ui_slot_viewport = nullptr;
  CharacterSelectLayout *ui_slot_layout = nullptr;
  QTreeWidget *ui_tree = nullptr;
  QLineEdit *ui_filter = nullptr;
  QCheckBox *ui_hide_taken = nullptr;

  void tryNotifySelection(int characterId);
  void tryDisplayContextMenu(const QPoint &position, int characterId);

private Q_SLOTS:
  void filterCharacterList();

  void tryNotifySelectionFromItem(QTreeWidgetItem *item);
  void tryDisplayContextMenuFromItem(const QPoint &position);
};
} // namespace kal
