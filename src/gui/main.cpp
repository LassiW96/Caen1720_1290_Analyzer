#include <QApplication>
#include "MainWindow.h"
#include <TApplication.h>

int main(int argc, char *argv[]) {
    // Initialize Qt Application
    QApplication app(argc, argv);

    // Initialize ROOT Application
    TApplication rootApp("ROOTApp", &argc, argv); 
    
    MainWindow window;
    window.show();

    return app.exec();
}
