#pragma once

#include "characterslotbutton.h"

#include <QLayout>
#include <QRect>
#include <QStyle>

namespace kal
{
class CharacterSelectLayout : public QLayout
{
public:
  explicit CharacterSelectLayout(QWidget *parent);
  virtual ~CharacterSelectLayout();

  void addItem(QLayoutItem *item) override;
  QLayoutItem *itemAt(int index) const override;
  QLayoutItem *takeAt(int index) override;
  int count() const override;

  QSize minimumSize() const override;
  void setGeometry(const QRect &rect) override;
  QSize sizeHint() const override;

  bool hasHeightForWidth() const override { return true; }
  int heightForWidth(int width) const override;

private:
  QList<QLayoutItem *> m_item_list;

  int calculateAndApplyLayout(const QRect &rect, bool onlyCalculate) const;
};
} // namespace kal
