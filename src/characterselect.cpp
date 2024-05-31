#include "characterselect.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QMenu>
#include <QStringBuilder>
#include <QTreeWidgetItem>
#include <QUiLoader>
#include <QVBoxLayout>

#include "aoapplication.h"
#include "debug_functions.h"
#include "file_functions.h"
#include "qtutil/qwidgetfinder.h"

const QString UI_FILE = "characterselect.ui";

namespace kal
{
CharacterSelect::CharacterSelect(AOApplication *app, QWidget *parent)
    : QWidget(parent)
    , m_app(app)
{
  QFile file(Options::getInstance().getUIAsset(UI_FILE));
  if (!file.open(QFile::ReadOnly))
  {
    qFatal("Failed to open file %s: %s", qPrintable(file.fileName()), qPrintable(file.errorString()));
    return;
  }

  QUiLoader loader;
  QWidget *widget = loader.load(&file, this);
  if (!widget)
  {
    qFatal("Failed to load UI file %s", qPrintable(file.fileName()));
    return;
  }
  QLayout *layout = new QVBoxLayout{this};
  layout->addWidget(widget);

  qtutil::QWidgetFinder finder(widget);

  finder.findChild(ui_exit_server, "exit_server");
  finder.findChild(ui_spectate, "spectate");
  finder.findChild(ui_hide_taken, "hide_taken");
  finder.findChild(ui_filter, "filter");
  finder.findChild(ui_slot_viewport, "slot_viewport");
  finder.findChild(ui_tree, "tree");

  ui_slot_layout = new CharacterSelectLayout(ui_slot_viewport);

  connect(ui_exit_server, &QPushButton::pressed, this, &CharacterSelect::exitServer);
  connect(ui_spectate, &QPushButton::pressed, this, &CharacterSelect::spectatorSelected);

  connect(ui_tree, &QTreeWidget::itemDoubleClicked, this, &CharacterSelect::tryNotifySelectionFromItem);
  connect(ui_tree, &QTreeWidget::customContextMenuRequested, this, &CharacterSelect::tryDisplayContextMenuFromItem);

  connect(ui_filter, &QLineEdit::textChanged, this, &CharacterSelect::filterCharacterList);
  connect(ui_hide_taken, &QCheckBox::clicked, this, &CharacterSelect::filterCharacterList);

  m_button_mapper = new QSignalMapper(this);

  connect(m_button_mapper, &QSignalMapper::mappedInt, this, &CharacterSelect::tryNotifySelection);
}

CharacterSelect::~CharacterSelect()
{}

void CharacterSelect::setCharacterList(const QStringList &characterList)
{
  ui_tree->clear();

  for (const CharacterSlotDetails &i_slot : qAsConst(m_slot_list))
  {
    m_button_mapper->removeMappings(i_slot.button);
    i_slot.button->deleteLater();
  }

  const int size = characterList.size();
  m_slot_list.resize(size);
  m_slot_list.squeeze();
  QHash<QString, QTreeWidgetItem *> category_map;
  for (int characterId = 0; characterId < size; characterId++)
  {
    CharacterSlotDetails &slot = m_slot_list[characterId];
    slot.name = characterList.at(characterId);
    slot.path = m_app->get_real_path(VPath("characters/" + slot.name + "/"));
    slot.ini_path = m_app->get_real_path(m_app->get_character_path(slot.name, "char.ini"));
    slot.exists = dir_exists(slot.path) && file_exists(slot.ini_path);

    // button
    slot.button = new CharacterSlotButton(slot.name, m_app, ui_slot_viewport);
    slot.button->setContextMenuPolicy(Qt::CustomContextMenu);
    m_button_mapper->setMapping(slot.button, characterId);
    ui_slot_layout->addWidget(slot.button);

    connect(slot.button, &CharacterSlotButton::clicked, m_button_mapper, QOverload<>::of(&QSignalMapper::map));
    connect(slot.button, &CharacterSlotButton::customContextMenuRequested, this, [this, characterId, slot](const QPoint &position) { tryDisplayContextMenu(slot.button->mapToGlobal(position), characterId); });

    // item
    slot.item = new QTreeWidgetItem(true);
    slot.item->setText(0, slot.name);
    slot.item->setIcon(0, QIcon(m_app->get_image_suffix(m_app->get_character_path(slot.name, "char_icon"), true)));

    slot.category = m_app->get_category(slot.name);
    if (!slot.category.isEmpty())
    {
      QTreeWidgetItem *category_item;
      if (category_map.contains(slot.category))
      {
        category_item = category_map[slot.category];
      }
      else
      {
        category_item = new QTreeWidgetItem;
        category_item->setText(0, slot.category);
        category_item->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicatorWhenChildless);
        category_map[slot.category] = category_item;
        ui_tree->addTopLevelItem(category_item);
      }

      category_item->addChild(slot.item);
    }
    else
    {
      ui_tree->addTopLevelItem(slot.item);
    }
  }
  ui_tree->expandAll();

  filterCharacterList();
}

void CharacterSelect::setTakenCharacterList(const QList<bool> &takenCharacterList)
{
  for (int i = 0; i < m_slot_list.size(); i++)
  {
    m_slot_list[i].is_taken = takenCharacterList.at(i);
  }

  filterCharacterList();
}

void CharacterSelect::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
}

void CharacterSelect::filterCharacterList()
{
  QString filter_string = ui_filter->text();
  bool filter = !filter_string.isEmpty();
  bool hide_taken = ui_hide_taken->isChecked();

  for (int i = 0; i < m_slot_list.size(); ++i)
  {
    const CharacterSlotDetails &slot = m_slot_list.at(i);

    bool is_visible = true;

    if (filter && !slot.name.contains(filter_string, Qt::CaseInsensitive) && !slot.category.contains(filter_string, Qt::CaseInsensitive))
    {
      is_visible = false;
    }

    if (hide_taken && slot.is_taken)
    {
      is_visible = false;
    }

    slot.button->setHidden(!is_visible);
    slot.item->setHidden(!is_visible);
  }
}

void CharacterSelect::tryNotifySelection(int characterId)
{
  const CharacterSlotDetails &slot = m_slot_list.at(characterId);
  if (slot.exists)
  {
    Q_EMIT characterSelected(characterId);
  }
  else
  {
    call_error("Cannot select character " + slot.name + ".\n\nMissing content. Please make sure the character is installed correctly.");
  }
}

void CharacterSelect::tryNotifySelectionFromItem(QTreeWidgetItem *item)
{
  if (item && item->type())
  {
    tryNotifySelection(item->data(0, Qt::UserRole).toInt());
  }
}

void CharacterSelect::tryDisplayContextMenu(const QPoint &position, int characterId)
{
  const CharacterSlotDetails &slot = m_slot_list.at(characterId);

  QMenu *menu = new QMenu{this};
  menu->setAttribute(Qt::WA_DeleteOnClose);
  menu->setEnabled(slot.exists);

  menu->addAction(tr("Select %1").arg(slot.name), this, [this, characterId] { Q_EMIT characterSelected(characterId); })->setDisabled(slot.is_taken);
  menu->addAction(tr("Open folder"), this, [slot] { QDesktopServices::openUrl(QUrl::fromLocalFile(slot.path)); });

  menu->popup(position);
}

void CharacterSelect::tryDisplayContextMenuFromItem(const QPoint &position)
{
  QTreeWidgetItem *item = ui_tree->itemAt(position);
  if (item && item->type())
  {
    tryDisplayContextMenu(ui_tree->mapToGlobal(position), item->data(0, Qt::UserRole).toInt());
  }
}
} // namespace kal
