#ifndef KEYDATAMANAGER_H
#define KEYDATAMANAGER_H

#include <QObject>
#include <QStringList>

class KeyDataManager : public QObject
{
    Q_OBJECT
public:
    static KeyDataManager* instance();

    // 获取当前选中的 Key
    QString currentKeyId() const;
    // 设置当前选中的 Key（会触发信号，通知所有 UI 更新）
    void setCurrentKeyId(const QString &keyId);

    // 获取当前的密钥列表
    QStringList keyList() const;
    // 更新密钥列表（会触发信号，通知所有 UI 刷新下拉框）
    void setKeyList(const QStringList &keys);

    // 刷新状态（加载中、无密钥、正常）
    enum LoadState { Loading, NoKeys, Normal };
    LoadState loadState() const;
    void setLoadState(LoadState state);

signals:
    void currentKeyChanged(const QString &keyId);
    void keyListChanged(const QStringList &keys);
    void loadStateChanged(LoadState state);

private:
    explicit KeyDataManager(QObject *parent = nullptr);
    static KeyDataManager *m_instance;

    QString m_currentKeyId;
    QStringList m_keyList;
    LoadState m_loadState;
};

#endif // KEYDATAMANAGER_H
