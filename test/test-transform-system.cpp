#include "transform-system.h"
#include <QCoreApplication>
#include <QDebug>

void testBasicOperations()
{
    qDebug() << "=== 测试基本变换操作 ===";
    
    // 创建一个矩形对象
    QRectF rect(0, 0, 100, 50);
    TransformObject obj(rect);
    
    qDebug() << "原始边界:" << rect;
    qDebug() << "变换后边界:" << obj.transformedBounds();
    
    // 平移
    obj.addOperation(QSharedPointer<TransformOperation>::create(TranslateOperation(QPointF(50, 30))));
    qDebug() << "平移(50,30)后:" << obj.transformedBounds();
    qDebug() << "当前平移量:" << obj.translation();
    
    // 旋转
    obj.addOperation(QSharedPointer<TransformOperation>::create(RotateOperation(45, QPointF(50, 25))));
    qDebug() << "旋转45度后:" << obj.transformedBounds();
    qDebug() << "当前旋转角度:" << obj.rotation();
    
    // 缩放
    obj.addOperation(QSharedPointer<TransformOperation>::create(ScaleOperation(1.5, 2.0, QPointF(50, 25))));
    qDebug() << "缩放(1.5,2.0)后:" << obj.transformedBounds();
    qDebug() << "当前缩放:" << obj.scale();
    
    // 斜切
    obj.addOperation(QSharedPointer<TransformOperation>::create(ShearOperation(0.2, 0.1, QPointF(50, 25))));
    qDebug() << "斜切(0.2,0.1)后:" << obj.transformedBounds();
    qDebug() << "当前斜切:" << obj.shear();
}

void testMultiSelection()
{
    qDebug() << "\n=== 测试多选操作 ===";
    
    // 创建多个对象
    TransformObject obj1(QRectF(0, 0, 50, 50));
    TransformObject obj2(QRectF(100, 0, 50, 50));
    TransformObject obj3(QRectF(50, 50, 50, 50));
    
    TransformManager manager;
    manager.addObject(&obj1);
    manager.addObject(&obj2);
    manager.addObject(&obj3);
    
    qDebug() << "选择边界:" << manager.selectionBounds();
    
    // 批量平移
    manager.translateSelection(QPointF(20, 20));
    qDebug() << "平移后选择边界:" << manager.selectionBounds();
    
    // 批量旋转
    QRectF bounds = manager.selectionBounds();
    QPointF center = bounds.center();
    manager.rotateSelection(30, center);
    qDebug() << "旋转后选择边界:" << manager.selectionBounds();
    
    // 撤销操作
    manager.saveSelectionState();
    manager.scaleSelection(1.2, 1.2, center);
    qDebug() << "缩放后选择边界:" << manager.selectionBounds();
    
    manager.restoreSelectionState();
    qDebug() << "撤销后选择边界:" << manager.selectionBounds();
}

void testUndoRedo()
{
    qDebug() << "\n=== 测试撤销/重做 ===";
    
    QRectF rect(0, 0, 100, 100);
    TransformObject obj(rect);
    
    // 保存初始状态
    obj.saveState();
    qDebug() << "初始状态:" << obj.transformedBounds();
    
    // 应用一系列变换
    obj.addOperation(QSharedPointer<TransformOperation>::create(TranslateOperation(QPointF(50, 50))));
    obj.addOperation(QSharedPointer<TransformOperation>::create(RotateOperation(45, QPointF(100, 100))));
    obj.addOperation(QSharedPointer<TransformOperation>::create(ScaleOperation(2, 2, QPointF(100, 100))));
    
    qDebug() << "变换后:" << obj.transformedBounds();
    
    // 撤销
    obj.restoreState();
    qDebug() << "撤销后:" << obj.transformedBounds();
    
    // 检查变换参数
    qDebug() << "平移:" << obj.translation();
    qDebug() << "旋转:" << obj.rotation();
    qDebug() << "缩放:" << obj.scale();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    testBasicOperations();
    testMultiSelection();
    testUndoRedo();
    
    qDebug() << "\n=== 新变换系统测试完成 ===";
    
    return 0;
}