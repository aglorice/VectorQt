#include <QApplication>
#include <QMainWindow>
#include <QSvgWidget>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("Qt 6.9 SVG特性测试");
    mainWindow.resize(450, 350);

    // 创建SVG显示控件
    QSvgWidget *svgWidget = new QSvgWidget;
    svgWidget->load(QString("strz.min.svg")); // 加载测试SVG

    mainWindow.setCentralWidget(svgWidget);
    mainWindow.show();

    return app.exec();
}
