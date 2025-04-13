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
    int getCpuTemperature(void);
    float getCPULoad(void);
    void getMemoryUsage(long &totalMem, long &freeMem);

private:
    Ui::Dialog *ui;
    std::vector<std::function<void()>> functions;
    const char *customMessage;
    QByteArray utf8Bytes;
    void callFunctions(const std::vector<int> &indices);
    void addFunction(std::function<void()> func);
    void submitFunction(void);
    void clearDisplayFunction(void);

    void printMemoryUsage(void);
    void printCpuLoad(void);
    void printCpuTemperature(void);
    void printCustomMessage(void);
    void printToScreen(const char *text);;;
};
#endif // DIALOG_H
