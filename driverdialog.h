#ifndef DRIVERDIALOG_H
#define DRIVERDIALOG_H

#include <QWidget>
#include <QToolButton>
#include <QScrollArea>
#include <QTextEdit>
#include <QPropertyAnimation>

#include <QDialog>
#include "refreshbutton.h"
#include "dropoverlay.h"

#include <QLocalServer>
#include <QLocalSocket>

enum class LicenseState {
    Uninstalled,  // 未安装（软连接失效）
    Locked,       // 已安装，未解锁
    Unlocked,     // 已解锁
    Unknown       // 检测中/出错
};

namespace Ui {
class DriverDialog;
}

class DriverDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DriverDialog(QWidget *parent = nullptr);
    ~DriverDialog();

private:
    void checkLicenseStatus();
    void updateLicenseUI(LicenseState state);
    void stackedWedgetSelect(int index);
private:
    QToolButton *rawOutputToggle;   // 展开按钮
    QScrollArea *rawOutputArea;     // 滚动区域
    QTextEdit *rawOutputTextEdit;   // 显示内容的文本框
    QPropertyAnimation *rawAnim;    // 动画对象
    void setupRawOutputUI();
    void initValidators();
    void initPageSwitching();
    void initKeyPickers();
    void initCheckBoxLineEdit();
    void initSpinBoxes();
    QLocalServer * localServer;
private:
    LicenseState currentLicenseState = LicenseState::Unknown;
    DropOverlay *dropOverlay;
    QString currentDropTarget; // 记录当前悬停的区域
    void setupDropOverlay();
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);
    void processSigdDrop(const QStringList &files, const QString &action);
    void processFilesDrop(const QStringList &files, const QString &action);
    void processFolderDrop(const QStringList &folder, const QString &action);
private slots:
    void handleNewConnection();
    void onKeyPairNameChanged(const QString &text);
    void onSpinNBitsChanged(int value);
    void on_pushButtonLock_clicked();
    void on_pushButtonUnlock_clicked();
    void on_pushButtonVer_clicked();
    void on_pushButtonDamageRM_clicked();
    void on_pushButtonGenerate_clicked();
    void on_keychainKey_clicked();
    void on_multiCore_clicked();
    void on_keychainFiles_clicked();
    void on_keychainFolder_clicked();
    void on_rsaKeeperF_clicked();
    void on_rsaKeeperD_clicked();
    void on_configKeeper_clicked();
    void on_pushButtonSMX_clicked();

private:
    Ui::DriverDialog *ui;
};

#endif // DRIVERDIALOG_H
