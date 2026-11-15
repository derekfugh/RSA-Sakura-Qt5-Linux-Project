#include "mainwindow.h"
#include "driverdialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationName("RSA-Sakura");
    a.setApplicationName("Driver");

    // 1. 尝试连接已经在运行的服务端
    QLocalSocket socket;
    socket.connectToServer("RSA-Sakura-Driver-Server");

    if (socket.waitForConnected(500)) {
        // 2. 如果连上了，说明程序已经在运行
        // 发送一个 "show" 消息，让主窗口弹出来
        socket.write("show");
        socket.waitForBytesWritten(500);
        socket.disconnectFromServer();

        // 然后自己悄悄退出
        return 0;
    }

    //MainWindow w;
    DriverDialog w;
    w.show();
    return a.exec();
}
