#include "characterselectlayout.h"

namespace kal
{
CharacterSelectLayout::CharacterSelectLayout(QWidget *parent)
    : QLayout(parent)
{}

CharacterSelectLayout::~CharacterSelectLayout()
{
  for (QLayoutItem *i_item : qAsConst(m_item_list))
  {
    delete i_item;
  }
}

void CharacterSelectLayout::addItem(QLayoutItem *item)
{
  if (m_item_list.contains(item))
  {
    return;
  }
  m_item_list.append(item);
}

QLayoutItem *CharacterSelectLayout::itemAt(int index) const
{
  if (index < 0 || index >= m_item_list.size())
  {
    return nullptr;
  }
  return m_item_list.at(index);
}

QLayoutItem *CharacterSelectLayout::takeAt(int index)
{
  if (index < 0 || index >= m_item_list.size())
  {
    return nullptr;
  }
  return m_item_list.takeAt(index);
}

int CharacterSelectLayout::count() const
{
  return m_item_list.size();
}

QSize CharacterSelectLayout::minimumSize() const
{
  int left, top, right, bottom;
  getContentsMargins(&left, &top, &right, &bottom);
  return QSize(CharacterSlotButton::DEFAULT_WIDTH + left + right, CharacterSlotButton::DEFAULT_HEIGHT + top + bottom);
}

void CharacterSelectLayout::setGeometry(const QRect &rect)
{
  QLayout::setGeometry(rect);
  calculateAndApplyLayout(rect, false);
}

QSize CharacterSelectLayout::sizeHint() const
{
  return minimumSize();
}

int CharacterSelectLayout::heightForWidth(int width) const
{
  return calculateAndApplyLayout(QRect(0, 0, width, 0), true);
}

int CharacterSelectLayout::calculateAndApplyLayout(const QRect &rect, bool onlyCalculate) const
{
  int left, top, right, bottom;
  getContentsMargins(&left, &top, &right, &bottom);
  QRect area = rect.adjusted(+left, +top, -right, -bottom);
  const int x_offset = area.x();
  const int y_offset = area.y();
  const int width = area.width();
  const int spacing = this->spacing();

  int x = 0;
  int y = 0;
  const int button_width = CharacterSlotButton::DEFAULT_WIDTH;
  const int button_height = CharacterSlotButton::DEFAULT_HEIGHT;
  for (QLayoutItem *item : qAsConst(m_item_list))
  {
    CharacterSlotButton *widget = qobject_cast<CharacterSlotButton *>(item->widget());
    if (!widget || widget->isHidden())
    {
      continue;
    }

    if (x > 0 && (x + button_width) > width)
    {
      x = 0;
      y += button_height + spacing;
    }

    if (!onlyCalculate)
    {
      item->setGeometry(QRect(QPoint(x + x_offset, y + y_offset), item->sizeHint()));
    }

    x += button_width + spacing;
  }

  return y + y_offset + button_height + spacing;
}
} // namespace kal
