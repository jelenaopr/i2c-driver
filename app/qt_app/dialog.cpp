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
}

void Dialog::addFunction(std::function<void()> func) {
    functions.push_back(func);
}

void Dialog::callFunctions(const std::vector<int> &indices){
    for(int index: indices) {
        if(index < functions.size()) {
            std::cout<<"call func with index: "<<index<<std::endl;
            functions[index]();
            sleep(5);

        }
    }
}

void Dialog::clearDisplayFunction(void) { 
    qDebug() << "Dialog: clear display: " << "\n";    
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

    if(load < 1) {
        message = "CPU load: underloaded";
    }
    else if(load == 1) {
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
    int roundedTemp = 0;
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

}


void Dialog::printToScreen(const char *text) {

    int fd;
    
    qDebug() << "Dialog: Entered Location: " << text << "\n";
    std::cout<<text<<std::endl;
    
    fd = ::open("/dev/i2c_device", O_RDWR);
    if(fd < 0) {
        printf("Cannot open device file...\n");
    }

    if (ioctl(fd, IOCTL_CMD_CLEAR, 0) == -1) {
        perror("ioctl failed");
        ::close(fd);    } 

    if (ioctl(fd, IOCTL_CMD_WRITE, text) == -1) {
        perror("ioctl failed");
        ::close(fd);
    }  

    ::close(fd);
}

//Hello! \n
void Dialog::submitFunction(void)
{
    QString name;
    const char* cstr;
    int fd;
    bool isChecked;
    std::vector<int> indices;
    
    name = ui->lineEdit->text();
    isChecked = ui->checkBox->isChecked();

    isChecked = ui->checkBox_2->isChecked();
    if(isChecked)
        indices.push_back(2);

    isChecked = ui->checkBox_3->isChecked();
    if(isChecked)
        indices.push_back(1);

    isChecked = ui->checkBox_4->isChecked();
    if(isChecked)
        indices.push_back(0);


    QByteArray utf8Bytes = name.toUtf8();
    cstr = utf8Bytes.constData();

     size_t length = strlen(cstr);
    if (cstr[length] == '\0') {
        std::cout<<"it is null"<<std::endl;
    }

    callFunctions(indices);

}

Dialog::~Dialog()
{
    
    delete ui;
    
}

