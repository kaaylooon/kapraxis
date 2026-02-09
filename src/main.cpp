#include <QApplication>
#include "app/AppWindow.h"

int main(int argc, char *argv[]) {
    /*QCoreApplication::addLibraryPath("/usr/lib/x86_64-linux-gnu/qt6/plugins");*/
    QApplication app(argc, argv);

    AppWindow window;
    window.resize(1280, 720);
    window.show();

    return app.exec();
}
