#ifndef DRAWINGLAYER_H
#define DRAWINGLAYER_H

#include <QGraphicsItem>
#include <QList>
#include <QString>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDomElement>

class DrawingShape;

/**
 * 绘图层类 - 管理图层和图层中的图形
 */
class DrawingLayer : public QGraphicsItem
{
public:
    explicit DrawingLayer(const QString &name = "Layer", QGraphicsItem *parent = nullptr);
    ~DrawingLayer();
    
    // 图层属性
    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }
    
    bool isVisible() const { return m_visible; }
    void setVisible(bool visible);
    
    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal opacity);
    
    bool isLocked() const { return m_locked; }
    void setLocked(bool locked) { m_locked = locked; }
    
    // 图层内容管理
    void addShape(DrawingShape *shape);
    void removeShape(DrawingShape *shape);
    QList<DrawingShape*> shapes() const { return m_shapes; }
    
    // QGraphicsItem接口
    enum { Type = UserType + 100 };  // 唯一的类型值
    int type() const override { return Type; }
    
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    
    // 图层变换
    void setLayerTransform(const QTransform &transform);
    QTransform layerTransform() const { return m_layerTransform; }
    
    // SVG相关
    void parseFromSvg(const QDomElement &element);
    QDomElement exportToSvg(QDomDocument &doc) const;

private:
    QString m_name;
    bool m_visible;
    qreal m_opacity;
    bool m_locked;
    QList<DrawingShape*> m_shapes;
    QTransform m_layerTransform;
    mutable QRectF m_boundingRect;
    mutable bool m_boundingRectDirty;
    
    void updateBoundingRect() const;
};

#endif // DRAWINGLAYER_H