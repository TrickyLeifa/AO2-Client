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
    if (!child)
    {
      qFatal("Could not find child widget with name %s", qPrintable(name));
    }
  }

private:
  QWidget *m_root;
};
} // namespace qtutil
