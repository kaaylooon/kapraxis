#include <QApplication>
#include <QCoreApplication>

#include "app/AppWindow.h"

int main(int argc, char *argv[]) {
    /*QCoreApplication::addLibraryPath("/usr/lib/x86_64-linux-gnu/qt6/plugins");*/
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Kapraxis");
    QCoreApplication::setApplicationName("Kapraxis");

    AppWindow window;
    window.showMaximized();
    window.show();

    return app.exec();
}
