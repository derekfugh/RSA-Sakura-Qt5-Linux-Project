#include "driverdialog.h"
#include "ui_driverdialog.h"
#include "keypicker.h"
#include "settingshelper.h"

#include <QLineEdit>
#include <QDir>
#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QFileInfo>
#include <QStandardPaths>
#include <QMessageBox>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>

#include <QLocalServer>
#include <QLocalSocket>

QString escapeForWindowsCmd(const QString &path) {
    // 1. 确保使用原生分隔符 (可选，但推荐)
    QString nativePath = QDir::toNativeSeparators(path);

    // 2. 转义内部的双引号: " -> \"
    QString escaped = nativePath.replace("\"", "\\\"");

    // 3. 用双引号包裹整个路径
    return "\"" + escaped + "\"";
}
QString escapeForUnixShell(const QString &path) {
    // 策略：优先使用单引号包裹，因为单引号内不解释特殊字符
    // 如果路径中包含单引号，则替换为 '\'' (即: 结束引号 + 转义引号 + 开始引号)
    QString escaped = path;
    escaped.replace("'", "'\\''");

    return "'" + escaped + "'";
}
QString escapeForShell(const QString &path) {
#ifdef Q_OS_WIN
    return escapeForWindowsCmd(path);
#else
    return escapeForUnixShell(path);
#endif
}
// 辅助函数：在 Linux 新终端中执行命令
void runInTerminal(const QString &command) {
    QProcess process;
    // 使用 gnome-terminal 打开新窗口，并在里面执行命令
    // -- 后面的部分会原样传给 bash -c
    //process.startDetached("gnome-terminal", QStringList() << "--" << "bash" << "-c" << command + "; exec bash");

    // 如果你也想兼容 KDE (Konsole) 或 XFCE (Xfce Terminal)，可以使用环境变量判断：
    //*
    QString term = qEnvironmentVariable("TERMINAL", "gnome-terminal");
    if (term.contains("gnome-terminal")) {
        process.startDetached(term, QStringList() << "--" << "bash" << "-c" << command + "; exec bash");
    } else {
        process.startDetached(term, QStringList() << "-e" << "bash -c \"" + command + "; exec bash\"");
    }
    //*/
}

// 辅助函数：探测命令是否真实存在（类似 Mac 的 AppPaths.shared 检查）
bool commandExists(const QString &cmd) {
    QProcess process;
    // 用 which 命令查找软链接是否存在
    process.start("which", QStringList() << cmd);
    process.waitForFinished();
    return process.exitCode() == 0;
}

void DriverDialog::checkLicenseStatus()
{
    updateLicenseUI(LicenseState::Unknown);

    // 1. 检查软链接是否失效
    if (!commandExists("xrsaoapp.sw")) {
        updateLicenseUI(LicenseState::Uninstalled);
        return;
    }

    // 2. 执行命令获取输出
    QProcess process;
    //process.start("xrsaoapp.sw", QStringList() << "\\?");
    process.start("xrsaoapp.sw", QStringList());
    // 等待启动，如果连启动都失败（软链接失效、没权限），直接判定为未安装
    if (!process.waitForStarted()) {
        updateLicenseUI(LicenseState::Uninstalled);
        return;
    }

    // 等待执行完成，设置超时时间为 3 秒防卡死
    if (!process.waitForFinished(3000)) {
        updateLicenseUI(LicenseState::Unknown);
        return;
    }

    // 获取标准输出和标准错误
    QString stdOut = QString::fromUtf8(process.readAllStandardOutput());
    QString stdErr = QString::fromUtf8(process.readAllStandardError());
    QString fullOutput = stdOut + stdErr;

    // 👈 核心：使用正则表达式清除 ANSI 转义序列
    QRegularExpression ansiRegex(
        "\x1B(?:[@-Z\\-_]|\\[[0-?]*[ -/]*[@-~])"
        );
    QString cleanedOutput = fullOutput;
    cleanedOutput.remove(ansiRegex);

    // 👈 把过滤后的干净文本显示到 Raw Output 框里
    //rawOutputTextEdit->setPlainText(fullOutput);
    rawOutputTextEdit->setPlainText(cleanedOutput);

    // 如果输出为空，隐藏展开按钮；有输出则显示
    if (fullOutput.trimmed().isEmpty()) {
        rawOutputToggle->hide();
    } else {
        rawOutputToggle->show();
    }

    // 3. 判断逻辑（对应 Mac 版）
    // 核心判断（完全照搬 Mac）
    // 不管退出码是 0 还是 1，只要拿到了输出，就按输出内容判断
    if (fullOutput.contains("Trial license")) {
        updateLicenseUI(LicenseState::Locked);
    } else {
        // 只要不是 trial license，且有输出，就认为是已解锁
        updateLicenseUI(LicenseState::Unlocked);
    }

    // 别忘了把原始输出存下来，因为你 UI 里有个“查看原始输出”
    // ui->rawOutputTextEdit->setPlainText(fullOutput);

    // 👇 加这两行，看看 Qt Creator 底部的“应用程序输出”里打印了什么
    qDebug() << "Exit Code:" << process.exitCode();
    qDebug() << "Output:" << fullOutput;
}
// 4. 更新 UI 状态
void DriverDialog::updateLicenseUI(LicenseState state)
{
    currentLicenseState = state; // 👈 记录当前状态
    // 假设你的第一页有个 QLabel 叫 statusLabel，旁边有个 QLabel 叫 statusDot
    switch (state) {
    case LicenseState::Locked:
        ui->statusLabel->setText("已锁定 Locked");
        ui->statusLabel->setStyleSheet("color: red; font-weight: bold;");
        // 👇 已锁定：禁用 Lock 按钮，启用 Unlock 按钮
        ui->pushButtonLock->setEnabled(false);
        ui->pushButtonUnlock->setEnabled(true);
        break;
    case LicenseState::Unlocked:
        ui->statusLabel->setText("已解锁 Unlocked");
        ui->statusLabel->setStyleSheet("color: green; font-weight: bold;");
        // 👇 已解锁：启用 Lock 按钮，禁用 Unlock 按钮
        ui->pushButtonLock->setEnabled(true);
        ui->pushButtonUnlock->setEnabled(false);
        break;
    case LicenseState::Uninstalled:
        ui->statusLabel->setText("未安装 Uninstalled");
        ui->statusLabel->setStyleSheet("color: #FFA500; font-weight: bold;"); // 黄色
        // 👇 未安装：两个按钮都禁用（因为你 Mac 版里是 `.disabled(!isInstalled)`）
        ui->pushButtonLock->setEnabled(false);
        ui->pushButtonUnlock->setEnabled(false);
        break;
    case LicenseState::Unknown:
        ui->statusLabel->setText("检测中 checking …");
        ui->statusLabel->setStyleSheet("color: gray; font-weight: bold;");
        // 👇 检测中：两个都禁用，防止误触
        ui->pushButtonLock->setEnabled(false);
        ui->pushButtonUnlock->setEnabled(false);
        break;
    }
}
void DriverDialog::setupRawOutputUI()
{
    // 1. 创建展开按钮
    rawOutputToggle = new QToolButton(this);
    rawOutputToggle->setText("▶ 查看原始输出 (Raw Output)");
    rawOutputToggle->setStyleSheet(
        "QToolButton { border: none; background: transparent; color: gray; font-size: 13px; text-align: left; padding: 0; }"
        // "QToolButton:hover { color: white; }"
        );
    rawOutputToggle->setCheckable(true);
    rawOutputToggle->setChecked(false);

    // 2. 创建滚动框
    rawOutputArea = new QScrollArea(this);
    rawOutputArea->setStyleSheet("QScrollArea { border: 1px solid #555; background-color: #1e1e1e; border-radius: 4px; }");

    // 👈 核心设置：固定高度策略，初始高度为 0
    rawOutputArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    rawOutputArea->setMinimumHeight(0);
    rawOutputArea->setMaximumHeight(0);

    rawOutputTextEdit = new QTextEdit(rawOutputArea);
    rawOutputTextEdit->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: #4A90E2; font-family: Consolas, monospace; font-size: 12px; border: none; }");
    rawOutputTextEdit->setReadOnly(true);
    //rawOutputArea->setMinimumHeight(0);
    //rawOutputArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    rawOutputArea->setWidget(rawOutputTextEdit);
    rawOutputArea->setWidgetResizable(true);

    // 3. 👈 放进你在 Designer 里预留的布局
    ui->rawOutputLayout->addWidget(rawOutputToggle);
    ui->rawOutputLayout->addWidget(rawOutputArea);

    // 4. 设置动画
    rawAnim = new QPropertyAnimation(rawOutputArea, "maximumHeight", this);
    rawAnim->setDuration(200); // 200毫秒，类似 Mac 的手感
    rawAnim->setEasingCurve(QEasingCurve::InOutQuad);

    // 5. 绑定点击事件
    connect(rawOutputToggle, &QToolButton::toggled, this, [this](bool checked) {
        rawOutputToggle->setText(checked ? "▼ 查看原始输出 (Raw Output)" : "▶ 查看原始输出 (Raw Output)");

        rawAnim->stop();
        if (checked) {
            // 👈 展开：高度过渡到 180px（大约显示 8~10 行代码），下方的按钮会被自动推下去
            rawAnim->setStartValue(0);
            rawAnim->setEndValue(144);
            rawOutputArea->show();
        } else {
            // 👈 收起：高度过渡到 0，下方的按钮会自动弹回来
            rawAnim->setStartValue(rawOutputArea->height());
            rawAnim->setEndValue(0);
        }
        rawAnim->start();
    });

    // 在你的 setupRawOutputUI 里加上这句
    connect(rawAnim, &QPropertyAnimation::finished, this, [this]() {
        qDebug() << "动画结束，当前 rawOutputArea 的 maximumHeight 是:" << rawOutputArea->maximumHeight();
        qDebug() << "实际物理高度是:" << rawOutputArea->height();
    });
}

DriverDialog::DriverDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DriverDialog)
{
    ui->setupUi(this);
    setFixedSize(448,249);

    setupDropOverlay();
    setupRawOutputUI();

    initValidators();
    initPageSwitching();
    initKeyPickers();
    initCheckBoxLineEdit();
    initSpinBoxes();

    // 启动本地服务器监听
    localServer = new QLocalServer(this);
    // 如果服务器已经存在，先清理掉旧连接（防止上次异常退出残留）
    QLocalServer::removeServer("RSA-Sakura-Driver-Server");
    localServer->listen("RSA-Sakura-Driver-Server");
    connect(localServer, &QLocalServer::newConnection, this, &DriverDialog::handleNewConnection);
}

void DriverDialog::stackedWedgetSelect(int index)
{
    qDebug() << "[Page] " << index;
    ui->stackedWidget->setCurrentIndex(index);
    switch (index) {
        case 0 :
            checkLicenseStatus();
            break;
        case 1 :
            break;
        case 2 :
            if (ui->keyPickerFiles->currentKeyId().isEmpty()) ui->keyPickerFiles->refreshKeys();
            break;
        case 3 :
            if (ui->keyPickerFolder->currentKeyId().isEmpty()) ui->keyPickerFolder->refreshKeys();
            break;
    }
    QSettings settings("RSA-Sakura", "Driver");
    settings.setValue("currentPage", index);
}
void DriverDialog::initValidators()
{
    // "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.@-_";
    QString patternKeyPairChar = "^[a-zA-Z0-9.@_-]*$";
    QRegularExpression regExp(patternKeyPairChar);
    // if (!regExp.isValid())  qDebug() << "正则表达式无效:" << regExp.errorString();
    // 创建验证器
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(regExp, this);
    // 将验证器绑定到 KeyPairName
    ui->KeyPairName->setValidator(validator);
    // 👈 绑定文本变化信号，实时更新字符计数
    connect(ui->KeyPairName, &QLineEdit::textChanged, this, &DriverDialog::onKeyPairNameChanged);
    // 👈 初始化时手动调用一次，防止启动时显示为空
    onKeyPairNameChanged(ui->KeyPairName->text());
}
void DriverDialog::initPageSwitching()
{
    // 读取上次的页面索引并初始化
    QSettings settings("RSA-Sakura", "Driver");
    int lastPageIndex = settings.value("currentPage", 0).toInt(); // 默认是第 0 页（授权页）
    // 防止历史记录里的索引越界（比如从旧版本升级上来，页面变少了）
    if (lastPageIndex < 0 || lastPageIndex >= ui->stackedWidget->count()) {
        lastPageIndex = 0;
    }
    ui->comboBox->setCurrentIndex(lastPageIndex);

    // 绑定切页信号
    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DriverDialog::stackedWedgetSelect);
    stackedWedgetSelect(lastPageIndex);
}
void DriverDialog::initKeyPickers()
{
    // 绑定刷新按钮
    connect(ui->refreshButtonStatus, &QPushButton::clicked, this, &DriverDialog::checkLicenseStatus);
    // 如果需要监听 keyPickerFiles 的变化
    connect(ui->keyPickerFiles, &KeyPicker::keyChanged, this, [](const QString &keyId) {
        qDebug() << "用户切换了密钥:" << keyId;
    });
}
void DriverDialog::initCheckBoxLineEdit()
{
    // 1. Page2 变化 -> 同步到 Page3
    connect(ui->checkBoxOverwriteF, &QCheckBox::toggled, this, [this](bool checked) {
        ui->checkBoxOverwriteD->blockSignals(true);
        ui->checkBoxOverwriteD->setChecked(checked);
        ui->checkBoxOverwriteD->blockSignals(false);
    });
    // 2. Page3 变化 -> 同步到 Page2
    connect(ui->checkBoxOverwriteD, &QCheckBox::toggled, this, [this](bool checked) {
        ui->checkBoxOverwriteF->blockSignals(true);
        ui->checkBoxOverwriteF->setChecked(checked);
        ui->checkBoxOverwriteF->blockSignals(false);
    });
    connect(ui->checkBoxResign, &QCheckBox::toggled, this, [this](bool checked) {
        if (checked) {
            // 如果 checkBoxResign 被勾选，则取消 checkBoxRecover
            // blockSignals(true) 防止 ui->checkBoxRecover 的状态改变再次触发 checkBoxResign 的信号
            ui->checkBoxRecover->blockSignals(true);
            ui->checkBoxRecover->setChecked(false);
            ui->checkBoxRecover->blockSignals(false);
        }
    });
    connect(ui->checkBoxRecover, &QCheckBox::toggled, this, [this](bool checked) {
        if (checked) {
            // 如果 checkBoxResign 被勾选，则取消 checkBoxRecover
            ui->checkBoxResign->blockSignals(true);
            ui->checkBoxResign->setChecked(false);
            ui->checkBoxResign->blockSignals(false);
        }
    });

    bindCheckboxState(ui->checkBoxFast, "fastmode");
    bindCheckboxState(ui->checkBoxOverwriteF, "overwrite");
    bindCheckboxState(ui->checkBoxOverwriteD, "overwrite");
    bindCheckboxState(ui->checkBoxRecursive, "folderView.recursive");

    bindLineEditStateOnFinished(ui->KeyPairName, "keyPairName");

    // 1. 定义颜色
    // QString darkRed = "#8B0000";        // 暗红
    QString brightRed = "#FF3B30";      // 亮红（苹果红）
    // QString darkGreen = "#006400";      // 暗绿
    QString brightGreen = "#34C759";    // 亮绿（苹果绿）

    // 2. 定义一个更新样式的 Lambda 函数
    auto updateSignRecoverStyles = [=]() {
        /*
        // Resign 的样式：勾选时亮红，未勾选时暗红
        QString signColor = ui->checkBoxResign->isChecked() ? brightRed : darkRed;
        ui->checkBoxResign->setStyleSheet(QString(
                                              "QCheckBox { color: %1; }"
                                              "QCheckBox::indicator { width: 14px; height: 14px; }"
                                              ).arg(signColor));

        // Recover 的样式：勾选时亮绿，未勾选时暗绿
        QString recoverColor = ui->checkBoxRecover->isChecked() ? brightGreen : darkGreen;
        ui->checkBoxRecover->setStyleSheet(QString(
                                               "QCheckBox { color: %1; }"
                                               "QCheckBox::indicator { width: 14px; height: 14px; }"
                                               ).arg(recoverColor));
        */
        // 勾选时 1.0 (100%)，未勾选时 0.48 (48%)
        double signAlpha = ui->checkBoxResign->isChecked() ? 1.0 : 0.48;
        double recoverAlpha = ui->checkBoxRecover->isChecked() ? 1.0 : 0.48;

        // 3. 利用 Qt 的 QColor 动态生成带透明度的颜色字符串 (rgba)
        QColor resignColor(brightRed);
        resignColor.setAlphaF(signAlpha);

        QColor recoverColor(brightGreen);
        recoverColor.setAlphaF(recoverAlpha);

        // 4. 应用样式：去掉 font-weight: bold;，使用 rgba 颜色
        ui->checkBoxResign->setStyleSheet(QString(
                                              "QCheckBox { color: rgba(%1, %2, %3, %4); }" // 注意这里没有 bold 了
                                              //"QCheckBox::indicator { width: 14px; height: 14px; }"
                                              ).arg(resignColor.red()).arg(resignColor.green()).arg(resignColor.blue()).arg(resignColor.alphaF()));

        ui->checkBoxRecover->setStyleSheet(QString(
                                               "QCheckBox { color: rgba(%1, %2, %3, %4); }"
                                               //"QCheckBox::indicator { width: 14px; height: 14px; }"
                                               ).arg(recoverColor.red()).arg(recoverColor.green()).arg(recoverColor.blue()).arg(recoverColor.alphaF()));
    };

    // 3. 绑定信号，当状态改变时立刻更新颜色
    connect(ui->checkBoxResign, &QCheckBox::toggled, this, updateSignRecoverStyles);
    connect(ui->checkBoxRecover, &QCheckBox::toggled, this, updateSignRecoverStyles);

    // 4. 初始化时手动调用一次，确保启动时颜色正确（防止读取历史状态时未触发）
    updateSignRecoverStyles();
}
void DriverDialog::initSpinBoxes()
{
    // 绑定 SpinBox 状态记忆
    bindSpinBoxState(ui->spinNBits, "kv.nBits", 2184);
    bindSpinBoxState(ui->spinPQDiff, "kv.pqDiff", -1);
    bindSpinBoxState(ui->spinEBits, "kv.eBits", 0);

    // 绑定 nBits 变化时的 UI 联动
    connect(ui->spinNBits, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &DriverDialog::onSpinNBitsChanged);

    // 👈 核心：因为 QSettings 在读取时可能不触发 valueChanged，
    // 所以程序初始化时，手动调用一次，刷新 UI 的 label 显示
    onSpinNBitsChanged(ui->spinNBits->value());
}

void DriverDialog::setupDropOverlay() // 在构造函数里调用
{
    dropOverlay = new DropOverlay(this);
    dropOverlay->setGeometry(this->rect());
    this->setAcceptDrops(true);
}

// 拖入
void DriverDialog::dragEnterEvent(QDragEnterEvent *event)
{
    // 1. 基本校验：拖进来的必须是本地文件 URL
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }
    int currentIndex = ui->comboBox->currentIndex();
    // 2. 如果当前在 Settings 页（索引 0）
    if (currentIndex == 0) {
        // 👇 核心拦截：只有已锁定（Locked）才允许拖入
        if (currentLicenseState != LicenseState::Locked) {
            event->ignore(); // 拒绝拖拽，鼠标会显示禁止图标
            return;          // 直接退出，不显示遮罩层
        }
    }
    // 3. 其他页面的逻辑（比如 Files/Folder 页面）
    else if (currentIndex == 2 || currentIndex == 3) {
        // 如果没有特殊状态限制，直接放行
    }
    // 4. 其他不需要拖拽的页面（如 Keys 页）
    else {
        event->ignore();
        return;
    }

    qDebug() << "=== 触发拖拽进入 ===";
    // 5. 校验通过，显示遮罩层
    event->acceptProposedAction();

    // 👇 根据当前选中的下拉框页面，切换 Overlay 的模式
    int index = ui->comboBox->currentIndex();
    if (index == 0) {
        dropOverlay->setMode("settings");
    } else if (index == 2) {
        dropOverlay->setMode("files");
    } else if (index == 3) {
        dropOverlay->setMode("folder");
    }

    dropOverlay->setGeometry(this->rect());
    dropOverlay->show();
    dropOverlay->raise();
}

// 在内部移动
void DriverDialog::dragMoveEvent(QDragMoveEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    QPoint pos = event->position().toPoint();
#else
    QPoint pos = event->pos();

#endif
    // 更新高亮，并记录当前悬停的目标
    currentDropTarget = dropOverlay->updateHighlight(pos);
    event->acceptProposedAction();
}

// 离开
void DriverDialog::dragLeaveEvent(QDragLeaveEvent *event)
{
    dropOverlay->hide();
    currentDropTarget.clear();
    event->accept();
}

// 释放
void DriverDialog::dropEvent(QDropEvent *event)
{
    dropOverlay->hide();

    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty() || currentDropTarget.isEmpty()) return;

    QStringList files;
    for (const QUrl &url : urls) {
        files << url.toLocalFile();
    }

    // 根据 currentDropTarget 分发业务逻辑
    if (currentDropTarget == "all" && ui->comboBox->currentIndex() == 0) {
        processSigdDrop(files, "解锁");
    } else if (currentDropTarget == "top" && ui->comboBox->currentIndex() == 2) {
        processFilesDrop(files, "加密文件");
    } else if (currentDropTarget == "bottom" && ui->comboBox->currentIndex() == 2) {
        processFilesDrop(files, "解密文件");
    } else if (currentDropTarget == "encrypt") {
        processFolderDrop(files, "加密目录");
    } else if (currentDropTarget == "zip") {
        processFolderDrop(files, "压缩目录");
    } else if (currentDropTarget == "decrypt") {
        processFolderDrop(files, "解密目录");
    } else if (currentDropTarget == "unzip") {
        processFolderDrop(files, "解压目录");
    }

    currentDropTarget.clear();
}

void DriverDialog::processSigdDrop(const QStringList &files, const QString &action)
{
    if (action != "解锁") {
        qDebug() << "未知动作:" << action;
    }
    // 1. 确保有文件传入
    if (files.isEmpty()) return;

    // 2. 只取第一个文件
    QString firstFilePath = files.first();
    QFileInfo fileInfo(firstFilePath);

    // 3. 校验：必须是文件，不能是目录（或其它特殊文件类型）
    if (!fileInfo.isFile()) {
        // 弹出与 Mac 版一致的错误提示
        QMessageBox::warning(this,
                             "无法处理 / Unable to Process",
                             "只接受文件，不接受目录或其它类型。请拖入单个文件。\nOnly files are accepted, not folders. Please drop a single file."
                             );
        return; // 拦截，不再执行后续操作
    }

    // 拼装命令：sige <文件路径>
    // 使用双引号包裹路径，防止路径中包含空格导致命令解析失败
    QString command = QString("sigd %1").arg(escapeForShell(firstFilePath));

    qDebug() << "[Signature] Unlock with file:" << firstFilePath;
    qDebug() << "执行命令:" << command;

    // 5. 在新终端中执行命令
    // 使用 gnome-terminal，并在命令结束后保留终端窗口（exec bash）
    runInTerminal(command);

    // 可选：由于终端是独立运行的，主界面不知道命令何时执行完毕。
    // 可以延迟 2 秒后自动刷新一次状态（红绿灯会自动改变）
    // QTimer::singleShot(2000, this, &DriverDialog::checkLicenseStatus);
}
void DriverDialog::processFilesDrop(const QStringList &files, const QString &action)
{
    QString ed;
    if (action == "加密文件") {
        ed = "-e";
    } else if (action == "解密文件") {
        ed = "-d";
    } else {
        qDebug() << "未知动作:" << action;
        return;
    }

    // 校验：必须是文件，不能是目录（或其它特殊文件类型）
    for (const QString &file : files) {
        QFileInfo fileInfo(file);
        if (!fileInfo.isFile()) {
            // 弹出与 Mac 版一致的错误提示
            QMessageBox::warning(this,
                                 "无法处理 / Unable to Process",
                                 "只接受文件，不接受目录或其它类型。请拖入单个文件。\nOnly files are accepted, not folders. Please drop a single file."
                                 );
            return; // 拦截，不再执行后续操作
        }
    }

    QString command("sxrsa.sw ");
    if ( ui->checkBoxOverwriteD->checkState() ) command += "--overwrite ";
    command += (ed +ui->keyPickerFiles->currentKeyId());
    for (const QString &file : files) {
        command += (" " + escapeForShell(file) );
    }

    qDebug() << "执行命令:" << command;
    runInTerminal(command);
}
void DriverDialog::processFolderDrop(const QStringList &folder, const QString &action)
{
    for (const QString &f : folder) {
        QFileInfo fileInfo(f);
        if (!fileInfo.isDir()) {
            // 弹出与 Mac 版一致的错误提示
            QMessageBox::warning(this,
                                 "无法处理 / Unable to Process",
                                 "检测到文件。这里只接受目录，请把文件拖到“文件”标签页处理。\nFile detected. Only folders are accepted here. Please drop files in the Files tab."
                                 );
            return; // 拦截，不再执行后续操作
        }
    }

    QString command, ed;
    if (action == "加密目录") {
        command = "dxrsa.sw";
        ed = QStringLiteral(" -e") + ui->keyPickerFolder->currentKeyId();
    } else if (action == "解密目录") {
        command = "dxrsa.sw";
        ed = QStringLiteral(" -d") + ui->keyPickerFolder->currentKeyId();
    } else if (action == "压缩目录") {
        command = "ezip";
    } else if (action == "解压目录") {
        command = "eunzip";
    } else {
        qDebug() << "未知动作:" << action;
        return;
    }
    if (ui->checkBoxRecursive->checkState())    command+=QStringLiteral(" --recursive");
    if (ui->checkBoxOverwriteD->checkState())    command+=QStringLiteral(" --overwrite");
    command += ed + QStringLiteral(" ");

    for (const QString &f : folder) {
        QString cmd = command + escapeForShell(f) + QStringLiteral(" ") + ui->recentInputWidget->text();
        qDebug() << "执行命令:" << cmd;
        runInTerminal(cmd);
    }
}
void DriverDialog::onKeyPairNameChanged(const QString &text)
{
    int length = text.length();

    // 更新 Label 文字
    ui->labelNameLength->setText(QString("%1/18").arg(length));

    // 根据长度变色（对应你 Mac 版里的逻辑）
    if (length > 18) {
        ui->labelNameLength->setStyleSheet("color: red;");
    } else if (length == 18) {
        ui->labelNameLength->setStyleSheet("color: #E0AA00;"); // 亮黄色警告
    } else {
        ui->labelNameLength->setStyleSheet("color: gray;"); // 默认灰色
    }
}
void DriverDialog::onSpinNBitsChanged(int value)
{
    qDebug() << "[N]" << value;
    if (value<5) {
        ui->labelNLimit->setStyleSheet("color: red;");
    } else {
        ui->labelNLimit->setStyleSheet("color: gray;");
    }
    ui->spinPQDiff->setMaximum(value-1);
    ui->labelDiffLimit->setText("(-1; 0 ~ " + QString::number(value-1) + ")");
    if (ui->spinPQDiff->value()>value-1) {
        ui->labelDiffLimit->setStyleSheet("color: red;");
    } else {
        ui->labelDiffLimit->setStyleSheet("color: gray;");
    }
    ui->spinEBits->setMaximum(value);
    ui->labelELimit->setText("(0; 1 ~ " + QString::number(value) + ")");
    if (ui->spinEBits->value()>value) {
        ui->labelELimit->setStyleSheet("color: red;");
    } else {
        ui->labelELimit->setStyleSheet("color: gray;");
    }
}
// 绑定 Lock/Unlock 按钮
void DriverDialog::on_pushButtonLock_clicked()
{
    // 假设 Linux 下的命令也是 sige
    runInTerminal("sige");
    // 可选：延迟几秒钟后重新检测状态
    // QTimer::singleShot(3000, this, &DriverDialog::checkLicenseStatus);
}
void DriverDialog::on_pushButtonUnlock_clicked()
{
    runInTerminal("sigd");
}

void DriverDialog::on_pushButtonVer_clicked()
{
    runInTerminal("xrsaoapp.sw \\?; xrsaoapp.sw -\\?");
}
void DriverDialog::on_pushButtonDamageRM_clicked()
{
    runInTerminal("sigrm");
}
void DriverDialog::on_pushButtonGenerate_clicked()
{
    if (ui->KeyPairName->text().length()<=0) {
        QMessageBox::warning(this,
                             "无法处理 / Unable to Process",
                             "缺少密钥对命名 / No key pair name"
                             );
        return;
    }

    QString appPath = QStandardPaths::findExecutable("npeminicp");
    if (appPath.isEmpty()) {
        QMessageBox::warning(this,
                             "无法处理 / Unable to Process",
                             "未安装 Uninstalled"
                             );
        return;
    }
    QFileInfo fileInfo(appPath);
    QString realPath = fileInfo.canonicalFilePath();
    if (realPath.isEmpty()) {
        QMessageBox::warning(this,
                             "无法处理 / Unable to Process",
                             "未安装 Uninstalled"
                             );
        return;
    }

    // 对应 `dirname`：获取父目录
    // QFileInfo(realPath).absolutePath() 会直接返回 /mnt/.../xrsa_shareware
    QString rootDir = QFileInfo(realPath).absolutePath();

    QString command;
    command = QStringLiteral("xrsaoapp.sw -k") + ui->KeyPairName->text()
              + QStringLiteral(" ") + escapeForShell(rootDir)
              + QStringLiteral(" -b") + QString::number(ui->spinNBits->value());
    int n = ui->spinPQDiff->value();
    if (n>=0)   command += QStringLiteral(" -0") + QString::number(n);
    n = ui->spinEBits->value();
    if (n>0)   command += QStringLiteral(" -m") + QString::number(n);
    if (ui->checkBoxFast->checkState()) command += " -t";
    qDebug() << "[Generate] " << command;
    runInTerminal(command);
}
void DriverDialog::on_keychainKey_clicked()
{
    QString command;
    command = QStringLiteral("npeminicp; xrsaoapp.sw -k") + ui->KeyPairName->text();
    runInTerminal(command);
}
void DriverDialog::on_multiCore_clicked()
{
    runInTerminal("xcompensate");
}
void DriverDialog::on_keychainFiles_clicked()
{
    if (ui->checkBoxResign->checkState()) {
        runInTerminal( "sigx " + ui->keyPickerFiles->currentKeyId() );
        return;
    }
    if (ui->checkBoxRecover->checkState()) {
        runInTerminal( "xrsaoapp.sw -x" + ui->keyPickerFiles->currentKeyId() );
        return;
    }
    runInTerminal("npeminicp; xrsaoapp.sw");
}
void DriverDialog::on_keychainFolder_clicked()
{
    runInTerminal("npeminicp; xrsaoapp.sw");
}
void DriverDialog::on_rsaKeeperF_clicked()
{
    runInTerminal("rsa-keeper");
}
void DriverDialog::on_rsaKeeperD_clicked()
{
    runInTerminal("rsa-keeper");
}
void DriverDialog::on_configKeeper_clicked()
{
    runInTerminal("config-keeper");
}
void DriverDialog::on_pushButtonSMX_clicked()
{
    runInTerminal("dcp; ssmx; dsmx; smx-configure 0");
}
void DriverDialog::handleNewConnection()
{
    QLocalSocket *socket = localServer->nextPendingConnection();
    if (!socket) return;

    // 收到新连接（说明有人第二次启动了程序）
    socket->waitForReadyRead(100);
    QString message = QString::fromUtf8(socket->readAll());
    socket->disconnectFromServer();
    socket->deleteLater();

    if (message == "show") {
        // 1. 如果窗口被最小化了，恢复它
        if (this->isMinimized()) {
            this->showNormal();
        }
        // 2. 如果窗口被隐藏了（比如通过关闭按钮），恢复它
        else if (this->isHidden()) {
            this->show();
        }

        // 3. 👈 关键：直接使用 setWindowState 激活窗口，而不是调用 show()
        // Qt::WindowActive 会要求窗口管理器将其激活并置顶，且不会改变它的大小和位置
        this->setWindowState((this->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);

        // 4. 双保险：强行提到最前并请求焦点
        this->raise();
        this->activateWindow();
    }
}

DriverDialog::~DriverDialog()
{
    delete ui;
}

