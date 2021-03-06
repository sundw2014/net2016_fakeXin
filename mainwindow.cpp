#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QStringListModel>
#include "common.h"
#include "updatefriendslistthread.h"
#include <QMessageBox>
#include <QTcpServer>
#include "sessionworkerthread.h"
#include <QUrl>
#include <QDebug>

static const QString _friendsListFilename = ":/list/friends.txt";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    friendsListModel = nullptr;
    QStringList *friendList = loadFriends(_friendsListFilename);
    friendsListModel = new QStringListModel(*friendList, NULL);
    ui->listView_friends->setModel(friendsListModel);

    tcpServer = new QTcpServer(this);
    tcpServer->listen(QHostAddress::Any,clientPort);
    connect(tcpServer,SIGNAL(newConnection()), this, SLOT(newConnect()));

    (new UpdateFriendsListThread(this,serverIP,serverPort))->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QStringList* MainWindow::loadFriends(const QString friendsListFilename)
{
    QStringList *friendsList = new QStringList();
    QFile inputFile(friendsListFilename);
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       in.setCodec("UTF-8");
       while (!in.atEnd())
       {
          QString line = in.readLine();
          if(line.mid(0,10) == usrName)
          {
              myName = line.mid(11,line.size()-11);
          }
          friendsList->append(line);
       }
       inputFile.close();
    }
//    qDebug()<<inputFile.errorString();
    return friendsList;
}

void MainWindow::on_listView_friends_doubleClicked(const QModelIndex &index)
{
    QStringList friendsSplited = index.data(Qt::DisplayRole).toString().split('\t');
    if(friendsSplited.size()==3){
        emit newSession(friendsSplited);
    }
    else
    {
        QString ErrMsg = "not online";
        QMessageBox::warning(this,"message",ErrMsg,QMessageBox::Yes);
    }
}

void MainWindow::newConnect(){
    QTcpSocket *tcpSocket =  tcpServer->nextPendingConnection();
    SessionWorkerThread *sessionThread = new SessionWorkerThread(tcpSocket);
    emit newSession(sessionThread);
}

void MainWindow::on_pushButton_newGroupSession_clicked()
{
    emit newGroupSession();
}
