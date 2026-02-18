#include <QApplication>
#include "MainWindow.h"
#include <TApplication.h>

int main(int argc, char *argv[]) {
    // Initialize Qt Application
    QApplication app(argc, argv);

    // Initialize ROOT Application (needed for canvas handling mechanisms even if embedded or separate)
    // passing 0, nullptr to avoid ROOT grabbing command line args that might be meant for Qt or vice versa,
    // though usually handled okay.
    // TApplication rootApp("ROOTApp", &argc, argv); 
    // We might not strictly need TApplication if we just open TCanvas, but it's safer for event loop integration usually.
    // For now, let's stick to standard Qt loop and see if we need TApplication for the interactive part.
    // Actually, TApplication is needed if we want interactive access.
    
    MainWindow window;
    window.show();

    return app.exec();
}
