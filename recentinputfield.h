#ifndef RECENTINPUTFIELD_H
#define RECENTINPUTFIELD_H

#include <QWidget>
#include <QLineEdit>
#include <QToolButton>
#include <QMenu>
#include <QLabel>
#include <QHBoxLayout>
#include <QStringList>

class RecentInputField : public QWidget
{
    Q_OBJECT
public:
    explicit RecentInputField(QWidget *parent = nullptr);

    // 设置前置提示文字（比如 "类型 Type"）
    void setTitle(const QString &title);

    // 获取和设置输入框内容
    QString text() const;
    void setText(const QString &text);

    // 设置最大历史记录数量，默认 21
    void setMaxHistory(int maxItems);

    // 手动载入历史记录
    void setHistory(const QStringList &history);

signals:
    void textChanged(const QString &text);
    void returnPressed();

private:
    QLabel *titleLabel;
    QLineEdit *lineEdit;
    QToolButton *historyBtn;
    QMenu *historyMenu;

    QStringList historyList;
    int maxHistory;

    QString lastSelectedText; // 👈 记录上次的选择
    void saveLastSelected(const QString &text);
    void loadLastSelected();

    void addToHistory(const QString &text);
    void rebuildHistoryMenu();
    void saveHistory();
    void loadHistory();
};

#endif // RECENTINPUTFIELD_H
