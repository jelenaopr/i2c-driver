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
    void submitFunction(void);
};
#endif // DIALOG_H
