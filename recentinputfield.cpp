#include "recentinputfield.h"
#include <QSettings>
#include <QAction>
#include <QDebug>

RecentInputField::RecentInputField(QWidget *parent)
    : QWidget(parent), maxHistory(21)
{
    // 1. 主水平布局
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    // 2. 前置提示 Label
    titleLabel = new QLabel("类型 Type", this);
    titleLabel->setStyleSheet("QLabel { margin-left: 3px; color: gray; }");

    // 3. 文本输入框
    lineEdit = new QLineEdit(this);
    lineEdit->setPlaceholderText("例如: \"*.txt\"");

    // 4. 历史记录按钮 (QToolButton)
    historyBtn = new QToolButton(this);
    historyBtn->setText("▼"); // 用字符作为下拉箭头
    historyBtn->setCursor(Qt::PointingHandCursor);
    // 样式：去掉边框和背景，字色灰，鼠标悬停变白
    historyBtn->setStyleSheet(
        "QToolButton { "
        "  border: none; "
        "  background: transparent; "
        "  color: gray; " // 默认灰色
        "  padding: 2px 4px; "
        "  border-radius: 4px; "
        "}"
        "QToolButton:hover { "
        "  background-color: rgba(0, 122, 255, 0.2); " // 👈 悬停时的半透明蓝色
        "  color: white; "                             // 👈 悬停时文字变白
        "}"
        "QToolButton:pressed { "
        "  background-color: rgba(0, 122, 255, 0.5); " // 👈 按下时的深蓝色
        "}"
        "QToolButton::menu-indicator { image: none; }"   // 隐藏系统自带的小三角
        );

    // 5. 创建弹出的菜单
    historyMenu = new QMenu(this);
    // 暗黑模式下菜单的样式（可选，如果不设则跟随系统）
    historyMenu->setStyleSheet(
        "QMenu { background-color: #2b2b2b; color: white; border: 1px solid #555; }"
        "QMenu::item { padding: 6px 24px; }"
        "QMenu::item:selected { background-color: #007AFF; color: white; }"
        );

    // 将菜单绑定到按钮上（InstantPopup 表示点击按钮立刻弹出菜单）
    historyBtn->setMenu(historyMenu);
    historyBtn->setPopupMode(QToolButton::InstantPopup);

    // 6. 组装界面
    layout->addWidget(titleLabel);
    layout->addWidget(lineEdit, 1); // 输入框占满剩余空间
    layout->addWidget(historyBtn);

    // 7. 事件绑定
    connect(lineEdit, &QLineEdit::returnPressed, this, [this]() {
        addToHistory(lineEdit->text());
        emit returnPressed();
    });

    // 输入框内容改变时发送信号
    connect(lineEdit, &QLineEdit::textChanged, this, [this](const QString &text){
        emit textChanged(text);
    });

    // 8. 加载历史记录
    loadHistory();
    loadLastSelected();
    connect(lineEdit, &QLineEdit::returnPressed, this, [this]() {
        addToHistory(lineEdit->text());
        // 👈 用户回车确认时，也保存为上次选择
        saveLastSelected(lineEdit->text());
        emit returnPressed();
    });
}

void RecentInputField::setTitle(const QString &title)
{
    titleLabel->setText(title);
}

QString RecentInputField::text() const
{
    return lineEdit->text();
}

void RecentInputField::setText(const QString &text)
{
    lineEdit->setText(text);
}

void RecentInputField::setMaxHistory(int maxItems)
{
    maxHistory = maxItems;
    if (historyList.size() > maxHistory) {
        historyList = historyList.mid(0, maxHistory);
        rebuildHistoryMenu();
        saveHistory();
    }
}

void RecentInputField::setHistory(const QStringList &history)
{
    historyList = history;
    if (historyList.size() > maxHistory) {
        historyList = historyList.mid(0, maxHistory);
    }
    rebuildHistoryMenu();
    saveHistory();
}

void RecentInputField::addToHistory(const QString &text)
{
    QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) return;

    // 去重：如果历史里已经有，先删掉旧的，再塞到最前面
    historyList.removeAll(trimmed);
    historyList.prepend(trimmed);

    // 限制最大条数
    while (historyList.size() > maxHistory) {
        historyList.removeLast();
    }

    rebuildHistoryMenu();
    saveHistory();
}

// 👈 核心逻辑：每次历史记录变化时，重新构建下拉菜单
void RecentInputField::rebuildHistoryMenu()
{
    historyMenu->clear();

    if (historyList.isEmpty()) {
        QAction *emptyAction = historyMenu->addAction("暂无历史记录");
        emptyAction->setEnabled(false);
        return;
    }

    for (const QString &item : historyList) {
        QAction *action = historyMenu->addAction(item);
        // 当用户点击菜单里的某一项时，把内容填入输入框
        connect(action, &QAction::triggered, this, [this, item]() {
            lineEdit->setText(item);
            lineEdit->setFocus(); // 选完后光标回到输入框
            emit textChanged(item);
            saveLastSelected(item); // 👈 用户从菜单选中后，立刻保存
        });
    }
}

// 持久化存储
void RecentInputField::saveHistory()
{
    QSettings settings("RSA-Sakura", "Driver");
    settings.setValue("recentInputs", historyList);
}

void RecentInputField::loadHistory()
{
    QSettings settings("RSA-Sakura", "Driver");
    historyList = settings.value("recentInputs").toStringList();

    // 首次使用给几个默认值
    if (historyList.isEmpty()) {
        historyList << "\"*.rsa\""
                    << "\"*.zip\""
                    << "\"*.txt\""
                    << "\"*.c\" \"*.h\" \"*.cpp\" \"*.hpp\""
                    << "\"*.jpg\" \"*.png\"";
    }

    rebuildHistoryMenu();
}

// 👈 实现保存上次选择
void RecentInputField::saveLastSelected(const QString &text)
{
    lastSelectedText = text;
    QSettings settings("RSA-Sakura", "Driver");
    settings.setValue("lastSelectedInput", text);
}
// 👈 实现加载上次选择
void RecentInputField::loadLastSelected()
{
    QSettings settings("RSA-Sakura", "Driver");
    lastSelectedText = settings.value("lastSelectedInput").toString();

    // 如果之前选过，并且输入框当前为空（防止覆盖用户手动输入的内容），则自动填入
    if (!lastSelectedText.isEmpty() && lineEdit->text().isEmpty()) {
        lineEdit->setText(lastSelectedText);
    }
}
