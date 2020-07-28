#include "xcwdgt.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    xCWdgt w;
    w.show();
    return a.exec();
}
