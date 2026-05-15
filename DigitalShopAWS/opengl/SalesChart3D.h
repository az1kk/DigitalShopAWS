#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QVector>
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
 *   - вершинный и фрагментный шейдеры;
 *   - VBO/VAO для геометрии куба;
 *   - матрицы Model-View-Projection для 3D-проекции.
 */
class SalesChart3D : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit SalesChart3D(QWidget* parent = nullptr);
    ~SalesChart3D() override;

    /// Задаёт данные для отображения и перерисовывает виджет.
    void setData(const QVector<ChartBar>& bars);

    /// Цвет столбца по индексу. Публичный — чтобы легенда вкладки
    /// могла использовать те же цвета.
    static QColor barColor(int index);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;

private:
    QOpenGLShaderProgram     m_program;
    QOpenGLBuffer            m_vbo;
    QOpenGLVertexArrayObject m_vao;

    QMatrix4x4 m_projection;

    QVector<ChartBar> m_bars;
    double m_maxValue = 1.0;

    float  m_yaw = -30.0f;   // поворот сцены вокруг вертикальной оси
    float  m_pitch = 20.0f;    // наклон сцены
    QPoint m_lastMouse;
};