#include <iostream>
#include <vector>
#include <cmath>
#include <memory>

// 简单的2D点类
struct Point {
    double x, y;
    Point(double x = 0, double y = 0) : x(x), y(y) {}
    Point operator+(const Point& other) const { return Point(x + other.x, y + other.y); }
    Point operator-(const Point& other) const { return Point(x - other.x, y - other.y); }
    Point operator*(double s) const { return Point(x * s, y * s); }
};

// 简单的矩形类
struct Rect {
    Point topLeft;
    Point bottomRight;
    Rect(Point tl, Point br) : topLeft(tl), bottomRight(br) {}
    Point center() const { return Point((topLeft.x + bottomRight.x) / 2, (topLeft.y + bottomRight.y) / 2); }
    double width() const { return bottomRight.x - topLeft.x; }
    double height() const { return bottomRight.y - topLeft.y; }
};

// 变换操作基类
class TransformOperation {
public:
    enum Type { Translate, Rotate, Scale, Shear };
    
    TransformOperation(Type type) : m_type(type) {}
    virtual ~TransformOperation() {}
    
    Type type() const { return m_type; }
    virtual Point apply(const Point& p) const = 0;
    virtual void print() const = 0;
    
protected:
    Type m_type;
};

// 平移操作
class TranslateOperation : public TransformOperation {
public:
    TranslateOperation(const Point& delta) 
        : TransformOperation(Translate), m_delta(delta) {}
    
    Point apply(const Point& p) const override {
        return p + m_delta;
    }
    
    void print() const override {
        std::cout << "  平移(" << m_delta.x << ", " << m_delta.y << ")" << std::endl;
    }
    
private:
    Point m_delta;
};

// 旋转操作
class RotateOperation : public TransformOperation {
public:
    RotateOperation(double angle, const Point& center = Point())
        : TransformOperation(Rotate), m_angle(angle), m_center(center) {}
    
    Point apply(const Point& p) const override {
        // 转换到弧度
        double rad = m_angle * M_PI / 180.0;
        double cos_a = cos(rad);
        double sin_a = sin(rad);
        
        // 平移到原点
        Point pt = p - m_center;
        // 旋转
        Point rotated(
            pt.x * cos_a - pt.y * sin_a,
            pt.x * sin_a + pt.y * cos_a
        );
        // 平移回去
        return rotated + m_center;
    }
    
    void print() const override {
        std::cout << "  旋转(" << m_angle << "°, 中心(" 
                  << m_center.x << ", " << m_center.y << "))" << std::endl;
    }
    
private:
    double m_angle;
    Point m_center;
};

// 缩放操作
class ScaleOperation : public TransformOperation {
public:
    ScaleOperation(double sx, double sy, const Point& center = Point())
        : TransformOperation(Scale), m_sx(sx), m_sy(sy), m_center(center) {}
    
    Point apply(const Point& p) const override {
        Point pt = p - m_center;
        pt.x *= m_sx;
        pt.y *= m_sy;
        return pt + m_center;
    }
    
    void print() const override {
        std::cout << "  缩放(" << m_sx << ", " << m_sy << ", 中心(" 
                  << m_center.x << ", " << m_center.y << "))" << std::endl;
    }
    
private:
    double m_sx, m_sy;
    Point m_center;
};

// 变换对象
class TransformObject {
public:
    TransformObject(const Rect& bounds) : m_localBounds(bounds) {}
    
    void addOperation(std::shared_ptr<TransformOperation> op) {
        m_operations.push_back(op);
    }
    
    Point mapToScene(const Point& localPos) const {
        Point p = localPos;
        for (const auto& op : m_operations) {
            p = op->apply(p);
        }
        return p;
    }
    
    Rect transformedBounds() const {
        Point tl = mapToScene(m_localBounds.topLeft);
        Point br = mapToScene(m_localBounds.bottomRight);
        Point tr = mapToScene(Point(m_localBounds.bottomRight.x, m_localBounds.topLeft.y));
        Point bl = mapToScene(Point(m_localBounds.topLeft.x, m_localBounds.bottomRight.y));
        
        // 找到最小和最大的x,y
        double minX = std::min({tl.x, tr.x, bl.x, br.x});
        double minY = std::min({tl.y, tr.y, bl.y, br.y});
        double maxX = std::max({tl.x, tr.x, bl.x, br.x});
        double maxY = std::max({tl.y, tr.y, bl.y, br.y});
        
        return Rect(Point(minX, minY), Point(maxX, maxY));
    }
    
    void printOperations() const {
        std::cout << "变换操作序列:" << std::endl;
        for (const auto& op : m_operations) {
            op->print();
        }
    }
    
private:
    Rect m_localBounds;
    std::vector<std::shared_ptr<TransformOperation>> m_operations;
};

// 演示函数
void demonstrateTransformSystem() {
    std::cout << "=== 新变换系统演示 ===" << std::endl;
    
    // 创建一个矩形对象
    Rect originalRect(Point(0, 0), Point(100, 50));
    TransformObject obj(originalRect);
    
    std::cout << "\n原始矩形:" << std::endl;
    std::cout << "  位置: (" << originalRect.topLeft.x << ", " << originalRect.topLeft.y << ")" << std::endl;
    std::cout << "  大小: " << originalRect.width() << " x " << originalRect.height() << std::endl;
    
    // 应用平移
    obj.addOperation(std::make_shared<TranslateOperation>(Point(50, 30)));
    
    std::cout << "\n平移后的矩形:" << std::endl;
    Rect translated = obj.transformedBounds();
    std::cout << "  位置: (" << translated.topLeft.x << ", " << translated.topLeft.y << ")" << std::endl;
    std::cout << "  大小: " << translated.width() << " x " << translated.height() << std::endl;
    
    // 应用旋转
    Point center = translated.center();
    obj.addOperation(std::make_shared<RotateOperation>(45, center));
    
    std::cout << "\n旋转后的矩形:" << std::endl;
    Rect rotated = obj.transformedBounds();
    std::cout << "  位置: (" << rotated.topLeft.x << ", " << rotated.topLeft.y << ")" << std::endl;
    std::cout << "  大小: " << rotated.width() << " x " << rotated.height() << std::endl;
    
    // 应用缩放
    obj.addOperation(std::make_shared<ScaleOperation>(1.5, 2.0, center));
    
    std::cout << "\n缩放后的矩形:" << std::endl;
    Rect scaled = obj.transformedBounds();
    std::cout << "  位置: (" << scaled.topLeft.x << ", " << scaled.topLeft.y << ")" << std::endl;
    std::cout << "  大小: " << scaled.width() << " x " << scaled.height() << std::endl;
    
    // 显示所有操作
    std::cout << "\n应用的变换操作:" << std::endl;
    obj.printOperations();
    
    // 演示点变换
    std::cout << "\n点变换演示:" << std::endl;
    Point originalPoint(25, 25);  // 矩形中心
    Point transformedPoint = obj.mapToScene(originalPoint);
    std::cout << "  原始点: (" << originalPoint.x << ", " << originalPoint.y << ")" << std::endl;
    std::cout << "  变换后: (" << transformedPoint.x << ", " << transformedPoint.y << ")" << std::endl;
}

int main() {
    demonstrateTransformSystem();
    
    std::cout << "\n=== 演示完成 ===" << std::endl;
    std::cout << "\n新变换系统的优势:" << std::endl;
    std::cout << "1. 每个变换操作都是独立的，可以单独撤销" << std::endl;
    std::cout << "2. 对象始终保持原始本地坐标" << std::endl;
    std::cout << "3. 避免了矩阵分解的精度损失" << std::endl;
    std::cout << "4. 多选时每个对象的本地坐标保持独立" << std::endl;
    
    return 0;
}