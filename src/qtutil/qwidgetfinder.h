#pragma once

#include <QWidget>

namespace qtutil
{
class QWidgetFinder
{
public:
  QWidgetFinder(QWidget *parent);

  template <typename T>
  void findChild(T *&child, const QString &name, Qt::FindChildOptions options = Qt::FindChildrenRecursively)
  {
    child = m_root->findChild<T *>(name, options);
  }

private:
  QWidget *m_root;
};
} // namespace qtutil
