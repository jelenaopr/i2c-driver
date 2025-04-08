#include "dialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;
    w.show();

        int temp = w.getCpuTemperature();
    std::cout << "CPU temperature is "<<temp<<std::endl;
    
        // Memory usage
    long totalMem = 0, freeMem = 0;
    w.getMemoryUsage(totalMem, freeMem);
    std::cout << "Total Memory: " << totalMem / 1024 << " MB" << std::endl;
    std::cout << "Available Memory: " << freeMem / 1024 << " MB" << std::endl;

    // CPU Load
    float load = w.getCPULoad();
    std::cout << "CPU Load (1 min avg): " << load << std::endl;
    return a.exec();
}
