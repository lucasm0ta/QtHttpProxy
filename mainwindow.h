#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "config.h"
#include "proxy_parse.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void test(int port);
    char* convert_Request_to_string(struct ParsedRequest *req);
    int createserverSocket(char *pcAddress, char *pcPort);
    void writeToserverSocket(const char* buff_to_server,int sockfd,int buff_length);
    void writeToclientSocket(const char* buff_to_server,int sockfd,int buff_length);
    void writeToClient (int Clientfd, int Serverfd);
    void* datafromclient(void* sockid);

private:
    Ui::MainWindow *ui;
    Config *_conf;
    int _port;

    void start();
};

#endif // MAINWINDOW_H
