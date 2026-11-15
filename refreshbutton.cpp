#include "refreshbutton.h"
#include <QIcon>

RefreshButton::RefreshButton(QWidget *parent) : QPushButton(parent)
{
    // 默认使用 Unicode 字符
    this->setText("↻");

    // 设置基础样式（默认无边框、白色、大字体）
    this->setStyleSheet(
        "QPushButton {"
        "  font-family: 'Noto Sans Symbols', 'Arial', sans-serif;" // 👈 强制指定字体
        "  border: none;"
        "  font-size: 20px;"
        "  background: transparent;"
        // "  color: black;"
        "  padding: 2px 5px;" // 给一点内边距，防止字符贴边
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(0, 122, 255, 0.2);" // 悬停时轻微变亮
        "  border-radius: 4px;"
        "}"
        "QPushButton:pressed {"
        "  background-color: rgba(0, 122, 255, 0.5);" // 按下时更亮
        "}"
        );

    // 固定大小，保持界面整洁（类似 Mac 的小按钮）
    this->setFixedSize(24, 24);
    this->setCursor(Qt::PointingHandCursor);
}

// 预留的接口：以后如果觉得 Unicode 不好看，直接在这里换成 Icon
void RefreshButton::setRefreshIcon(const QString &iconPath)
{
    this->setText(""); // 清空文字
    this->setIcon(QIcon(iconPath));
    this->setIconSize(QSize(18, 18));
}
