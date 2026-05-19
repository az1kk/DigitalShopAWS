#include "SalesChart3D.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QVector4D>
#include <QFontMetrics>
#include <QtMath>

namespace {

    // === Геометрия столбца ===
    // Плотный куб без зазоров. 36 вершин (6 граней x 2 треугольника).
    // На вершину: x, y, z, brightness. Куб: x,z in [-0.5;0.5], y in [0;1].
    const float kCubeData[] = {
        -0.5f,1.0f,-0.5f,1.00f, -0.5f,1.0f, 0.5f,1.00f,  0.5f,1.0f, 0.5f,1.00f,
        -0.5f,1.0f,-0.5f,1.00f,  0.5f,1.0f, 0.5f,1.00f,  0.5f,1.0f,-0.5f,1.00f,
        -0.5f,0.0f, 0.5f,0.88f,  0.5f,0.0f, 0.5f,0.88f,  0.5f,1.0f, 0.5f,0.88f,
        -0.5f,0.0f, 0.5f,0.88f,  0.5f,1.0f, 0.5f,0.88f, -0.5f,1.0f, 0.5f,0.88f,
         0.5f,0.0f, 0.5f,0.74f,  0.5f,0.0f,-0.5f,0.74f,  0.5f,1.0f,-0.5f,0.74f,
         0.5f,0.0f, 0.5f,0.74f,  0.5f,1.0f,-0.5f,0.74f,  0.5f,1.0f, 0.5f,0.74f,
         -0.5f,0.0f,-0.5f,0.68f, -0.5f,0.0f, 0.5f,0.68f, -0.5f,1.0f, 0.5f,0.68f,
         -0.5f,0.0f,-0.5f,0.68f, -0.5f,1.0f, 0.5f,0.68f, -0.5f,1.0f,-0.5f,0.68f,
          0.5f,0.0f,-0.5f,0.60f, -0.5f,0.0f,-0.5f,0.60f, -0.5f,1.0f,-0.5f,0.60f,
          0.5f,0.0f,-0.5f,0.60f, -0.5f,1.0f,-0.5f,0.60f,  0.5f,1.0f,-0.5f,0.60f,
          -0.5f,0.0f,-0.5f,0.52f,  0.5f,0.0f,-0.5f,0.52f,  0.5f,0.0f, 0.5f,0.52f,
          -0.5f,0.0f,-0.5f,0.52f,  0.5f,0.0f, 0.5f,0.52f, -0.5f,0.0f, 0.5f,0.52f,
    };

    const float kFloorData[] = {
        -1.0f, 0.0f, -1.0f, 1.0f,  1.0f, 0.0f, -1.0f, 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
        -1.0f, 0.0f, -1.0f, 1.0f,  1.0f, 0.0f,  1.0f, 1.0f, -1.0f, 0.0f,  1.0f, 1.0f,
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
void main() { FragColor = vec4(vColor, 1.0); }
)";

} // namespace


SalesChart3D::SalesChart3D(QWidget* parent)
    : QOpenGLWidget(parent)
{
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setDepthBufferSize(24);
    fmt.setSamples(4);
    setFormat(fmt);
    setMinimumHeight(360);

    // Лёгкий изометрический разворот. Небольшие углы — объём есть,
    // но при ОРТОГРАФИЧЕСКОЙ проекции высота столбцов точно
    // соответствует горизонтальным линиям сетки.
    m_yaw   = -18.0f;
    m_pitch =  16.0f;
}

SalesChart3D::~SalesChart3D()
{
    if (m_vbo.isCreated() || m_vao.isCreated() ||
        m_floorVbo.isCreated() || m_gridVbo.isCreated()) {
        makeCurrent();
        m_vbo.destroy(); m_vao.destroy();
        m_floorVbo.destroy(); m_floorVao.destroy();
        m_gridVbo.destroy(); m_gridVao.destroy();
        doneCurrent();
    }
}

QColor SalesChart3D::barColor(int /*index*/)
{
    return QColor(54, 120, 178);
}

void SalesChart3D::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glClearColor(0.975f, 0.978f, 0.985f, 1.0f);

    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, kVertexShader);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, kFragmentShader);
    m_program.link();
    m_program.bind();
    const int stride = 4 * sizeof(float);

    m_vao.create(); m_vao.bind();
    m_vbo.create(); m_vbo.bind();
    m_vbo.allocate(kCubeData, sizeof(kCubeData));
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3, stride);
    m_program.enableAttributeArray(1);
    m_program.setAttributeBuffer(1, GL_FLOAT, 3*sizeof(float), 1, stride);
    m_vao.release(); m_vbo.release();

    m_floorVao.create(); m_floorVao.bind();
    m_floorVbo.create(); m_floorVbo.bind();
    m_floorVbo.allocate(kFloorData, sizeof(kFloorData));
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3, stride);
    m_program.enableAttributeArray(1);
    m_program.setAttributeBuffer(1, GL_FLOAT, 3*sizeof(float), 1, stride);
    m_floorVao.release(); m_floorVbo.release();

    m_gridVao.create(); m_gridVao.bind();
    m_gridVbo.create(); m_gridVbo.bind();
    m_gridVbo.allocate(nullptr, 0);
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3, stride);
    m_program.enableAttributeArray(1);
    m_program.setAttributeBuffer(1, GL_FLOAT, 3*sizeof(float), 1, stride);
    m_gridVao.release(); m_gridVbo.release();

    m_program.release();
    m_initialized = true;
    if (m_dataDirty) { rebuildGridGeometry(); m_dataDirty = false; }
}

void SalesChart3D::resizeGL(int w, int h)
{
    // Ортографическая проекция настраивается в drawScene
    // (зависит от ширины сцены). Здесь только сохраняем размер.
    Q_UNUSED(w); Q_UNUSED(h);
}

double SalesChart3D::niceMax(double rawMax) const
{
    if (rawMax <= 0.0) return 1.0;
    const double e = std::pow(10.0, std::floor(std::log10(rawMax)));
    const double f = rawMax / e;
    double nf = 10.0;
    if (f <= 1.0) nf = 1.0;
    else if (f <= 2.0) nf = 2.0;
    else if (f <= 2.5) nf = 2.5;
    else if (f <= 5.0) nf = 5.0;
    return nf * e;
}

QString SalesChart3D::formatMoney(double value)
{
    if (value <= 0.0) return QStringLiteral("0");
    if (value >= 1000.0) {
        const double k = value / 1000.0;
        return QString::number(k, 'f', (k < 10.0 ? 1 : 0)) + QStringLiteral("k");
    }
    return QString::number(value, 'f', 0);
}

void SalesChart3D::setData(const QVector<ChartBar>& bars)
{
    m_bars = bars;
    m_maxValue = 0.0;
    for (const ChartBar& b : m_bars)
        if (b.value > m_maxValue) m_maxValue = b.value;
    m_scaleMax = niceMax(m_maxValue);
    if (m_scaleMax < 1.0) m_scaleMax = 1.0;
    if (m_initialized) rebuildGridGeometry();
    else m_dataDirty = true;
    update();
}

void SalesChart3D::rebuildGridGeometry()
{
    QVector<float> grid;
    const int count   = m_bars.size();
    const float total = qMax(1, count - 1) * m_spacing;
    const float halfX = total / 2.0f + m_floorPadding;
    const float backZ = -(m_barThickness / 2.0f + m_floorPadding);
    const float frontZ =  m_barThickness / 2.0f + m_floorPadding;

    auto addLine = [&](const QVector3D& a, const QVector3D& b, float br) {
        grid << a.x() << a.y() << a.z() << br;
        grid << b.x() << b.y() << b.z() << br;
    };

    const int kLevels = 5;
    // Горизонтальные линии шкалы рисуем КАК ПЛОСКОСТИ-СТУПЕНИ:
    // линия на задней стенке + линия по верху, протянутая вперёд.
    // Это создаёт "ступенчатый" фон, по которому глаз точно
    // считывает высоту столбца относительно шкалы.
    for (int i = 1; i <= kLevels; ++i) {
        const float y = (float)i / kLevels * m_chartHeight;
        // Линия на задней стенке (яркая)
        addLine(QVector3D(-halfX, y, backZ),
                QVector3D( halfX, y, backZ), 0.66f);
        // Та же линия, но протянутая к переднему краю пола (бледнее) —
        // визуально "полка", помогает сопоставить высоту.
        addLine(QVector3D(-halfX, y, backZ),
                QVector3D(-halfX, y, frontZ), 0.40f);
        addLine(QVector3D( halfX, y, backZ),
                QVector3D( halfX, y, frontZ), 0.40f);
    }
    // Вертикальные стойки задней стенки
    addLine(QVector3D(-halfX, 0.0f, backZ),
            QVector3D(-halfX, m_chartHeight, backZ), 0.66f);
    addLine(QVector3D( halfX, 0.0f, backZ),
            QVector3D( halfX, m_chartHeight, backZ), 0.66f);
    // Контур пола
    addLine(QVector3D(-halfX, 0.0f, backZ),  QVector3D( halfX, 0.0f, backZ), 0.46f);
    addLine(QVector3D(-halfX, 0.0f, frontZ), QVector3D( halfX, 0.0f, frontZ), 0.46f);
    addLine(QVector3D(-halfX, 0.0f, backZ),  QVector3D(-halfX, 0.0f, frontZ), 0.46f);
    addLine(QVector3D( halfX, 0.0f, backZ),  QVector3D( halfX, 0.0f, frontZ), 0.46f);

    m_gridVertexCount = grid.size() / 4;

    makeCurrent();
    m_program.bind();
    m_gridVao.bind(); m_gridVbo.bind();
    m_gridVbo.allocate(grid.constData(), int(grid.size() * sizeof(float)));
    const int stride = 4 * sizeof(float);
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3, stride);
    m_program.enableAttributeArray(1);
    m_program.setAttributeBuffer(1, GL_FLOAT, 3*sizeof(float), 1, stride);
    m_gridVao.release(); m_gridVbo.release();
    m_program.release();
    doneCurrent();
}

void SalesChart3D::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (m_bars.isEmpty() || !m_initialized) return;
    drawScene();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    drawLabels(painter);
    painter.end();
}

void SalesChart3D::drawScene()
{
    const int   count = m_bars.size();
    const float total = qMax(1, count - 1) * m_spacing;
    const float halfX = total / 2.0f + m_floorPadding;
    const float halfZ = m_barThickness / 2.0f + m_floorPadding;

    // === ОРТОГРАФИЧЕСКАЯ проекция ===
    // Нет схождения линий в перспективе => высота столбца строго
    // пропорциональна и точно ложится на сетку. Это и делает
    // диаграмму "практичной" и читаемой.
    const float aspect = (height() > 0)
                             ? float(width()) / float(height()) : 1.0f;
    // Габарит сцены по диагонали (с учётом наклона) — задаём поле зрения.
    const float spanX = halfX * 1.18f;
    const float spanY = m_chartHeight * 1.30f;
    float orthoX, orthoY;
    if (spanX / spanY > aspect) { orthoX = spanX; orthoY = spanX / aspect; }
    else                        { orthoY = spanY; orthoX = spanY * aspect; }

    QMatrix4x4 proj;
    proj.ortho(-orthoX, orthoX, -orthoY * 0.55f, orthoY * 1.45f,
               -100.0f, 100.0f);
    m_projection = proj;

    QMatrix4x4 view; // камера в ортопроекции — просто единичная
    QMatrix4x4 world;
    world.rotate(m_pitch, 1.0f, 0.0f, 0.0f);
    world.rotate(m_yaw,   0.0f, 1.0f, 0.0f);

    m_lastView  = view;
    m_lastWorld = world;

    m_program.bind();

    // Пол (двусторонний — изнанка не просвечивает)
    {
        glDisable(GL_CULL_FACE);
        QMatrix4x4 model;
        model.translate(0.0f, -0.002f, 0.0f);
        model.scale(halfX, 1.0f, halfZ);
        m_program.setUniformValue("uMvp", m_projection * view * world * model);
        m_program.setUniformValue("uColor", QVector3D(0.91f, 0.92f, 0.94f));
        m_floorVao.bind(); glDrawArrays(GL_TRIANGLES, 0, 6); m_floorVao.release();
    }

    // Сетка
    if (m_gridVertexCount > 0) {
        m_program.setUniformValue("uMvp", m_projection * view * world);
        m_program.setUniformValue("uColor", QVector3D(0.58f, 0.61f, 0.67f));
        glLineWidth(1.3f);
        m_gridVao.bind();
        glDrawArrays(GL_LINES, 0, m_gridVertexCount);
        m_gridVao.release();
    }

    // Столбцы
    m_vao.bind();
    for (int i = 0; i < count; ++i) {
        const float x = i * m_spacing - total / 2.0f;
        const float h = (m_scaleMax > 0.0)
                            ? float(m_bars[i].value / m_scaleMax) * m_chartHeight
                            : 0.0f;
        if (h < 0.004f) continue;
        QMatrix4x4 model;
        model.translate(x, 0.0f, 0.0f);
        model.scale(m_barThickness, h, m_barThickness);
        m_program.setUniformValue("uMvp", m_projection * view * world * model);
        const QColor c = barColor(i);
        m_program.setUniformValue("uColor",
                                  QVector3D(c.redF(), c.greenF(), c.blueF()));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    m_vao.release();
    m_program.release();
}

QPointF SalesChart3D::worldToScreen(const QVector3D& world,
                                    const QMatrix4x4& mvp) const
{
    QVector4D clip = mvp.map(QVector4D(world, 1.0f));
    if (qFuzzyIsNull(clip.w())) return QPointF(-1, -1);
    const float ndcX = clip.x() / clip.w();
    const float ndcY = clip.y() / clip.w();
    return QPointF((ndcX * 0.5f + 0.5f) * width(),
                   (1.0f - (ndcY * 0.5f + 0.5f)) * height());
}

void SalesChart3D::drawLabels(QPainter& painter)
{
    const int count = m_bars.size();
    const float total = qMax(1, count - 1) * m_spacing;
    QFont lf = painter.font();
    lf.setPointSize(qMax(9, lf.pointSize()));
    painter.setFont(lf);
    const QFontMetrics fm(lf);
    const QMatrix4x4 mvp = m_projection * m_lastView * m_lastWorld;

    for (int i = 0; i < count; ++i) {
        const float x = i * m_spacing - total / 2.0f;
        const float h = (m_scaleMax > 0.0)
                            ? float(m_bars[i].value / m_scaleMax) * m_chartHeight
                            : 0.0f;

        const QVector3D base(x, 0.0f, m_barThickness / 2.0f + 0.20f);
        const QPointF bp = worldToScreen(base, mvp);
        if (bp.x() >= 0 && bp.x() < width() && bp.y() >= 0 && bp.y() < height()) {
            const QString t = m_bars[i].label;
            const QRect r = fm.boundingRect(t);
            painter.setPen(QColor(52, 55, 63));
            painter.drawText(QPointF(bp.x() - r.width()/2.0, bp.y()+18.0), t);
        }
        if (m_bars[i].value <= 0.0) continue;
        const QVector3D top(x, h + 0.10f, 0.0f);
        const QPointF tp = worldToScreen(top, mvp);
        if (tp.x() >= 0 && tp.x() < width() && tp.y() >= 0 && tp.y() < height()) {
            const QString t = formatMoney(m_bars[i].value);
            const QRect r = fm.boundingRect(t);
            const QRectF bg(tp.x()-r.width()/2.0-5, tp.y()-r.height()+1,
                            r.width()+10, r.height()+3);
            painter.setBrush(QColor(255,255,255,215));
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(bg, 4, 4);
            painter.setPen(QColor(28, 78, 122));
            QFont bf = lf; bf.setBold(true);
            painter.setFont(bf);
            painter.drawText(QPointF(tp.x()-r.width()/2.0, tp.y()), t);
            painter.setFont(lf);
        }
    }

    const float halfX = total / 2.0f + m_floorPadding;
    const float backZ = -(m_barThickness / 2.0f + m_floorPadding);
    const int kLevels = 5;
    painter.setPen(QColor(92, 96, 105));
    for (int i = 0; i <= kLevels; ++i) {
        const float y = (float)i / kLevels * m_chartHeight;
        const double value = (double)i / kLevels * m_scaleMax;
        const QVector3D wp(-halfX - 0.05f, y, backZ);
        const QPointF p = worldToScreen(wp, mvp);
        if (p.x() >= 0 && p.x() < width() && p.y() >= 0 && p.y() < height()) {
            const QString t = formatMoney(value);
            const QRect r = fm.boundingRect(t);
            painter.drawText(QPointF(p.x()-r.width()-6.0,
                                     p.y()+r.height()/3.0), t);
        }
    }
}

void SalesChart3D::mousePressEvent(QMouseEvent* e)
{
    m_lastMouse = e->pos();
}

void SalesChart3D::mouseMoveEvent(QMouseEvent* e)
{
    const QPoint d = e->pos() - m_lastMouse;
    m_lastMouse = e->pos();
    m_yaw   += d.x() * 0.30f;
    m_pitch += d.y() * 0.30f;
    // Узкий аккуратный диапазон — диаграмма остаётся читаемой,
    // дно не видно, перекоса нет.
    m_pitch = qBound(6.0f, m_pitch, 38.0f);
    m_yaw   = qBound(-40.0f, m_yaw, 40.0f);
    update();
}
