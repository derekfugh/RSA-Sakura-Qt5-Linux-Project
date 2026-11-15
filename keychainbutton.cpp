#include "keychainbutton.h"
#include <QPixmap>

KeychainButton::KeychainButton(QWidget *parent) : QPushButton(parent), showBorder(false)
{
    // 1. 设置按钮基础属性（去掉系统默认边框和背景）
    this->setFixedSize(50, 50); // 稍微比图标大一圈，方便悬停和按下
    this->setCursor(Qt::PointingHandCursor);
    this->setStyleSheet(
        "QPushButton { border: none; background: transparent; }"
        "QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); border-radius: 6px; }"
        "QPushButton:pressed { background-color: rgba(255, 255, 255, 0.2); border-radius: 6px; }"
        );

    // 2. 创建 Label 用来显示透明图片
    iconLabel = new QLabel(this);
    iconLabel->setStyleSheet("background: transparent;");
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents); // 鼠标事件穿透给按钮

    // 3. 用水平布局把图标居中
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2); // 给图标四周留点空隙
    layout->addWidget(iconLabel);

    // 4. 设置默认图标（请确保资源路径正确）
    setIconPath(":/appdir/images/Keychain1024.png");
    setShowBorder(true);
}

void KeychainButton::setIconPath(const QString &iconPath)
{
    QPixmap pix(iconPath);
    if (!pix.isNull()) {
        // 缩放图片到 32x32，并保持宽高比（SmoothTransformation 保证透明边缘不出锯齿）
        iconLabel->setPixmap(pix.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void KeychainButton::setShowBorder(bool show)
{
    showBorder = show;
    // 如果开启边框，利用 QSS 画一个圆角边框
    if (showBorder) {
        this->setStyleSheet(
            "QPushButton { border: 1px solid #555; background: transparent; border-radius: 6px; }"
            "QPushButton:hover { border: 1px solid #007AFF; background-color: rgba(0, 122, 255, 0.1); }"
            "QPushButton:pressed { background-color: rgba(0, 122, 255, 0.2); }"
            );
    } else {
        this->setStyleSheet(
            "QPushButton { border: none; background: transparent; }"
            "QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); border-radius: 6px; }"
            "QPushButton:pressed { background-color: rgba(255, 255, 255, 0.2); border-radius: 6px; }"
            );
    }
}
