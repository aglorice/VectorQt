#ifndef SELECTION_LAYER_H
#define SELECTION_LAYER_H

#include <QObject>
#include <QTimer>
#include <QRectF>
#include <QTransform>
#include <QList>
#include "drawing-shape.h"

// 前向声明
class EditHandleManager;
class QGraphicsScene;

/**
 * 选择管理器 - 管理选中的图形和变换操作
 * 不再是QGraphicsItem，而是独立的管理类
 */
class SelectionLayer : public QObject
{
    Q_OBJECT
public:
    explicit SelectionLayer(QObject *parent = nullptr);
    ~SelectionLayer();
    
    // 添加/移除选中的图形
    void addShape(DrawingShape *shape);
    void removeShape(DrawingShape *shape);
    void clearShapes();
    
    // 获取选中的图形列表
    QList<DrawingShape*> selectedShapes() const { return m_selectedShapes; }
    
    // 计算选择边界框（轴对齐）
    QRectF selectionBounds() const;
    
    // 变换状态枚举（参考Inkscape的SelTrans状态）
    enum TransformState {
        STATE_SCALE = 0,    // 缩放状态
        STATE_ROTATE = 1,   // 旋转状态
        STATE_SKEW = 2,     // 倾斜状态
        STATE_ALIGN = 3,    // 对齐状态
        STATE_NONE = 4      // 无变换状态
    };
    
    // 预览模式枚举（参考Inkscape的显示模式）
    enum PreviewMode {
        SHOW_CONTENT = 0,   // 显示内容预览
        SHOW_OUTLINE = 1    // 显示轮廓预览
    };
    
    // 手柄类型枚举（参考Inkscape的HANDLE_*）
    enum HandleType {
        HANDLE_CENTER = 0,      // 中心手柄
        HANDLE_SCALE = 1,       // 缩放手柄
        HANDLE_STRETCH = 2,     // 拉伸手柄
        HANDLE_SKEW = 3,        // 倾斜手柄
        HANDLE_ROTATE = 4,      // 旋转手柄
        HANDLE_ALIGN = 5        // 对齐手柄
    };
    
    // 手柄索引定义（与control-frame保持一致）
    enum HandleIndex {
        None = 0,
        TopLeft = 1,
        Top = 2,
        TopRight = 3,
        Left = 4,
        Right = 5,
        BottomLeft = 6,
        Bottom = 7,
        BottomRight = 8,
        Rotate = 9
    };
    
    // 变换操作（基于仿射变换矩阵）
    void translate(const QPointF &delta);
    void rotate(double angle, const QPointF &center);
    void scale(double sx, double sy, const QPointF &center);
    void scaleAroundAnchor(double sx, double sy, int handleIndex, const QPointF &anchorPoint = QPointF());
    void rotateAroundAnchor(double angle, int handleIndex);
    void skew(double skewX, double skewY, const QPointF &center);
    
    // 仿射变换管理（参考Inkscape的transform方法）
    void applyTransform(const QTransform &relAffine, const QPointF &norm);
    void grabTransform();  // 保存初始变换状态（参考Inkscape的grab）
    void ungrabTransform(); // 释放变换状态
    
    // 获取锚点位置（基于选择边界框）
    QPointF getAnchorPoint(int handleIndex) const;
    QPointF getTransformCenter() const;
    
    // 拖动状态
    void setMouseDown(bool down) { m_mouseDown = down; }
    void setStartScenePos(const QPointF &pos) { m_startScenePos = pos; }
    void setInitialBounds(const QRectF &bounds) { m_initialBounds = bounds; }
    void setDragHandle(int handle) { m_dragHandle = handle; }
    void setTransformState(TransformState state) { m_transformState = state; }
    TransformState getTransformState() const { return m_transformState; }
    
    QGraphicsItem* itemAt(const QPointF &pos) const;
    
    // 设置编辑把手管理器
    void setHandleManager(EditHandleManager *manager) { m_handleManager = manager; }
    EditHandleManager* handleManager() const { return m_handleManager; }
    
    // 预览模式控制
    void setPreviewMode(PreviewMode mode) { m_previewMode = mode; }
    PreviewMode getPreviewMode() const { return m_previewMode; }
    
    // 初始化变换和手柄位置（用于缩放/旋转操作）
    void updateInitialTransforms();
    void setAnchorAndHandlePositions(int handleIndex);
    
    // 更新选择层状态
    void updateSelectionBounds(); // 设为public，让场景可以调用
    void updateHandPositions(); // 更新手柄位置
    void updateHandles(); // 更新所有手柄（参考Inkscape的_updateHandles）
    
    // 获取场景坐标中的手柄位置
    QVector<QPointF> getSceneHandlePositions() const;
    
    // 手柄拖动处理
    void handleDrag(int handleIndex, const QPointF &scenePos);
    void handleScaleDrag(int handleIndex, const QPointF &scenePos);
    void handleStretchDrag(int handleIndex, const QPointF &scenePos);
    void handleRotateDrag(const QPointF &scenePos);
    
    // 不再是QGraphicsItem，移除相关接口
    
private:
    public:
    // 让外部可以访问这些成员变量
    QRectF m_initialBounds;
    QPointF m_anchorPoint;  // 当前操作使用的锚点
    QTransform m_currentRelativeAffine; // 当前相对仿射变换（参考Inkscape）

private:
    QList<DrawingShape*> m_selectedShapes;
    QRectF m_selectionBounds;      // 相对于中心的本地边界
    QRectF m_sceneSelectionBounds;  // 场景坐标系中的边界
    QHash<DrawingShape*, QTransform> m_initialTransforms;
    QPointF m_startScenePos;
    QVector<QPointF> m_initialHandles;  // 保存初始手柄位置
    int m_dragHandle;
    bool m_mouseDown;
    bool m_grabbed;  // 是否处于变换抓取状态（参考Inkscape）
    TransformState m_transformState;  // 当前变换状态
    EditHandleManager *m_handleManager;
    
    // 变换相关成员
    QTransform m_accumulatedTransform;  // 累积变换矩阵
    QPointF m_transformCenter;  // 变换中心点
    PreviewMode m_previewMode;  // 当前预览模式
    
    // 预览相关成员
    QVector<QRectF> m_previewOutlines;  // 预览轮廓
    bool m_showPreview;  // 是否显示预览
    
    // 定时器用于定期更新选择边界
    QTimer *m_updateTimer;
    
    // 内部方法
    void applyTransformToShapes();
    QVector<QPointF> m_handles; // 手柄位置数组
    
    // 变换计算辅助方法
    QTransform calculateScaleTransform(double sx, double sy, const QPointF &center);
    QTransform calculateRotateTransform(double angle, const QPointF &center);
    QTransform calculateSkewTransform(double skewX, double skewY, const QPointF &center);
    
    // 边界框计算（参考Inkscape的边界框处理）
    QRectF calculateVisualBounds() const;
    QRectF calculateGeometricBounds() const;
    
    // 手柄辅助方法
    int getOppositeHandle(int handleIndex) const;
    
    // 预览相关方法
    void updatePreview();
    void showPreview(bool show);
    void drawPreviewOutlines(QPainter *painter);
    void drawPreviewContent(QPainter *painter);
    void applyFinalTransform();
    
private slots:
    // void updateSelectionPeriodically(); // 已移除定时器
};

#endif // SELECTION_LAYER_H