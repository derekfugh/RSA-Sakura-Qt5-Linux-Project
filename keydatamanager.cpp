#include "keydatamanager.h"

KeyDataManager* KeyDataManager::m_instance = nullptr;

KeyDataManager* KeyDataManager::instance()
{
    if (!m_instance) {
        m_instance = new KeyDataManager();
    }
    return m_instance;
}

KeyDataManager::KeyDataManager(QObject *parent) : QObject(parent), m_loadState(Loading)
{
}

QString KeyDataManager::currentKeyId() const { return m_currentKeyId; }

void KeyDataManager::setCurrentKeyId(const QString &keyId)
{
    if (m_currentKeyId != keyId) {
        m_currentKeyId = keyId;
        emit currentKeyChanged(m_currentKeyId);
    }
}

QStringList KeyDataManager::keyList() const { return m_keyList; }

void KeyDataManager::setKeyList(const QStringList &keys)
{
    m_keyList = keys;
    // 如果当前选中的 key 不在新列表里，或者根本没选，默认选第一个
    if (!m_keyList.contains(m_currentKeyId) && !m_keyList.isEmpty()) {
        m_currentKeyId = m_keyList.first();
        emit currentKeyChanged(m_currentKeyId);
    }
    emit keyListChanged(m_keyList);
}

KeyDataManager::LoadState KeyDataManager::loadState() const { return m_loadState; }

void KeyDataManager::setLoadState(LoadState state)
{
    m_loadState = state;
    emit loadStateChanged(state);
}
