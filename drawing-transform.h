#ifndef DRAWING_TRANSFORM_H
#define DRAWING_TRANSFORM_H

#include <QTransform>
#include <QPointF>
#include <QSizeF>
#include <QRectF>
#include <QtMath>
#include <QVariant>
#include <QVariantList>

/**
 * 改进的变换管理器 - 解决锚点和坐标系问题
 * 参考Inkscape的变换处理方式
 */
class DrawingTransform
{
public:
    enum AnchorPoint {
        TopLeft,
        TopCenter,
        TopRight,
        CenterLeft,
        Center,
        CenterRight,
        BottomLeft,
        BottomCenter,
        BottomRight,
        Custom
    };
    
    DrawingTransform();
    
    // 🌟 添加接受QTransform的构造函数
    DrawingTransform(const QTransform &transform) { m_transform = transform; }
    
    // 基础变换操作
    void translate(const QPointF &delta);
    void rotate(double angle, const QPointF &center = QPointF());
    void scale(double sx, double sy, const QPointF &center = QPointF());
    void shear(double sh, double sv, const QPointF &center = QPointF());
    
    // 锚点相关的变换
    void rotateAroundAnchor(double angle, AnchorPoint anchor, const QRectF &bounds);
    void scaleAroundAnchor(double sx, double sy, AnchorPoint anchor, const QRectF &bounds);
    void shearAroundAnchor(double sh, double sv, AnchorPoint anchor, const QRectF &bounds);
    
    // 获取和设置变换矩阵
    void setTransform(const QTransform &transform) { m_transform = transform; }
    QTransform transform() const { return m_transform; }
    
    // 获取边界框（考虑变换）
    QRectF transformedBounds(const QRectF &localBounds) const;
    
    // 坐标转换
    QPointF mapToScene(const QPointF &localPos, const QRectF &localBounds) const;
    QPointF mapToLocal(const QPointF &scenePos, const QRectF &localBounds) const;
    
    // 重置变换
    void reset() { m_transform = QTransform(); }
    
    // 获取变换信息
    double rotation() const;
    QPointF scale() const;
    QPointF shear() const;
    QPointF translation() const;
    
    // 公共方法：获取锚点位置
    QPointF getAnchorPoint(AnchorPoint anchor, const QRectF &bounds) const;
    
    // 变换状态管理
    void beginTransform();
    void applyRelativeTransform(const QTransform &relTransform, const QPointF &center);
    void endTransform();
    
    // 变换历史记录
    void saveState();
    void restoreState();
    
    QTransform m_transform;
    QTransform m_savedTransform; // 用于保存变换状态
};

/**
 * 变换操作记录 - 支持撤销/重做
 */
class TransformOperation
{
public:
    enum Type {
        Translate,
        Rotate,
        Scale,
        Shear
    };
    
    TransformOperation(Type type, const QVariantList &params);
    
    Type type() const { return m_type; }
    QVariantList parameters() const { return m_params; }
    
    // 应用到变换管理器
    void apply(DrawingTransform &transform) const;
    
    // 创建逆操作
    TransformOperation inverse() const;

private:
    Type m_type;
    QVariantList m_params;
};

#endif // DRAWING_TRANSFORM_H