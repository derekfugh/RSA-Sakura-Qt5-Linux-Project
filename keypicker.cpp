#include "keypicker.h"
#include <QDebug>
#include <QRegularExpression>
#include <QFileInfo>
#include <QStandardPaths>

KeyPicker::KeyPicker(QWidget *parent) : QWidget(parent)
{
    // 1. 创建水平布局
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 0, 4, 0);
    layout->setSpacing(8);

    // 2. 创建前置 Label
    QLabel *titleLabel = new QLabel("密钥 key", this);
    titleLabel->setStyleSheet("QLabel { color: gray; }");

    // 3. 创建下拉框
    comboBox = new QComboBox(this);
    comboBox->setMinimumWidth(160);
    comboBox->setStyleSheet(
        "QComboBox { "
        // "  background-color: #3c3c3c; color: white; "
        "  border: 1px solid #555; border-radius: 4px; padding: 4px; "
        "}"
        "QComboBox::drop-down { border: none; }"
        );

    // 4. 创建刷新按钮
    refreshBtn = new RefreshButton(this);

    // 5. 组装
    layout->addWidget(titleLabel);
    layout->addWidget(comboBox);
    layout->addWidget(refreshBtn);
    layout->addStretch(); // 弹簧，把左边的推到最左

    // 6. 绑定事件
    connect(refreshBtn, &QPushButton::clicked, this, &KeyPicker::refreshKeys);
    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index >= 0) {
            QString keyId = comboBox->itemData(index).toString();
            if (keyId.isEmpty()) return; // 忽略“加载中”或“没有可用密钥”的无效选项

            // 更新单例的选中项
            KeyDataManager::instance()->setCurrentKeyId(keyId);

            // 发出本地信号
            emit keyChanged(keyId);
        }
    });

    // 👈 核心：绑定单例的信号，只要数据一变，立刻更新 UI
    connect(KeyDataManager::instance(), &KeyDataManager::keyListChanged, this, &KeyPicker::onKeyListChanged);
    connect(KeyDataManager::instance(), &KeyDataManager::currentKeyChanged, this, &KeyPicker::onCurrentKeyChanged);
    connect(KeyDataManager::instance(), &KeyDataManager::loadStateChanged, this, &KeyPicker::onLoadStateChanged);

    // 初始化时，从单例拉取一次当前状态
    updateUIFromManager();
}

// 从单例拉取数据更新 UI
void KeyPicker::updateUIFromManager()
{
    KeyDataManager *manager = KeyDataManager::instance();

    // 暂时断开信号，防止 clear 时触发错误的 keyChanged
    QSignalBlocker blocker(comboBox);

    comboBox->clear();

    if (manager->loadState() == KeyDataManager::Loading) {
        comboBox->addItem("加载中...", "");
        comboBox->setEnabled(false);
        comboBox->setStyleSheet("");
        return;
    }

    QString highlightYellow = "#E0AA00";
    QStringList keys = manager->keyList();

    if (keys.isEmpty()) {
        comboBox->addItem("没有可用密钥 / No keys available", "");
        comboBox->setStyleSheet(QString("QComboBox { color: %1; }").arg(highlightYellow));
    } else {
        for (const QString &key : keys) {
            comboBox->addItem(key, key);
        }
        comboBox->setStyleSheet("");
    }

    comboBox->setEnabled(true);

    // 设置当前选中的项
    QString currentKey = manager->currentKeyId();
    int idx = comboBox->findData(currentKey);
    if (idx >= 0) {
        comboBox->setCurrentIndex(idx);
    }
}
// 单例通知：列表变了
void KeyPicker::onKeyListChanged(const QStringList &)
{
    updateUIFromManager();
}
// 单例通知：选中项变了
void KeyPicker::onCurrentKeyChanged(const QString &keyId)
{
    int idx = comboBox->findData(keyId);
    if (idx >= 0 && comboBox->currentIndex() != idx) {
        QSignalBlocker blocker(comboBox); // 防止引起循环触发
        comboBox->setCurrentIndex(idx);
    }
}
// 单例通知：加载状态变了
void KeyPicker::onLoadStateChanged(KeyDataManager::LoadState state)
{
    updateUIFromManager();
}

QString KeyPicker::currentKeyId() const
{
    return comboBox->currentData().toString();
}

void KeyPicker::refreshKeys()
{
    KeyDataManager *manager = KeyDataManager::instance();
    manager->setLoadState(KeyDataManager::Loading);

    /*
    QString highlightYellow = "#E0AA00";    //FFD700
    comboBox->clear();
    comboBox->addItem("加载中...", "");
    comboBox->setEnabled(false);
    */

    QString appDir = "";

    // 1. 对应 `which xrsaoapp.sw`：使用 Qt 原生的跨平台查找
    QString appPath = QStandardPaths::findExecutable("xrsaoapp.sw");

    if (appPath.isEmpty()) {
        /*
        comboBox->clear();
        //comboBox->addItem("安装错误：未找到 xrsaoapp.sw 软链接");
        comboBox->addItem("没有可用密钥 / No keys available", "");
        comboBox->setStyleSheet(QString("QComboBox { color: %1; }").arg(highlightYellow));
        comboBox->setEnabled(true);
        */
        manager->setKeyList({});
        manager->setLoadState(KeyDataManager::NoKeys);
        return;
    }

    // 2. 对应 `readlink -f`：使用 QFileInfo 解析真实路径
    // QFileInfo::canonicalFilePath() 会自动穿透所有的软链接，找到最终的目标文件
    QFileInfo fileInfo(appPath);
    QString realPath = fileInfo.canonicalFilePath();

    if (realPath.isEmpty()) {
        /*
        comboBox->clear();
        //comboBox->addItem("安装错误：软链接已损坏", "");
        comboBox->addItem("没有可用密钥 / No keys available", "");
        comboBox->setStyleSheet(QString("QComboBox { color: %1; }").arg(highlightYellow));
        comboBox->setEnabled(true);
        */
        manager->setKeyList({});
        manager->setLoadState(KeyDataManager::NoKeys);
        return;
    }

    // 3. 对应 `dirname`：获取父目录
    // QFileInfo(realPath).absolutePath() 会直接返回 /mnt/.../xrsa_shareware
    appDir = QFileInfo(realPath).absolutePath();
    qDebug() << "主程序目录:" << appDir;

    // 4. 拼接获取密钥列表的命令
    // 注意：用双引号包裹路径，防止路径中有空格导致命令解析失败
    QString iniPath = appDir + "/xrsao.ini";
    QStringList arguments;
    arguments << "-l"
              << "-crsa_key_list"
              << "-kkey"
              << "-i" << iniPath;

    // 5. 执行 xinitool 命令
    QProcess process;
    process.start("xinitool", arguments);
    process.waitForFinished(3000); // 最多等3秒

    // 6. 获取输出并解析
    QString output = QString::fromUtf8(process.readAllStandardOutput() + process.readAllStandardError()).trimmed();
    qDebug() << "xinitool 输出:" << output;

    /*
    comboBox->clear();

    if (output.trimmed().isEmpty()) {
        comboBox->addItem("没有可用密钥 / No keys available", "");
        // 👈 没有密钥时，下拉框文字变黄
        comboBox->setStyleSheet(QString("QComboBox { color: %1; }").arg(highlightYellow));
        return;
    }

    // 👇 这里写你的解析逻辑！比如正则匹配
    // 假设输出是逐行的 key 名称
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        QString keyName = line.trimmed();
        if (!keyName.isEmpty()) {
            comboBox->addItem(keyName, keyName);
        }
    }

    // 7. 收尾检查
    if (comboBox->count() == 0) {
        comboBox->addItem("没有可用密钥 / No keys available", "");
        // 👈 没有密钥时，下拉框文字变黄
        comboBox->setStyleSheet(QString("QComboBox { color: %1; }").arg(highlightYellow));
    } else {
        // 👈 有密钥时，清除之前的样式（恢复系统默认颜色）
        comboBox->setStyleSheet("");
    }
    comboBox->setEnabled(true);
    */

    // 执行完拿到 keys 列表后：
    if (output.isEmpty()) {
        manager->setKeyList({}); // 空列表
        manager->setLoadState(KeyDataManager::NoKeys);
        return;
    }

    QStringList newKeys;
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        QString keyName = line.trimmed();
        if (!keyName.isEmpty()) newKeys << keyName;
    }

    if (newKeys.isEmpty()) {
        manager->setKeyList({});
        manager->setLoadState(KeyDataManager::NoKeys);
    } else {
        manager->setKeyList(newKeys);
        manager->setLoadState(KeyDataManager::Normal);
    }
}
