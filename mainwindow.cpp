#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <netdb.h>
#include <cstring>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setEnabled(false);
    _conf = new Config(this);
    _conf->show();
    
    connect(_conf, &Config::done, this, &MainWindow::test);
    connect(_conf, &Config::rejected, this, [=] () {
        _conf->hide();
        _conf->close();
        this->close();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::start() {
    // Create proxy
    int sockfd, newsockfd;

    struct sockaddr_in serv_addr;
    struct sockaddr cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // create an IPv4 TCP socket
    if (sockfd < 0) {
        qDebug() << "SORRY! Cannot create a socket !";
        this->close();
        return;
    }

    memset(&serv_addr, 0, sizeof serv_addr);

    serv_addr.sin_family = AF_INET;     // ip4 family
    serv_addr.sin_addr.s_addr = INADDR_ANY;  // represents for localhost i.e 127.0.0.1
    serv_addr.sin_port = htons(_port);


    int binded = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    if (binded < 0) {
        qDebug() << "Error on binding!";
        this->close();
        return;
    }

    listen(sockfd, 100);  // can have maximum of 100 browser requests

    int clilen = sizeof(struct sockaddr);
    while (true) {
        qDebug() << "Test";
        /* A browser request starts here */
        newsockfd = accept(sockfd, &cli_addr, (socklen_t*) &clilen);
        if (newsockfd <0){
            qDebug() << "ERROR! On Accepting Request ! i.e requests limit crossed";
            continue;
        }

        /*int pid = fork();
        if(pid == 0){
            datafromclient((void*)&newsockfd);
            close(newsockfd);
            _exit(0);
        } else {
            close(newsockfd);     // pid =1 parent process
        }*/
        qDebug() << "Test2";
    }
}

void MainWindow::test(int port)
{
    qDebug() << port;
    _conf->hide();
    _conf->close();
    _port = port;

    start();
}


char* MainWindow::convert_Request_to_string(struct ParsedRequest *req)
{
    /* Set headers */
//    ParsedHeader_set();
    ParsedHeader_set(req, "");
    ::ParsedHeader_set(req, "Connection", "close");

    int iHeadersLen = ::ParsedHeader_headersLen(req);

    char *headersBuf;

    headersBuf = (char*) malloc(iHeadersLen + 1);

    if (headersBuf == NULL) {
        fprintf (stderr," Error in memory allocation  of headersBuffer ! \n");
        exit (1);
    }


    ParsedRequest_unparse_headers(req, headersBuf, iHeadersLen);
    headersBuf[iHeadersLen] = '\0';


    int request_size = strlen(req->method) + strlen(req->path) + strlen(req->version) + iHeadersLen + 4;

    char *serverReq;

    serverReq = (char *) malloc(request_size + 1);

    if (serverReq == NULL) {
        fprintf (stderr," Error in memory allocation for serverrequest ! \n");
        exit (1);
    }

    serverReq[0] = '\0';
    strcpy(serverReq, req->method);
    strcat(serverReq, " ");
    strcat(serverReq, req->path);
    strcat(serverReq, " ");
    strcat(serverReq, req->version);
    strcat(serverReq, "\r\n");
    strcat(serverReq, headersBuf);

    free(headersBuf);

    return serverReq;
}

int MainWindow::createserverSocket(char *pcAddress, char *pcPort) {
    struct addrinfo ahints;
    struct addrinfo *paRes;

    int iSockfd;

    /* Get address information for stream socket on input port */
    memset(&ahints, 0, sizeof(ahints));
    ahints.ai_family = AF_UNSPEC;
    ahints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(pcAddress, pcPort, &ahints, &paRes) != 0) {
        fprintf (stderr," Error in server address format ! \n");
        exit (1);
    }

    /* Create and connect */
    if ((iSockfd = socket(paRes->ai_family, paRes->ai_socktype, paRes->ai_protocol)) < 0) {
        fprintf (stderr," Error in creating socket to server ! \n");
        exit (1);
    }
    if (::connect(iSockfd, paRes->ai_addr, paRes->ai_addrlen) < 0) {
        fprintf (stderr," Error in connecting to server ! \n");
        exit (1);
    }

    /* Free paRes, which was dynamically allocated by getaddrinfo */
    freeaddrinfo(paRes);
    return iSockfd;
}

void MainWindow::writeToserverSocket(const char* buff_to_server,int sockfd,int buff_length)
{
    std::string temp;
    temp.append(buff_to_server);

    int totalsent = 0;
    int senteach;

    while (totalsent < buff_length) {
        if ((senteach = send(sockfd, (void *) (buff_to_server + totalsent), buff_length - totalsent, 0)) < 0) {
            fprintf (stderr," Error in sending to server ! \n");
            exit (1);
        }
        totalsent += senteach;
    }
}

void MainWindow::writeToclientSocket(const char* buff_to_server,int sockfd,int buff_length)
{
    std::string temp;
    temp.append(buff_to_server);

    int totalsent = 0;
    int senteach;

    while (totalsent < buff_length) {
        if ((senteach = send(sockfd, (void *) (buff_to_server + totalsent), buff_length - totalsent, 0)) < 0) {
            fprintf (stderr," Error in sending to server ! \n");
            exit (1);
        }
        totalsent += senteach;
    }
}

void MainWindow::writeToClient (int Clientfd, int Serverfd) {
    int MAX_BUF_SIZE = 5000;
    int iRecv;
    char buf[MAX_BUF_SIZE];

    while ((iRecv = recv(Serverfd, buf, MAX_BUF_SIZE, 0)) > 0) {
          writeToclientSocket(buf, Clientfd,iRecv);         // writing to client
        memset(buf,0,sizeof buf);
    }

    /* Error handling */
    if (iRecv < 0) {
        fprintf (stderr,"Yo..!! Error while recieving from server ! \n");
      exit (1);
    }
}

void* MainWindow::datafromclient(void* sockid)
{
    int MAX_BUFFER_SIZE = 5000;
    char buf[MAX_BUFFER_SIZE];
    int newsockfd = *((int*)sockid);
    char *request_message;  // Get message from URL

    request_message = (char *) malloc(MAX_BUFFER_SIZE);

    if (request_message == NULL) {
        fprintf (stderr," Error in memory allocation ! \n");
        exit (1);
    }

    request_message[0] = '\0';
    int total_recieved_bits = 0;

    while (strstr(request_message, "\r\n\r\n") == NULL) {  // determines end of request
      int recvd = recv(newsockfd, buf, MAX_BUFFER_SIZE, 0) ;

      if(recvd < 0 ){
        fprintf (stderr," Error while recieving ! \n");
        exit (1);
      }else if(recvd == 0) {
            break;
      } else {
        total_recieved_bits += recvd;

        /* if total message size greater than our string size,double the string size */

        buf[recvd] = '\0';
        if (total_recieved_bits > MAX_BUFFER_SIZE) {
            MAX_BUFFER_SIZE *= 2;
            request_message = (char *) realloc(request_message, MAX_BUFFER_SIZE);
            if (request_message == NULL) {
                fprintf (stderr," Error in memory re-allocation ! \n");
                exit (1);
            }
        }
      }
      strcat(request_message, buf);
    }

    struct ::ParsedRequest *req;    // contains parsed request

    req = ::ParsedRequest_create();

    if (::ParsedRequest_parse(req, request_message, strlen(request_message)) < 0) {
        fprintf (stderr,"Error in request message..only http and get with headers are allowed ! \n");
        exit(0);
    }

    if (req->port == NULL) {            // if port is not mentioned in URL, we take default as 80
         req->port = (char *) "80";
    }


    /*final request to be sent*/

    char*  browser_req  = convert_Request_to_string(req);

    int iServerfd;

    iServerfd = createserverSocket(req->host, req->port);

    /* To run on IIIT proxy, comment above line and uncomment below line */

    //char iiitproxy[] = "172.31.1.4";
    //char iiitport[] = "8080";

    // iServerfd = createserverSocket(iiitproxy, iiitport);

    writeToserverSocket(browser_req, iServerfd, total_recieved_bits);
    writeToClient(newsockfd, iServerfd);


    ::ParsedRequest_destroy(req);

    ::shutdown(newsockfd, 2);   // close the sockets
    ::shutdown(iServerfd, 2);

    //  Doesn't make any sense ..as to send something
    int y = 3;
    int *p = &y;
    return p;
}

