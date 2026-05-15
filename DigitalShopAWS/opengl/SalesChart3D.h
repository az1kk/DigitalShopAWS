#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QVector>
#include <QVector3D>
#include <QString>
#include <QColor>
#include <QPoint>

/// Одна полоса диаграммы: подпись + значение.
struct ChartBar {
    QString label;
    double  value = 0.0;
};

/**
 * @brief 3D-диаграмма продаж на OpenGL.
 *
 * Рисует набор трёхмерных столбцов, высота которых пропорциональна
 * значению. Сцену можно вращать мышью (зажать и тянуть).
 *
 * Реализована на современном OpenGL (версия 3.3 Core):
 *   - вершинный и фрагментный шейдеры с тонированием граней;
 *   - VBO/VAO для геометрии куба, пола и линий сетки;
 *   - матрицы Model-View-Projection для 3D-проекции;
 *   - перспективная проекция с изометрическим видом сверху;
 *   - подписи месяцев и значений через QPainter поверх OpenGL.
 */
class SalesChart3D : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit SalesChart3D(QWidget* parent = nullptr);
    ~SalesChart3D() override;

    /// Задаёт данные для отображения и перерисовывает виджет.
    void setData(const QVector<ChartBar>& bars);

    /// Цвет столбца по индексу. Сохранено для совместимости с легендой.
    /// В новой версии возвращает единый цвет (бирюзовый) для всех столбцов.
    static QColor barColor(int index);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;

private:
    /// Выполняет всю отрисовку 3D-сцены (вызывается из paintGL).
    void drawScene();

    /// Рисует подписи (месяцы, значения) поверх OpenGL через QPainter.
    void drawLabels(QPainter& painter);

    /// Округляет максимум до "красивого" значения для шкалы.
    double niceMax(double rawMax) const;

    /// Перестраивает геометрию сетки и осей. Должен вызываться только
    /// когда OpenGL-контекст готов (m_initialized == true).
    void rebuildGridGeometry();

    /// Проецирует точку из мирового пространства в координаты виджета.
    /// Используется для размещения подписей возле 3D-объектов.
    QPointF worldToScreen(const QVector3D& world,
                          const QMatrix4x4& mvp) const;

    /// Форматирует сумму денег для подписи (например, "1.99k" или "350").
    static QString formatMoney(double value);

private:
    QOpenGLShaderProgram     m_program;

    // Геометрия столбца (единичный куб с тонированными гранями)
    QOpenGLBuffer            m_vbo{ QOpenGLBuffer::VertexBuffer };
    QOpenGLVertexArrayObject m_vao;

    // Геометрия пола (один прямоугольник)
    QOpenGLBuffer            m_floorVbo{ QOpenGLBuffer::VertexBuffer };
    QOpenGLVertexArrayObject m_floorVao;

    // Геометрия линий сетки и осей (динамически генерируются при ресайзе данных)
    QOpenGLBuffer            m_gridVbo{ QOpenGLBuffer::VertexBuffer };
    QOpenGLVertexArrayObject m_gridVao;
    int                      m_gridVertexCount = 0;

    QMatrix4x4 m_projection;

    // Кэш матриц последнего кадра — нужно для размещения подписей через QPainter.
    QMatrix4x4 m_lastView;
    QMatrix4x4 m_lastWorld;

    QVector<ChartBar> m_bars;
    double m_maxValue   = 1.0;  // фактический максимум данных
    double m_scaleMax   = 1.0;  // округлённый максимум для шкалы

    // Параметры размещения столбцов в мире (используются и при отрисовке,
    // и при расчёте позиций подписей).
    float  m_spacing      = 1.6f;
    float  m_barThickness = 0.7f;
    float  m_chartHeight  = 2.5f;
    float  m_floorPadding = 1.0f;

    float  m_yaw   = -25.0f;
    float  m_pitch =  28.0f;
    QPoint m_lastMouse;

    bool   m_initialized = false;   // OpenGL-контекст готов?
    bool   m_dataDirty   = false;   // нужно ли перестроить геометрию сетки
};
