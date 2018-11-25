/*
#include "qt/utopiacoingui.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UtopiacoinGUI w;
    w.show();

    return a.exec();
}
*/
//#include "utopiacoin.h"
#include <QApplication>

#include "intro.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Intro w;
    w.show();

    return a.exec();
}
