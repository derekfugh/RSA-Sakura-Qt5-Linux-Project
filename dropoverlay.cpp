#include "dropoverlay.h"
#include <QVBoxLayout>

DropOverlay::DropOverlay(QWidget *parent) : QWidget(parent)
{
    this->hide();
    this->setStyleSheet("background-color: rgba(40, 40, 40, 0.84);");

    gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(10, 10, 10, 10);
    gridLayout->setSpacing(10);
}

QLabel* DropOverlay::createLabel(const QString &text)
{
    QLabel *label = new QLabel(text, this);
    label->setAlignment(Qt::AlignCenter);
    // 如果是设置页，边框可以粗一点，字大一点
    if (text.contains("解锁")) {
        label->setStyleSheet(
            "QLabel { font-size: 24px; font-weight: bold; color: gray; "
            "border: 3px dashed #555; border-radius: 12px; }"
            );
    } else {
        label->setStyleSheet(
            "QLabel { font-size: 16px; font-weight: bold; color: gray; "
            "border: 2px dashed #555; border-radius: 8px; }"
            );
    }
    return label;
}

void DropOverlay::setMode(const QString &mode)
{
    if (currentMode == mode) return; // 如果模式没变，不用重复重建

    // 清空旧布局
    QLayoutItem *child;
    while ((child = gridLayout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }
    labels.clear();
    currentMode = mode;

    if (mode == "settings") {
        labels << createLabel("解锁\nUnlock");
        // 让它占据整个网格（第0行，第0列，跨1行，跨1列，但由于是唯一的控件，它会自动撑满）
        gridLayout->addWidget(labels[0], 0, 0, 1, 1);
    }
    else if (mode == "files") {
        labels << createLabel("加密文件\nEncrypt File(s)");
        labels << createLabel("解密文件\nDecrypt File(s)");
        gridLayout->addWidget(labels[0], 0, 0, 1, 2);
        gridLayout->addWidget(labels[1], 1, 0, 1, 2);
    }
    else if (mode == "folder") {
        labels << createLabel("加密\nEncrypt");
        labels << createLabel("压缩\nZip");
        labels << createLabel("解密\nDecrypt");
        labels << createLabel("解压\nUnzip");
        gridLayout->addWidget(labels[0], 0, 0); // 第0行第0列
        gridLayout->addWidget(labels[1], 0, 1); // 第0行第1列
        gridLayout->addWidget(labels[2], 1, 0); // 第1行第0列
        gridLayout->addWidget(labels[3], 1, 1); // 第1行第1列
    }
}

QString DropOverlay::updateHighlight(const QPoint &pos)
{
    resetHighlight();
    if (labels.isEmpty()) return "";

    QString highlightStyle =
        "QLabel { font-size: 16px; font-weight: bold; color: rgba(0, 121, 255, 1.0); "
        // "QLabel { font-size: 18px; font-weight: bold; color: #4A90E2; "  // 亮蓝
        "background-color: rgba(0, 122, 255, 0.2); "  // 背景可以稍微淡一点，衬托文字
        // "QLabel { font-size: 16px; font-weight: bold; color: orange; "
        "background-color: rgba(128, 192, 255, 0.84); "
        "border: 2px solid #007AFF; border-radius: 8px; }";

    // 计算鼠标在哪个区域
    bool isTop = pos.y() < this->height() / 2;

    if (currentMode == "settings") {
        // 👈 无论鼠标在哪，都高亮唯一的 Label，返回 "all"
        labels[0]->setStyleSheet(highlightStyle);
        return "all";
    }
    else if (currentMode == "files") {
        if (isTop) {
            labels[0]->setStyleSheet(highlightStyle);
            return "top";
        } else {
            labels[1]->setStyleSheet(highlightStyle);
            return "bottom";
        }
    }
    else if (currentMode == "folder") {
        bool isLeft = pos.x() < this->width() / 2;
        if (isTop && isLeft) {
            labels[0]->setStyleSheet(highlightStyle);
            return "encrypt";
        } else if (isTop && !isLeft) {
            labels[1]->setStyleSheet(highlightStyle);
            return "zip";
        } else if (!isTop && isLeft) {
            labels[2]->setStyleSheet(highlightStyle);
            return "decrypt";
        } else {
            labels[3]->setStyleSheet(highlightStyle);
            return "unzip";
        }
    }
    return "";
}

void DropOverlay::resetHighlight()
{
    QString normalStyle =
        "QLabel { font-size: 16px; font-weight: bold; color: gray; "
        "border: 2px dashed #555; border-radius: 8px; }";
    for (QLabel *label : labels) {
        label->setStyleSheet(normalStyle);
    }
}
