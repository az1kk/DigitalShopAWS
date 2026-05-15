#include "SalesChart3D.h"

#include <QMouseEvent>

namespace {

    // Геометрия единичного куба: 36 вершин (6 граней по 2 треугольника).
    // Каждая вершина — 4 float: x, y, z, brightness.
    // brightness — затемнение грани, чтобы куб выглядел объёмным
    // без расчёта освещения.
    // Куб: x,z ∈ [-0.5; 0.5], y ∈ [0; 1] — основание лежит на плоскости y=0.
    const float kCubeData[] = {
        // нижняя грань (y=0), темнее всего
        -0.5f,0.0f,-0.5f,0.40f,  0.5f,0.0f,-0.5f,0.40f,  0.5f,0.0f, 0.5f,0.40f,
        -0.5f,0.0f,-0.5f,0.40f,  0.5f,0.0f, 0.5f,0.40f, -0.5f,0.0f, 0.5f,0.40f,
        // верхняя грань (y=1), самая светлая
        -0.5f,1.0f,-0.5f,1.00f, -0.5f,1.0f, 0.5f,1.00f,  0.5f,1.0f, 0.5f,1.00f,
        -0.5f,1.0f,-0.5f,1.00f,  0.5f,1.0f, 0.5f,1.00f,  0.5f,1.0f,-0.5f,1.00f,
        // передняя грань (z=0.5)
        -0.5f,0.0f, 0.5f,0.85f,  0.5f,0.0f, 0.5f,0.85f,  0.5f,1.0f, 0.5f,0.85f,
        -0.5f,0.0f, 0.5f,0.85f,  0.5f,1.0f, 0.5f,0.85f, -0.5f,1.0f, 0.5f,0.85f,
        // задняя грань (z=-0.5)
         0.5f,0.0f,-0.5f,0.65f, -0.5f,0.0f,-0.5f,0.65f, -0.5f,1.0f,-0.5f,0.65f,
         0.5f,0.0f,-0.5f,0.65f, -0.5f,1.0f,-0.5f,0.65f,  0.5f,1.0f,-0.5f,0.65f,
         // левая грань (x=-0.5)
         -0.5f,0.0f,-0.5f,0.70f, -0.5f,0.0f, 0.5f,0.70f, -0.5f,1.0f, 0.5f,0.70f,
         -0.5f,0.0f,-0.5f,0.70f, -0.5f,1.0f, 0.5f,0.70f, -0.5f,1.0f,-0.5f,0.70f,
         // правая грань (x=0.5)
          0.5f,0.0f, 0.5f,0.75f,  0.5f,0.0f,-0.5f,0.75f,  0.5f,1.0f,-0.5f,0.75f,
          0.5f,0.0f, 0.5f,0.75f,  0.5f,1.0f,-0.5f,0.75f,  0.5f,1.0f, 0.5f,0.75f,
    };

    const char* kVertexShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in float aBright;
uniform mat4 uMvp;
uniform vec3 uColor;
out vec3 vColor;
void main() {
    gl_Position = uMvp * vec4(aPos, 1.0);
    vColor = uColor * aBright;
}
)";

    const char* kFragmentShader = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

} // namespace

SalesChart3D::SalesChart3D(QWidget* parent)
    : QOpenGLWidget(parent)
{
    // Запрашиваем контекст OpenGL 3.3 Core с буфером глубины.
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setDepthBufferSize(24);
    setFormat(fmt);

    setMinimumHeight(320);
}

SalesChart3D::~SalesChart3D()
{
    // Освобождать ресурсы OpenGL нужно при активном контексте.
    if (m_vbo.isCreated() || m_vao.isCreated()) {
        makeCurrent();
        m_vbo.destroy();
        m_vao.destroy();
        doneCurrent();
    }
}

QColor SalesChart3D::barColor(int index)
{
    // Палитра различимых цветов; при выходе за размер — повтор по кругу.
    static const QColor palette[] = {
        QColor(66, 133, 244),  // синий
        QColor(219,  68,  55),  // красный
        QColor(244, 180,   0),  // жёлтый
        QColor(15, 157,  88),  // зелёный
        QColor(171,  71, 188),  // фиолетовый
        QColor(255, 112,  67),  // оранжевый
        QColor(0, 172, 193),  // бирюзовый
        QColor(120, 144, 156),  // серо-синий
    };
    const int count = sizeof(palette) / sizeof(palette[0]);
    return palette[index % count];
}

void SalesChart3D::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.96f, 0.96f, 0.97f, 1.0f);

    // Компиляция и сборка шейдерной программы.
    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, kVertexShader);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, kFragmentShader);
    m_program.link();

    // Загрузка геометрии куба в видеопамять.
    m_program.bind();

    m_vao.create();
    m_vao.bind();

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(kCubeData, sizeof(kCubeData));

    // Атрибут 0 — позиция (3 float), атрибут 1 — яркость (1 float).
    const int stride = 4 * sizeof(float);
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3, stride);
    m_program.enableAttributeArray(1);
    m_program.setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 1, stride);

    m_vao.release();
    m_vbo.release();
    m_program.release();
}

void SalesChart3D::resizeGL(int w, int h)
{
    m_projection.setToIdentity();
    const float aspect = (h > 0) ? float(w) / float(h) : 1.0f;
    m_projection.perspective(45.0f, aspect, 0.1f, 100.0f);
}

void SalesChart3D::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_bars.isEmpty()) {
        return;
    }

    // Камера: смотрим на сцену немного сверху и сбоку.
    QMatrix4x4 view;
    view.lookAt(QVector3D(0.0f, 2.2f, 6.0f),   // позиция камеры
        QVector3D(0.0f, 1.0f, 0.0f),   // куда смотрит
        QVector3D(0.0f, 1.0f, 0.0f));  // вектор "вверх"

    // Поворот всей сцены по управлению мышью.
    QMatrix4x4 world;
    world.rotate(m_pitch, 1.0f, 0.0f, 0.0f);
    world.rotate(m_yaw, 0.0f, 1.0f, 0.0f);

    m_program.bind();
    m_vao.bind();

    const int   count = m_bars.size();
    const float spacing = 1.3f;
    const float totalWidth = (count - 1) * spacing;
    const float maxHeight = 2.5f;
    const float barThickness = 0.7f;

    for (int i = 0; i < count; ++i) {
        const float x = i * spacing - totalWidth / 2.0f;
        const float h = (m_maxValue > 0.0)
            ? float(m_bars[i].value / m_maxValue) * maxHeight
            : 0.0f;

        // Матрица модели: сдвиг по X + масштаб по высоте.
        QMatrix4x4 model;
        model.translate(x, 0.0f, 0.0f);
        model.scale(barThickness, qMax(h, 0.001f), barThickness);

        const QMatrix4x4 mvp = m_projection * view * world * model;
        m_program.setUniformValue("uMvp", mvp);

        const QColor c = barColor(i);
        m_program.setUniformValue("uColor",
            QVector3D(c.redF(), c.greenF(), c.blueF()));

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    m_vao.release();
    m_program.release();
}

void SalesChart3D::setData(const QVector<ChartBar>& bars)
{
    m_bars = bars;

    m_maxValue = 1.0;
    for (const ChartBar& b : m_bars) {
        if (b.value > m_maxValue) {
            m_maxValue = b.value;
        }
    }

    update();  // запросить перерисовку
}

void SalesChart3D::mousePressEvent(QMouseEvent* e)
{
    m_lastMouse = e->pos();
}

void SalesChart3D::mouseMoveEvent(QMouseEvent* e)
{
    const QPoint delta = e->pos() - m_lastMouse;
    m_lastMouse = e->pos();

    m_yaw += delta.x() * 0.5f;
    m_pitch += delta.y() * 0.5f;

    // Ограничиваем наклон, чтобы не перевернуть сцену.
    m_pitch = qBound(-5.0f, m_pitch, 85.0f);

    update();
}