#ifndef KEYPICKER_H
#define KEYPICKER_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QProcess>
#include "refreshbutton.h"
#include "keydatamanager.h"

class KeyPicker : public QWidget
{
    Q_OBJECT
public:
    explicit KeyPicker(QWidget *parent = nullptr);

    // 获取当前选中的 Key ID
    QString currentKeyId() const;

    // 刷新密钥列表
    void refreshKeys();

signals:
    // 当用户切换密钥时发出信号（如果需要的话）
    void keyChanged(const QString &keyId);

private:
    QComboBox *comboBox;
    RefreshButton *refreshBtn;
    QLabel *loadingLabel;

    void parseKeysFromCommand(); // 调用终端命令并解析的逻辑
    void updateUIFromManager();

private slots:
    void onKeyListChanged(const QStringList &keys);
    void onCurrentKeyChanged(const QString &keyId);
    void onLoadStateChanged(KeyDataManager::LoadState state);
};

#endif // KEYPICKER_H
