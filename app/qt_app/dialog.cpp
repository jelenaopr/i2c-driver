#include "dialog.h"
#include "ui_dialog.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

#define IOCTL_CMD_WRITE 0x00
#define IOCTL_CMD_CLEAR 0x01

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    ui->pushButton->setText("Submit");

    connect(ui->pushButton, &QPushButton::clicked, this, &Dialog::submitFunction);
}
//Hello!
void Dialog::submitFunction(void)
{
    QString name;
    const char* cstr;
    int fd;
    
    name = ui->lineEdit->text();
    
    qDebug() << "Dialog: Entered Location: " << name << "\n";

    cstr = name.toUtf8().constData();

    fd = ::open("/dev/i2c_device", O_RDWR);
    if(fd < 0) {
        printf("Cannot open device file...\n");
    }

    if (ioctl(fd, IOCTL_CMD_CLEAR, 0) == -1) {
        perror("ioctl failed");
        ::close(fd);    } 

    if (ioctl(fd, IOCTL_CMD_WRITE, cstr) == -1) {
        perror("ioctl failed");
        ::close(fd);
    }  

    ::close(fd);
}

Dialog::~Dialog()
{
    delete ui;
}

