#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "drawing-group.h"
#include "drawing-shape.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QGraphicsScene scene;
    QGraphicsView view(&scene);
    view.resize(800, 600);
    view.show();
    
    // 创建一个组
    DrawingGroup *group = new DrawingGroup();
    scene.addItem(group);
    
    // 创建一个矩形并添加到组
    DrawingRectangle *rect = new DrawingRectangle(QRectF(0, 0, 100, 100));
    group->addItem(rect);
    
    // 选择组
    group->setSelected(true);
    
    return app.exec();
}