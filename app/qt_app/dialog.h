#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "iostream"
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <cmath>

#include <thread>
#include <atomic>

#include <fcntl.h>      // open()
#include <unistd.h>     // close()
#include <sys/ioctl.h>  // ioctl()
#include <cstring>      // strcpy()
#include "fstream"
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private:
    Ui::Dialog *ui;
    std::vector<std::function<void()>> functions;
    std::vector<int> indices;
    QByteArray utf8Bytes;
    std::thread worker;
    const char *customMessage;
    std::atomic<bool> running {false};

    void runTask();

    void callFunctions(const std::vector<int> &indices);
    void addFunction(std::function<void()> func);
    void submitFunction(void);
    void clearDisplayFunction(void);

    void printMemoryUsage(void);
    void printCpuLoad(void);
    void printCpuTemperature(void);
    void printToScreen(const char *text);
    void printCustomMessage(void);
};
#endif // DIALOG_H
