#include "transform-demo.h"
#include <QApplication>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QInputDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QSplitter>
#include <QDialog>

// DemoGraphicsItem 实现
DemoGraphicsItem::DemoGraphicsItem(const QRectF &rect, TransformObject *transformObj, QGraphicsItem *parent)
    : QGraphicsRectItem(rect, parent)
    , m_transformObj(transformObj)
    , m_isDragging(false)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, false); // 禁用Qt的移动，使用我们的变换系统
    setBrush(QBrush(QColor(100, 150, 200, 150)));
    setPen(QPen(Qt::darkBlue, 2));
    
    updateDisplay();
}

void DemoGraphicsItem::updateDisplay()
{
    if (!m_transformObj) return;
    
    // 如果正在拖动，不更新位置，避免跳跃
    if (m_isDragging) {
        update();
        return;
    }
    
    // 获取变换后的边界
    QRectF transformedBounds = m_transformObj->transformedBounds();
    
    // 更新Qt图形项的位置和大小
    setPos(transformedBounds.topLeft());
    setRect(0, 0, transformedBounds.width(), transformedBounds.height());
    
    // 应用变换矩阵（用于绘制）
    QTransform qtTransform = m_transformObj->combinedTransform();
    // 将变换中心移动到本地坐标系原点
    QTransform adjust;
    adjust.translate(-m_transformObj->localBounds().x(), -m_transformObj->localBounds().y());
    setTransform(qtTransform * adjust);
    
    update();
}

void DemoGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStart = event->scenePos();
        m_isDragging = true;
        // 保存拖动开始时的位置
        m_dragStartPos = pos();
        event->accept();
    } else {
        QGraphicsRectItem::mousePressEvent(event);
    }
}

void DemoGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isDragging && m_transformObj) {
        QPointF delta = event->scenePos() - m_dragStart;
        
        // 临时更新位置（不添加到变换操作）
        setPos(m_dragStartPos + delta);
        
        event->accept();
    } else {
        QGraphicsRectItem::mouseMoveEvent(event);
    }
}

void DemoGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isDragging) {
        // 计算总的平移量
        QPointF totalDelta = pos() - m_dragStartPos;
        
        // 只添加一个最终的平移操作
        if (!totalDelta.isNull()) {
            auto translateOp = std::make_shared<TranslateOperation>(totalDelta);
            m_transformObj->addOperation(translateOp);
        }
        
        m_isDragging = false;
        // 拖动结束后，更新显示以同步变换系统
        updateDisplay();
        event->accept();
    } else {
        QGraphicsRectItem::mouseReleaseEvent(event);
    }
}

void DemoGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsRectItem::paint(painter, option, widget);
    
    // 绘制本地坐标系原点
    painter->setPen(QPen(Qt::red, 3));
    QPointF localOrigin = mapFromScene(m_transformObj->mapToScene(m_transformObj->localBounds().topLeft()));
    painter->drawEllipse(localOrigin, 3, 3);
}

// TransformScene 实现
TransformScene::TransformScene(QObject *parent)
    : QGraphicsScene(parent)
    , m_isPanning(false)
{
    setSceneRect(-500, -500, 1000, 1000);
    
    // 添加网格
    QPen gridPen(Qt::lightGray, 0.5);
    for (int i = -500; i <= 500; i += 50) {
        addLine(i, -500, i, 500, gridPen);
        addLine(-500, i, 500, i, gridPen);
    }
    
    // 添加坐标轴
    QPen axisPen(Qt::gray, 2);
    addLine(0, -500, 0, 500, axisPen);
    addLine(-500, 0, 500, 0, axisPen);
}

void TransformScene::addTransformItem(const QRectF &rect)
{
    // 创建变换对象
    TransformObject *transformObj = new TransformObject(rect);
    m_transformObjects.append(transformObj);
    
    // 创建Qt图形项
    DemoGraphicsItem *item = new DemoGraphicsItem(rect, transformObj);
    addItem(item);
    
    // 添加到变换管理器
    m_transformManager.addObject(transformObj);
}

QList<TransformObject*> TransformScene::selectedTransformObjects()
{
    QList<TransformObject*> result;
    for (auto *item : selectedItems()) {
        if (auto *demoItem = qgraphicsitem_cast<DemoGraphicsItem*>(item)) {
            result.append(demoItem->transformObject());
        }
    }
    return result;
}

void TransformScene::applyToSelection(std::shared_ptr<TransformOperation> op)
{
    auto selected = selectedTransformObjects();
    for (auto *obj : selected) {
        obj->addOperation(op);
    }
    
    // 更新所有选中项的显示
    for (auto *item : selectedItems()) {
        if (auto *demoItem = qgraphicsitem_cast<DemoGraphicsItem*>(item)) {
            demoItem->updateDisplay();
        }
    }
}

void TransformScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        showContextMenu(event->scenePos());
        event->accept();
    } else if (event->button() == Qt::MiddleButton) {
        m_isPanning = true;
        m_lastScenePos = event->scenePos();
        event->accept();
    } else {
        QGraphicsScene::mousePressEvent(event);
    }
}

void TransformScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isPanning) {
        QPointF delta = event->scenePos() - m_lastScenePos;
        for (auto *view : views()) {
            view->translate(delta.x(), delta.y());
        }
        m_lastScenePos = event->scenePos();
        event->accept();
    } else {
        QGraphicsScene::mouseMoveEvent(event);
    }
}

void TransformScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = false;
        event->accept();
    } else {
        QGraphicsScene::mouseReleaseEvent(event);
    }
}

void TransformScene::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        // 删除选中项
        auto selected = selectedItems();
        for (auto *item : selected) {
            if (auto *demoItem = qgraphicsitem_cast<DemoGraphicsItem*>(item)) {
                m_transformObjects.removeAll(demoItem->transformObject());
                m_transformManager.removeObject(demoItem->transformObject());
                delete demoItem->transformObject();
                removeItem(item);
                delete item;
            }
        }
    } else if (event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_A) {
        // 全选
        for (auto *item : items()) {
            if (auto *demoItem = qgraphicsitem_cast<DemoGraphicsItem*>(item)) {
                demoItem->setSelected(true);
            }
        }
    } else {
        QGraphicsScene::keyPressEvent(event);
    }
}

void TransformScene::showContextMenu(QPointF pos)
{
    QMenu contextMenu;
    
    QAction *translateAction = contextMenu.addAction("平移(T)");
    QAction *rotateAction = contextMenu.addAction("旋转(R)");
    QAction *scaleAction = contextMenu.addAction("缩放(S)");
    QAction *shearAction = contextMenu.addAction("斜切(H)");
    contextMenu.addSeparator();
    QAction *resetAction = contextMenu.addAction("重置变换");
    QAction *infoAction = contextMenu.addAction("显示信息");
    
    QAction *selectedAction = contextMenu.exec(QCursor::pos());
    
    if (selectedAction == translateAction) {
        bool ok;
        double dx = QInputDialog::getDouble(nullptr, "平移", "X方向:", 0, -1000, 1000, 1, &ok);
        if (ok) {
            double dy = QInputDialog::getDouble(nullptr, "平移", "Y方向:", 0, -1000, 1000, 1, &ok);
            if (ok) {
                applyToSelection(std::make_shared<TranslateOperation>(QPointF(dx, dy)));
            }
        }
    } else if (selectedAction == rotateAction) {
        bool ok;
        double angle = QInputDialog::getDouble(nullptr, "旋转", "角度:", 45, -360, 360, 1, &ok);
        if (ok) {
            QRectF bounds = m_transformManager.selectionBounds();
            QPointF center = bounds.center();
            applyToSelection(std::make_shared<RotateOperation>(angle, center));
        }
    } else if (selectedAction == scaleAction) {
        bool ok;
        double sx = QInputDialog::getDouble(nullptr, "缩放", "X方向:", 1.2, 0.1, 5, 2, &ok);
        if (ok) {
            double sy = QInputDialog::getDouble(nullptr, "缩放", "Y方向:", 1.2, 0.1, 5, 2, &ok);
            if (ok) {
                QRectF bounds = m_transformManager.selectionBounds();
                QPointF center = bounds.center();
                applyToSelection(std::make_shared<ScaleOperation>(sx, sy, center));
            }
        }
    } else if (selectedAction == shearAction) {
        bool ok;
        double sh = QInputDialog::getDouble(nullptr, "斜切", "X方向:", 0.2, -2, 2, 2, &ok);
        if (ok) {
            double sv = QInputDialog::getDouble(nullptr, "斜切", "Y方向:", 0.1, -2, 2, 2, &ok);
            if (ok) {
                QRectF bounds = m_transformManager.selectionBounds();
                QPointF center = bounds.center();
                applyToSelection(std::make_shared<ShearOperation>(sh, sv, center));
            }
        }
    } else if (selectedAction == resetAction) {
        auto selected = selectedTransformObjects();
        for (auto *obj : selected) {
            obj->clearOperations();
        }
        for (auto *item : selectedItems()) {
            if (auto *demoItem = qgraphicsitem_cast<DemoGraphicsItem*>(item)) {
                demoItem->updateDisplay();
            }
        }
    } else if (selectedAction == infoAction) {
        QString info;
        auto selected = selectedTransformObjects();
        for (int i = 0; i < selected.size(); ++i) {
            auto *obj = selected[i];
            info += QString("对象 %1:\n").arg(i + 1);
            info += QString("  本地边界: (%2, %3) %4x%5\n")
                    .arg(obj->localBounds().topLeft().x())
                    .arg(obj->localBounds().topLeft().y())
                    .arg(obj->localBounds().width())
                    .arg(obj->localBounds().height());
            info += QString("  变换后边界: (%2, %3) %4x%5\n")
                    .arg(obj->transformedBounds().topLeft().x())
                    .arg(obj->transformedBounds().topLeft().y())
                    .arg(obj->transformedBounds().width())
                    .arg(obj->transformedBounds().height());
            info += QString("  平移: (%1, %2)\n")
                    .arg(obj->translation().x())
                    .arg(obj->translation().y());
            info += QString("  旋转: %1°\n").arg(obj->rotation());
            info += QString("  缩放: (%1, %2)\n")
                    .arg(obj->scale().x())
                    .arg(obj->scale().y());
            info += QString("  斜切: (%1, %2)\n")
                    .arg(obj->shear().x())
                    .arg(obj->shear().y());
            info += "\n";
        }
        
        QDialog dialog;
        QVBoxLayout layout(&dialog);
        QTextEdit textEdit;
        textEdit.setPlainText(info);
        textEdit.setReadOnly(true);
        layout.addWidget(&textEdit);
        dialog.setWindowTitle("变换信息");
        dialog.exec();
    }
}

// TransformView 实现
TransformView::TransformView(TransformScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
    , m_isPanning(false)
{
    setDragMode(QGraphicsView::RubberBandDrag);
    setRenderHint(QPainter::Antialiasing);
}

void TransformView::wheelEvent(QWheelEvent *event)
{
    const double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        scale(scaleFactor, scaleFactor);
    } else {
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
}

void TransformView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = true;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void TransformView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPanning) {
        QPoint delta = event->pos() - m_lastPanPoint;
        m_lastPanPoint = event->pos();
        
        translate(delta.x(), delta.y());
        event->accept();
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void TransformView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

// TransformDemoWindow 实现
TransformDemoWindow::TransformDemoWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_showOriginal(false)
    , m_showTransformChain(false)
{
    setupUI();
    setupActions();
    setupToolbar();
    
    // 添加一些初始对象
    m_scene->addTransformItem(QRectF(-100, -50, 100, 50));
    m_scene->addTransformItem(QRectF(50, -50, 80, 80));
    m_scene->addTransformItem(QRectF(-50, 50, 120, 60));
}

void TransformDemoWindow::setupUI()
{
    m_scene = new TransformScene(this);
    m_view = new TransformView(m_scene, this);
    
    setCentralWidget(m_view);
    setWindowTitle("变换系统演示 - 新系统与Qt的协作");
    resize(1000, 700);
}

void TransformDemoWindow::setupActions()
{
    m_addRectAction = new QAction("添加矩形", this);
    m_addRectAction->setShortcut(QKeySequence("Ctrl+R"));
    connect(m_addRectAction, &QAction::triggered, this, &TransformDemoWindow::addRectangle);
    
    m_addCircleAction = new QAction("添加圆形", this);
    m_addCircleAction->setShortcut(QKeySequence("Ctrl+C"));
    connect(m_addCircleAction, &QAction::triggered, this, &TransformDemoWindow::addCircle);
    
    m_translateAction = new QAction("平移", this);
    m_translateAction->setShortcut(QKeySequence("T"));
    connect(m_translateAction, &QAction::triggered, this, &TransformDemoWindow::translateSelection);
    
    m_rotateAction = new QAction("旋转", this);
    m_rotateAction->setShortcut(QKeySequence("R"));
    connect(m_rotateAction, &QAction::triggered, this, &TransformDemoWindow::rotateSelection);
    
    m_scaleAction = new QAction("缩放", this);
    m_scaleAction->setShortcut(QKeySequence("S"));
    connect(m_scaleAction, &QAction::triggered, this, &TransformDemoWindow::scaleSelection);
    
    m_shearAction = new QAction("斜切", this);
    m_shearAction->setShortcut(QKeySequence("H"));
    connect(m_shearAction, &QAction::triggered, this, &TransformDemoWindow::shearSelection);
    
    m_resetAction = new QAction("重置", this);
    m_resetAction->setShortcut(QKeySequence("Ctrl+Z"));
    connect(m_resetAction, &QAction::triggered, this, &TransformDemoWindow::resetTransform);
    
    m_showInfoAction = new QAction("显示信息", this);
    m_showInfoAction->setShortcut(QKeySequence("I"));
    connect(m_showInfoAction, &QAction::triggered, this, &TransformDemoWindow::showTransformInfo);
    
    m_showOriginalAction = new QAction("显示原始", this);
    m_showOriginalAction->setCheckable(true);
    connect(m_showOriginalAction, &QAction::toggled, this, &TransformDemoWindow::toggleShowOriginal);
    
    m_showChainAction = new QAction("显示变换链", this);
    m_showChainAction->setCheckable(true);
    connect(m_showChainAction, &QAction::toggled, this, &TransformDemoWindow::toggleShowTransformChain);
    
    addAction(m_addRectAction);
    addAction(m_addCircleAction);
    addAction(m_translateAction);
    addAction(m_rotateAction);
    addAction(m_scaleAction);
    addAction(m_shearAction);
    addAction(m_resetAction);
    addAction(m_showInfoAction);
}

void TransformDemoWindow::setupToolbar()
{
    QToolBar *toolbar = addToolBar("操作");
    toolbar->addAction(m_addRectAction);
    toolbar->addAction(m_addCircleAction);
    toolbar->addSeparator();
    toolbar->addAction(m_translateAction);
    toolbar->addAction(m_rotateAction);
    toolbar->addAction(m_scaleAction);
    toolbar->addAction(m_shearAction);
    toolbar->addSeparator();
    toolbar->addAction(m_resetAction);
    toolbar->addAction(m_showInfoAction);
    toolbar->addAction(m_showOriginalAction);
    toolbar->addAction(m_showChainAction);
}

void TransformDemoWindow::addRectangle()
{
    bool ok;
    double width = QInputDialog::getDouble(this, "添加矩形", "宽度:", 100, 10, 500, 1, &ok);
    if (!ok) return;
    double height = QInputDialog::getDouble(this, "添加矩形", "高度:", 60, 10, 500, 1, &ok);
    if (!ok) return;
    
    QRectF rect(-width/2, -height/2, width, height);
    m_scene->addTransformItem(rect);
}

void TransformDemoWindow::addCircle()
{
    bool ok;
    double radius = QInputDialog::getDouble(this, "添加圆形", "半径:", 50, 10, 250, 1, &ok);
    if (!ok) return;
    
    QRectF rect(-radius, -radius, radius*2, radius*2);
    m_scene->addTransformItem(rect);
}

void TransformDemoWindow::translateSelection()
{
    bool ok;
    double dx = QInputDialog::getDouble(this, "平移", "X方向:", 50, -500, 500, 1, &ok);
    if (!ok) return;
    double dy = QInputDialog::getDouble(this, "平移", "Y方向:", 30, -500, 500, 1, &ok);
    if (!ok) return;
    
    m_scene->applyToSelection(std::make_shared<TranslateOperation>(QPointF(dx, dy)));
}

void TransformDemoWindow::rotateSelection()
{
    auto selected = m_scene->selectedTransformObjects();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择对象");
        return;
    }
    
    bool ok;
    double angle = QInputDialog::getDouble(this, "旋转", "角度:", 45, -360, 360, 1, &ok);
    if (!ok) return;
    
    // 计算选择中心
    QRectF bounds;
    for (auto *obj : selected) {
        bounds = bounds.united(obj->transformedBounds());
    }
    QPointF center = bounds.center();
    
    m_scene->applyToSelection(std::make_shared<RotateOperation>(angle, center));
}

void TransformDemoWindow::scaleSelection()
{
    auto selected = m_scene->selectedTransformObjects();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择对象");
        return;
    }
    
    bool ok;
    double sx = QInputDialog::getDouble(this, "缩放", "X方向:", 1.2, 0.1, 5, 2, &ok);
    if (!ok) return;
    double sy = QInputDialog::getDouble(this, "缩放", "Y方向:", 1.2, 0.1, 5, 2, &ok);
    if (!ok) return;
    
    // 计算选择中心
    QRectF bounds;
    for (auto *obj : selected) {
        bounds = bounds.united(obj->transformedBounds());
    }
    QPointF center = bounds.center();
    
    m_scene->applyToSelection(std::make_shared<ScaleOperation>(sx, sy, center));
}

void TransformDemoWindow::shearSelection()
{
    auto selected = m_scene->selectedTransformObjects();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择对象");
        return;
    }
    
    bool ok;
    double sh = QInputDialog::getDouble(this, "斜切", "X方向:", 0.2, -2, 2, 2, &ok);
    if (!ok) return;
    double sv = QInputDialog::getDouble(this, "斜切", "Y方向:", 0.1, -2, 2, 2, &ok);
    if (!ok) return;
    
    // 计算选择中心
    QRectF bounds;
    for (auto *obj : selected) {
        bounds = bounds.united(obj->transformedBounds());
    }
    QPointF center = bounds.center();
    
    m_scene->applyToSelection(std::make_shared<ShearOperation>(sh, sv, center));
}

void TransformDemoWindow::resetTransform()
{
    auto selected = m_scene->selectedTransformObjects();
    for (auto *obj : selected) {
        obj->clearOperations();
    }
    for (auto *item : m_scene->selectedItems()) {
        if (auto *demoItem = qgraphicsitem_cast<DemoGraphicsItem*>(item)) {
            demoItem->updateDisplay();
        }
    }
}

void TransformDemoWindow::showTransformInfo()
{
    QString info;
    auto selected = m_scene->selectedTransformObjects();
    for (int i = 0; i < selected.size(); ++i) {
        auto *obj = selected[i];
        info += QString("对象 %1:\n").arg(i + 1);
        info += QString("  本地边界: (%2, %3) %4x%5\n")
                .arg(obj->localBounds().topLeft().x())
                .arg(obj->localBounds().topLeft().y())
                .arg(obj->localBounds().width())
                .arg(obj->localBounds().height());
        info += QString("  变换后边界: (%2, %3) %4x%5\n")
                .arg(obj->transformedBounds().topLeft().x())
                .arg(obj->transformedBounds().topLeft().y())
                .arg(obj->transformedBounds().width())
                .arg(obj->transformedBounds().height());
        info += QString("  平移: (%1, %2)\n")
                .arg(obj->translation().x())
                .arg(obj->translation().y());
        info += QString("  旋转: %1°\n").arg(obj->rotation());
        info += QString("  缩放: (%1, %2)\n")
                .arg(obj->scale().x())
                .arg(obj->scale().y());
        info += QString("  斜切: (%1, %2)\n")
                .arg(obj->shear().x())
                .arg(obj->shear().y());
        
        // 显示变换链
        info += "  变换操作链:\n";
        for (const auto &op : obj->operations()) {
            switch (op->type()) {
                case TransformOperation::Translate:
                    info += QString("    - 平移(%1, %2)\n")
                            .arg(std::dynamic_pointer_cast<TranslateOperation>(op)->delta().x())
                            .arg(std::dynamic_pointer_cast<TranslateOperation>(op)->delta().y());
                    break;
                case TransformOperation::Rotate:
                    info += QString("    - 旋转(%1°)\n")
                            .arg(std::dynamic_pointer_cast<RotateOperation>(op)->angle());
                    break;
                case TransformOperation::Scale:
                    info += QString("    - 缩放(%1, %2)\n")
                            .arg(std::dynamic_pointer_cast<ScaleOperation>(op)->scaleX())
                            .arg(std::dynamic_pointer_cast<ScaleOperation>(op)->scaleY());
                    break;
                case TransformOperation::Shear:
                    info += QString("    - 斜切(%1, %2)\n")
                            .arg(std::dynamic_pointer_cast<ShearOperation>(op)->shearX())
                            .arg(std::dynamic_pointer_cast<ShearOperation>(op)->shearY());
                    break;
                case TransformOperation::Matrix:
                    info += "    - 矩阵变换\n";
                    break;
            }
        }
        info += "\n";
    }
    
    QDialog dialog;
    QVBoxLayout layout(&dialog);
    QTextEdit textEdit;
    textEdit.setPlainText(info);
    textEdit.setReadOnly(true);
    layout.addWidget(&textEdit);
    dialog.setWindowTitle("变换信息");
    dialog.resize(400, 500);
    dialog.exec();
}

void TransformDemoWindow::toggleShowOriginal(bool show)
{
    m_showOriginal = show;
    update();
}

void TransformDemoWindow::toggleShowTransformChain(bool show)
{
    m_showTransformChain = show;
    update();
}

void TransformDemoWindow::updateInfo()
{
    // 更新状态栏信息
    QString info = QString("选中对象: %1").arg(m_scene->selectedItems().size());
    if (!m_scene->selectedItems().isEmpty()) {
        QRectF bounds;
        auto selected = m_scene->selectedTransformObjects();
        for (auto *obj : selected) {
            bounds = bounds.united(obj->transformedBounds());
        }
        info += QString(" | 选择边界: (%1, %2) %3x%4")
                .arg(bounds.left(), 0, 'f', 1)
                .arg(bounds.top(), 0, 'f', 1)
                .arg(bounds.width(), 0, 'f', 1)
                .arg(bounds.height(), 0, 'f', 1);
    }
    statusBar()->showMessage(info);
}

