#ifndef REFRESHBUTTON_H
#define REFRESHBUTTON_H

#include <QPushButton>

class RefreshButton : public QPushButton
{
    Q_OBJECT
public:
    explicit RefreshButton(QWidget *parent = nullptr);

    // 预留一个接口，方便以后换成 SVG / PNG 图标
    void setRefreshIcon(const QString &iconPath);
};

#endif // REFRESHBUTTON_H
