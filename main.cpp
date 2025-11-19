#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 设置应用程序信息
    a.setApplicationName("VectorQt");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("VectorQt Team");
    
    MainWindow window;
    window.show();
    
    return a.exec();
}