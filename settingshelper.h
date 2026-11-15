#ifndef SETTINGSHELPER_H
#define SETTINGSHELPER_H

#include <QSettings>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>

// 读取 Checkbox 状态
inline void loadCheckboxState(QCheckBox *cb, const QString &key, bool defaultState = false)
{
    if (!cb) return;
    QSettings settings("RSA-Sakura", "Driver");
    bool state = settings.value(key, defaultState).toBool();
    cb->setChecked(state);
}

// 监听 Checkbox 变化并自动保存
inline void bindCheckboxState(QCheckBox *cb, const QString &key)
{
    if (!cb) return;

    // 初始加载
    loadCheckboxState(cb, key);

    // 状态改变时自动保存
    QObject::connect(cb, &QCheckBox::toggled, [key](bool checked) {
        QSettings settings("RSA-Sakura", "Driver");
        settings.setValue(key, checked);
    });
}

// 读取 LineEdit 文本
inline void loadLineEditState(QLineEdit *le, const QString &key, const QString &defaultText = "")
{
    if (!le) return;
    QSettings settings("RSA-Sakura", "Driver");
    QString text = settings.value(key, defaultText).toString();
    le->setText(text);
}

// 监听 LineEdit 变化并自动保存
inline void bindLineEditState(QLineEdit *le, const QString &key)
{
    if (!le) return;

    // 初始加载
    loadLineEditState(le, key);

    // 监听文本变化，实时保存
    QObject::connect(le, &QLineEdit::textChanged, [key](const QString &text) {
        QSettings settings("RSA-Sakura", "Driver");
        settings.setValue(key, text);
    });
}
// 在这个版本中，只在用户敲回车或切换到别的控件时保存
inline void bindLineEditStateOnFinished(QLineEdit *le, const QString &key)
{
    if (!le) return;
    loadLineEditState(le, key);

    // editingFinished 信号会在按回车或失去焦点时触发
    QObject::connect(le, &QLineEdit::editingFinished, [le, key]() {
        QSettings settings("RSA-Sakura", "Driver");
        settings.setValue(key, le->text());
    });
}

// 读取 SpinBox 数值
inline void loadSpinBoxState(QSpinBox *sb, const QString &key, int defaultValue = 0)
{
    if (!sb) return;
    QSettings settings("RSA-Sakura", "Driver");
    int value = settings.value(key, defaultValue).toInt();
    sb->setValue(value);
}
// 监听 SpinBox 变化并自动保存
inline void bindSpinBoxState(QSpinBox *sb, const QString &key, int defaultValue = 0)
{
    if (!sb) return;

    // 初始加载
    loadSpinBoxState(sb, key, defaultValue);

    // 监听数值变化，实时保存
    QObject::connect(sb, QOverload<int>::of(&QSpinBox::valueChanged), [key](int value) {
        QSettings settings("RSA-Sakura", "Driver");
        settings.setValue(key, value);
    });
}

#endif // SETTINGSHELPER_H
