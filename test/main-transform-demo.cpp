#include "transform-demo.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    TransformDemoWindow window;
    window.show();
    
    return app.exec();
}