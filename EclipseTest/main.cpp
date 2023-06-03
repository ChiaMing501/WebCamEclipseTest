#include "EclipseTest.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EclipseTest w;
    w.show();
    return a.exec();
}
