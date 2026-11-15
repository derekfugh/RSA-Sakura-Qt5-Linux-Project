#ifndef DROPOVERLAY_H
#define DROPOVERLAY_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>

class DropOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit DropOverlay(QWidget *parent = nullptr);

    // 根据不同的页面，设置不同的布局模式
    // mode: "settings" (上下), "files" (上下), "folder" (四宫格)
    void setMode(const QString &mode);

    // 根据鼠标位置更新高亮，返回当前选中的区域名称 (例如 "top", "bottom", "encrypt", "zip")
    QString updateHighlight(const QPoint &pos);

    // 重置所有高亮
    void resetHighlight();

private:
    QGridLayout *gridLayout;
    QList<QLabel*> labels; // 存放所有的区域标签
    QString currentMode;

    QLabel* createLabel(const QString &text);
};

#endif // DROPOVERLAY_H
