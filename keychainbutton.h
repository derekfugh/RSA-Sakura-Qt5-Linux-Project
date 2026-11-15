#ifndef KEYCHAINBUTTON_H
#define KEYCHAINBUTTON_H

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

class KeychainButton : public QPushButton
{
    Q_OBJECT
public:
    explicit KeychainButton(QWidget *parent = nullptr);

    // 如果你以后想换图标，可以开放这个接口
    void setIconPath(const QString &iconPath);

    // 控制是否显示边框（对应你之前代码里的 showBorder 参数）
    void setShowBorder(bool show);

private:
    QLabel *iconLabel;
    bool showBorder;
};

#endif // KEYCHAINBUTTON_H
