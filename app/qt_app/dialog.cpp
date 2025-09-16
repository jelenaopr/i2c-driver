#include "dialog.h"
#include "ui_dialog.h"
#include <cmath>

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
    ui->pushButton_2->setText("Clear display");

    connect(ui->pushButton, &QPushButton::clicked, this, &Dialog::submitFunction);
    connect(ui->pushButton_2,&QPushButton::clicked, this, &Dialog::clearDisplayFunction);

    addFunction(std::bind(&Dialog::printCpuTemperature, this));
    addFunction(std::bind(&Dialog::printCpuLoad, this));
    addFunction(std::bind(&Dialog::printMemoryUsage, this));
    addFunction(std::bind(&Dialog::printCustomMessage, this));
}

void Dialog::addFunction(std::function<void()> func) {
    functions.push_back(func);
}

void Dialog::callFunctions(const std::vector<int> &indices){
    std::cout<<"call fundtions called"<<std::endl;
    for(int index: indices) {
        if(index < functions.size()) {
            std::cout<<"call fundtion at  index"<<index<<std::endl;
            functions[index]();
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
    std::cout<<"done"<<std::endl;
}

void Dialog::clearDisplayFunction(void) {
    int fd;
    running.store(false);

    if(worker.joinable()) {
        worker.join();
    }

    qDebug() << "Dialog: clearing display " << "\n";

    fd = ::open("/dev/i2c_device", O_RDWR);
    if(fd < 0) {
        printf("Cannot open device file...\n");
    }

    if (ioctl(fd, IOCTL_CMD_CLEAR, 0) == -1) {
        perror("ioctl failed");
        messageBox.setText("IOCTL failed!");
        messageBox.exec();
    }

    indices.clear();
    ::close(fd);
}

void Dialog::printMemoryUsage(void) {
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    char *message;
    long freeMemory;
    while (getline(meminfo, line)) {
        std::istringstream iss(line);
        std::string key;
        long value;
        std::string unit;
        iss >> key >> value >> unit;
        if (key == "MemAvailable:") {
            freeMemory = value;
            break;
        }
    }
    sprintf(message, "Free memory: %ld", freeMemory);
    std::cout<<"print memory usage"<<std::endl;
    printToScreen(message);
}

void Dialog::printCpuLoad(void) {
    std::ifstream loadavg("/proc/loadavg");
    float load;
    std::string message;
    loadavg >> load;

    if(load < 4) {
        message = "CPU load: underloaded";
    }
    else if(load == 4) {
       message = "CPU load: balanced";
    } else {
        message = "CPU load: overloaded";
    }
    printToScreen(message.c_str());
}

void Dialog::printCpuTemperature(void) {

    char *message;
    std::fstream tempFile("/sys/class/thermal/thermal_zone0/temp");
    float temperature = 0.0;
    if(tempFile.is_open()) {
        std::string line;
        std::getline(tempFile,line);
        temperature = std::stof(line)/1000;
         qDebug() << "Dialog: CPU h temperature is: " << temperature << "\n";
    }
    tempFile.close();
    sprintf(message, "CPU temperature: %.2fC", temperature);
    printToScreen(message);
}

void Dialog::printCustomMessage(void) {
    printToScreen(customMessage);
}


void Dialog::printToScreen(const char *text) {
    int fd;
    
    qDebug() << "Dialog: printToScreen: " << text << "\n";

    fd = ::open("/dev/i2c_device", O_RDWR);
    if(fd < 0) {
        messageBox.setText("Cannot open device file, driver not loaded!");
        messageBox.exec();
        return;
    }
    if (ioctl(fd, IOCTL_CMD_CLEAR, 0) == -1) {
        perror("ioctl failed");
        messageBox.setText("IOCTL failed!");
        messageBox.exec();
    }
    if (ioctl(fd, IOCTL_CMD_WRITE, text) == -1) {
        perror("ioctl failed");
        messageBox.setText("IOCTL failed!");
        messageBox.exec();
    }
    ::close(fd);
}

void Dialog::runTask()
{
    while(running.load())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        callFunctions(indices);
    }
}

void Dialog::submitFunction(void)
{
    QString message;
    char brightnessMessage[24];

    int brightness = ui->horizontalSlider->value();

    if(brightness != 0) {
        int scaled = (brightness * 255 + 99)/100;
        sprintf(brightnessMessage, "brightness=%d", scaled);
        printToScreen(brightnessMessage);
    } else {
        messageBox.setText("Brightness cannot be set to zero, skipping!");
        messageBox.exec();
    }

    if(ui->checkBox->isChecked())
    {
        message = ui->lineEdit->text();
        if(!(message.isEmpty()))
        {
            utf8Bytes = message.toUtf8();
            customMessage = utf8Bytes.constData();
            indices.push_back(3);
        }
        else
        {
            messageBox.setText("Custom message box cannot be empty, skipping!");
            messageBox.exec();
        }
    }
    if(ui->checkBox_2->isChecked()) indices.push_back(2);
    if(ui->checkBox_3->isChecked()) indices.push_back(1);
    if(ui->checkBox_4->isChecked()) indices.push_back(0);

    worker = std::thread(&Dialog::runTask, this);
    running.store(true);
}

Dialog::~Dialog()
{
    delete ui;   
}
