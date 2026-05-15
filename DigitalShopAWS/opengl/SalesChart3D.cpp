#include "SalesChart3D.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QVector4D>
#include <QFontMetrics>
#include <QtMath>

namespace {

    // === Геометрия столбца ===
    // 6 граней × 2 треугольника × 3 вершины = 36 вершин.
    // На каждой вершине 4 float: x, y, z, brightness.
    // Куб: x,z ∈ [-0.5; 0.5], y ∈ [0; 1] — основание лежит на плоскости y=0.
    const float kCubeData[] = {
        // нижняя грань (y=0) — самая тёмная, но обычно скрыта полом
        -0.5f,0.0f,-0.5f,0.45f,  0.5f,0.0f,-0.5f,0.45f,  0.5f,0.0f, 0.5f,0.45f,
        -0.5f,0.0f,-0.5f,0.45f,  0.5f,0.0f, 0.5f,0.45f, -0.5f,0.0f, 0.5f,0.45f,
        // верхняя грань (y=1) — самая светлая
        -0.5f,1.0f,-0.5f,1.00f, -0.5f,1.0f, 0.5f,1.00f,  0.5f,1.0f, 0.5f,1.00f,
        -0.5f,1.0f,-0.5f,1.00f,  0.5f,1.0f, 0.5f,1.00f,  0.5f,1.0f,-0.5f,1.00f,
        // передняя грань (z=+0.5)
        -0.5f,0.0f, 0.5f,0.82f,  0.5f,0.0f, 0.5f,0.82f,  0.5f,1.0f, 0.5f,0.82f,
        -0.5f,0.0f, 0.5f,0.82f,  0.5f,1.0f, 0.5f,0.82f, -0.5f,1.0f, 0.5f,0.82f,
        // задняя грань (z=-0.5)
         0.5f,0.0f,-0.5f,0.62f, -0.5f,0.0f,-0.5f,0.62f, -0.5f,1.0f,-0.5f,0.62f,
         0.5f,0.0f,-0.5f,0.62f, -0.5f,1.0f,-0.5f,0.62f,  0.5f,1.0f,-0.5f,0.62f,
         // левая грань (x=-0.5)
         -0.5f,0.0f,-0.5f,0.72f, -0.5f,0.0f, 0.5f,0.72f, -0.5f,1.0f, 0.5f,0.72f,
         -0.5f,0.0f,-0.5f,0.72f, -0.5f,1.0f, 0.5f,0.72f, -0.5f,1.0f,-0.5f,0.72f,
         // правая грань (x=+0.5)
          0.5f,0.0f, 0.5f,0.78f,  0.5f,0.0f,-0.5f,0.78f,  0.5f,1.0f,-0.5f,0.78f,
          0.5f,0.0f, 0.5f,0.78f,  0.5f,1.0f,-0.5f,0.78f,  0.5f,1.0f, 0.5f,0.78f,
    };

    // Геометрия пола — один прямоугольник в плоскости y=0.
    // Координаты в диапазоне [-1; +1], масштабируется при отрисовке.
    const float kFloorData[] = {
        -1.0f, 0.0f, -1.0f, 1.0f,
         1.0f, 0.0f, -1.0f, 1.0f,
         1.0f, 0.0f,  1.0f, 1.0f,
        -1.0f, 0.0f, -1.0f, 1.0f,
         1.0f, 0.0f,  1.0f, 1.0f,
        -1.0f, 0.0f,  1.0f, 1.0f,
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
    fmt.setSamples(4);   // сглаживание линий и краёв граней
    setFormat(fmt);

    setMinimumHeight(360);
}

SalesChart3D::~SalesChart3D()
{
    if (m_vbo.isCreated() || m_vao.isCreated() ||
        m_floorVbo.isCreated() || m_gridVbo.isCreated()) {
        makeCurrent();
        m_vbo.destroy();
        m_vao.destroy();
        m_floorVbo.destroy();
        m_floorVao.destroy();
        m_gridVbo.destroy();
        m_gridVao.destroy();
        doneCurrent();
    }
}

QColor SalesChart3D::barColor(int /*index*/)
{
    // Единый цвет для всех столбцов — бирюзово-синий.
    // Параметр index сохранён для совместимости с легендой.
    return QColor(56, 142, 200);
}

void SalesChart3D::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glClearColor(0.97f, 0.97f, 0.98f, 1.0f);

    // Компиляция шейдеров.
    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, kVertexShader);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, kFragmentShader);
    m_program.link();

    m_program.bind();

    // === VAO/VBO для столбца (куба) ===
    m_vao.create();
    m_vao.bind();

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(kCubeData, sizeof(kCubeData));

    const int stride = 4 * sizeof(float);
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3, stride);
    m_program.enableAttributeArray(1);
    m_program.setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 1, stride);

    m_vao.release();
    m_vbo.release();

    // === VAO/VBO для пола ===
    m_floorVao.create();
    m_floorVao.bind();

    m_floorVbo.create();
    m_floorVbo.bind();
    m_floorVbo.allocate(kFloorData, sizeof(kFloorData));

    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3, stride);
    m_program.enableAttributeArray(1);
    m_program.setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 1, stride);

    m_floorVao.release();
    m_floorVbo.release();

    // === VAO/VBO для сетки (будет заполнен в setData) ===
    m_gridVao.create();
    m_gridVao.bind();

    m_gridVbo.create();
    m_gridVbo.bind();
    // Аллокация с пустым размером — данные зальём позже в setData.
    m_gridVbo.allocate(nullptr, 0);

    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3, stride);
    m_program.enableAttributeArray(1);
    m_program.setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 1, stride);

    m_gridVao.release();
    m_gridVbo.release();

    m_program.release();

    m_initialized = true;

    // Если данные пришли до initializeGL — построим геометрию сейчас.
    if (m_dataDirty) {
        rebuildGridGeometry();
        m_dataDirty = false;
    }
}

void SalesChart3D::resizeGL(int w, int h)
{
    m_projection.setToIdentity();
    const float aspect = (h > 0) ? float(w) / float(h) : 1.0f;
    m_projection.perspective(38.0f, aspect, 0.1f, 100.0f);
}

double SalesChart3D::niceMax(double rawMax) const
{
    if (rawMax <= 0.0) return 1.0;
    // Округляем максимум до удобного значения для шкалы.
    // Например 4389 → 5000, 198 → 200, 15644 → 20000.
    const double exponent = std::pow(10.0, std::floor(std::log10(rawMax)));
    const double frac = rawMax / exponent;
    double niceFrac = 10.0;
    if (frac <= 1.0) niceFrac = 1.0;
    else if (frac <= 2.0) niceFrac = 2.0;
    else if (frac <= 2.5) niceFrac = 2.5;
    else if (frac <= 5.0) niceFrac = 5.0;
    return niceFrac * exponent;
}

QString SalesChart3D::formatMoney(double value)
{
    if (value <= 0.0) return QStringLiteral("0");
    if (value >= 1000.0) {
        // Например, 4389 → "4.4k", 15644 → "15.6k"
        const double k = value / 1000.0;
        return QString::number(k, 'f', (k < 10.0 ? 1 : 0)) + QStringLiteral("k");
    }
    return QString::number(value, 'f', 0);
}

void SalesChart3D::setData(const QVector<ChartBar>& bars)
{
    m_bars = bars;

    m_maxValue = 0.0;
    for (const ChartBar& b : m_bars) {
        if (b.value > m_maxValue) m_maxValue = b.value;
    }
    m_scaleMax = niceMax(m_maxValue);
    if (m_scaleMax < 1.0) m_scaleMax = 1.0;

    if (m_initialized) {
        rebuildGridGeometry();
    } else {
        // OpenGL ещё не готов — отложим до initializeGL.
        m_dataDirty = true;
    }

    update();
}

void SalesChart3D::rebuildGridGeometry()
{
    // Делим высоту графика на 5 уровней, рисуем горизонтальные линии
    // и линии задней/боковых границ "комнаты".
    QVector<float> grid;

    const int count       = m_bars.size();
    const float total     = qMax(1, count - 1) * m_spacing;
    const float halfX     = total / 2.0f + m_floorPadding;
    const float halfZ     = m_barThickness / 2.0f + m_floorPadding;

    auto addLine = [&](const QVector3D& a, const QVector3D& b, float bright) {
        grid << a.x() << a.y() << a.z() << bright;
        grid << b.x() << b.y() << b.z() << bright;
    };

    // Горизонтальные линии сетки (5 уровней), на задней стенке
    const int kLevels = 5;
    for (int i = 1; i <= kLevels; ++i) {
        const float y = (float)i / kLevels * m_chartHeight;
        addLine(QVector3D(-halfX, y, -halfZ),
                QVector3D( halfX, y, -halfZ),
                0.55f);
    }

    // Задние "стойки" — вертикальные линии по краям задней стенки
    addLine(QVector3D(-halfX, 0.0f, -halfZ),
            QVector3D(-halfX, m_chartHeight, -halfZ), 0.55f);
    addLine(QVector3D( halfX, 0.0f, -halfZ),
            QVector3D( halfX, m_chartHeight, -halfZ), 0.55f);

    // Линия основания на задней стенке
    addLine(QVector3D(-halfX, 0.0f, -halfZ),
            QVector3D( halfX, 0.0f, -halfZ), 0.40f);

    // Боковые линии пола (по передней кромке)
    addLine(QVector3D(-halfX, 0.0f,  halfZ),
            QVector3D( halfX, 0.0f,  halfZ), 0.40f);

    // Боковые соединения (правая и левая глубина пола)
    addLine(QVector3D(-halfX, 0.0f, -halfZ),
            QVector3D(-halfX, 0.0f,  halfZ), 0.40f);
    addLine(QVector3D( halfX, 0.0f, -halfZ),
            QVector3D( halfX, 0.0f,  halfZ), 0.40f);

    m_gridVertexCount = grid.size() / 4;

    makeCurrent();
    m_program.bind();
    m_gridVao.bind();
    m_gridVbo.bind();
    m_gridVbo.allocate(grid.constData(),
                       int(grid.size() * sizeof(float)));
    const int stride = 4 * sizeof(float);
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3, stride);
    m_program.enableAttributeArray(1);
    m_program.setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 1, stride);
    m_gridVao.release();
    m_gridVbo.release();
    m_program.release();
    doneCurrent();
}

void SalesChart3D::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_bars.isEmpty() || !m_initialized) {
        return;
    }

    drawScene();

    // Подписи рисуем поверх 3D-сцены через QPainter.
    // QOpenGLWidget позволяет это: paintGL может быть совмещён с QPainter,
    // если использовать QPainter напрямую на этом виджете.
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    drawLabels(painter);
    painter.end();
}

void SalesChart3D::drawScene()
{
    // Камера адаптивно отъезжает в зависимости от ширины сцены,
    // чтобы все столбцы помещались в кадр на любом количестве данных.
    const int   count = m_bars.size();
    const float total = qMax(1, count - 1) * m_spacing;
    const float halfX = total / 2.0f + m_floorPadding;

    // Дистанция камеры по Z подбирается так, чтобы вся ширина "комнаты"
    // влезала по горизонтали с небольшим запасом.
    // tan(FOV/2) задаёт половину видимой ширины на единицу расстояния,
    // с поправкой на соотношение сторон.
    const float fovDeg = 38.0f;
    const float aspect = (height() > 0)
        ? float(width()) / float(height())
        : 1.0f;
    const float halfFovRad = qDegreesToRadians(fovDeg / 2.0f);
    const float horizHalfTan = std::tan(halfFovRad) * aspect;

    // Минимальная дистанция, чтобы по ширине влезло (halfX) с запасом 15%.
    const float distForWidth = (halfX * 1.15f) / horizHalfTan;
    // Минимальная дистанция, чтобы по высоте влезло.
    const float distForHeight = (m_chartHeight * 0.85f) / std::tan(halfFovRad);
    // Берём ту, что больше — то есть ту, при которой влезает оба измерения.
    const float camDist = qMax(distForWidth, distForHeight);

    QMatrix4x4 view;
    view.lookAt(QVector3D(0.0f, m_chartHeight * 1.3f, camDist),   // позиция
        QVector3D(0.0f, m_chartHeight * 0.4f, 0.0f),       // центр
        QVector3D(0.0f, 1.0f, 0.0f));                       // вверх

    // Поворот всей сцены по управлению мышью.
    QMatrix4x4 world;
    world.rotate(m_pitch, 1.0f, 0.0f, 0.0f);
    world.rotate(m_yaw, 0.0f, 1.0f, 0.0f);

    m_lastView = view;
    m_lastWorld = world;

    m_program.bind();

    // === Рисуем пол ===
    {
        QMatrix4x4 model;
        const float halfZ = m_barThickness / 2.0f + m_floorPadding;
        model.translate(0.0f, -0.001f, 0.0f); // чуть ниже, чтобы избежать z-fighting
        model.scale(halfX, 1.0f, halfZ);

        const QMatrix4x4 mvp = m_projection * view * world * model;
        m_program.setUniformValue("uMvp", mvp);
        m_program.setUniformValue("uColor", QVector3D(0.92f, 0.92f, 0.94f));

        m_floorVao.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        m_floorVao.release();
    }

    // === Рисуем сетку и оси ===
    if (m_gridVertexCount > 0) {
        QMatrix4x4 model; // identity
        const QMatrix4x4 mvp = m_projection * view * world * model;
        m_program.setUniformValue("uMvp", mvp);
        m_program.setUniformValue("uColor", QVector3D(0.62f, 0.65f, 0.70f));

        glLineWidth(1.2f);
        m_gridVao.bind();
        glDrawArrays(GL_LINES, 0, m_gridVertexCount);
        m_gridVao.release();
    }

    // === Рисуем столбцы ===
    m_vao.bind();

    for (int i = 0; i < count; ++i) {
        const float x = i * m_spacing - total / 2.0f;
        const float h = (m_scaleMax > 0.0)
            ? float(m_bars[i].value / m_scaleMax) * m_chartHeight
            : 0.0f;

        if (h < 0.005f) continue;

        QMatrix4x4 model;
        model.translate(x, 0.0f, 0.0f);
        model.scale(m_barThickness, h, m_barThickness);

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

QPointF SalesChart3D::worldToScreen(const QVector3D& world,
                                    const QMatrix4x4& mvp) const
{
    QVector4D clip = mvp.map(QVector4D(world, 1.0f));
    if (qFuzzyIsNull(clip.w())) return QPointF(-1, -1);
    const float ndcX = clip.x() / clip.w();
    const float ndcY = clip.y() / clip.w();
    const float screenX = (ndcX * 0.5f + 0.5f) * width();
    const float screenY = (1.0f - (ndcY * 0.5f + 0.5f)) * height();
    return QPointF(screenX, screenY);
}

void SalesChart3D::drawLabels(QPainter& painter)
{
    const int count = m_bars.size();
    const float total = qMax(1, count - 1) * m_spacing;

    QFont labelFont = painter.font();
    labelFont.setPointSize(qMax(8, labelFont.pointSize()));
    painter.setFont(labelFont);
    const QFontMetrics fm(labelFont);

    const QMatrix4x4 mvp = m_projection * m_lastView * m_lastWorld;

    for (int i = 0; i < count; ++i) {
        const float x = i * m_spacing - total / 2.0f;
        const float h = (m_scaleMax > 0.0)
                            ? float(m_bars[i].value / m_scaleMax) * m_chartHeight
                            : 0.0f;

        // === Подпись месяца под столбцом (на полу) ===
        const QVector3D base(x, 0.0f, m_barThickness / 2.0f + 0.15f);
        const QPointF basePt = worldToScreen(base, mvp);

        if (basePt.x() >= 0 && basePt.x() < width() &&
            basePt.y() >= 0 && basePt.y() < height()) {
            const QString text = m_bars[i].label;
            const QRect textRect = fm.boundingRect(text);
            painter.setPen(QColor(60, 60, 70));
            painter.drawText(QPointF(basePt.x() - textRect.width() / 2.0,
                                     basePt.y() + 16.0),
                             text);
        }

        // === Подпись значения над столбцом ===
        if (m_bars[i].value <= 0.0) continue;

        const QVector3D top(x, h + 0.08f, 0.0f);
        const QPointF topPt = worldToScreen(top, mvp);

        if (topPt.x() >= 0 && topPt.x() < width() &&
            topPt.y() >= 0 && topPt.y() < height()) {
            const QString text = formatMoney(m_bars[i].value);
            const QRect textRect = fm.boundingRect(text);
            // Полупрозрачная подложка для читаемости
            const QRectF bg(topPt.x() - textRect.width() / 2.0 - 4,
                            topPt.y() - textRect.height() + 2,
                            textRect.width() + 8,
                            textRect.height() + 2);
            painter.setBrush(QColor(255, 255, 255, 200));
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(bg, 3, 3);

            painter.setPen(QColor(36, 90, 130));
            QFont boldFont = labelFont;
            boldFont.setBold(true);
            painter.setFont(boldFont);
            painter.drawText(QPointF(topPt.x() - textRect.width() / 2.0,
                                     topPt.y()),
                             text);
            painter.setFont(labelFont);
        }
    }

    // === Подписи шкалы по левому краю задней стенки ===
    const float halfX = total / 2.0f + m_floorPadding;
    const float halfZ = m_barThickness / 2.0f + m_floorPadding;
    const int kLevels = 5;
    painter.setPen(QColor(90, 95, 105));
    for (int i = 0; i <= kLevels; ++i) {
        const float y = (float)i / kLevels * m_chartHeight;
        const double value = (double)i / kLevels * m_scaleMax;
        const QVector3D worldPt(-halfX - 0.05f, y, -halfZ);
        const QPointF pt = worldToScreen(worldPt, mvp);

        if (pt.x() >= 0 && pt.x() < width() &&
            pt.y() >= 0 && pt.y() < height()) {
            const QString text = formatMoney(value);
            const QRect textRect = fm.boundingRect(text);
            painter.drawText(QPointF(pt.x() - textRect.width() - 4.0,
                                     pt.y() + textRect.height() / 3.0),
                             text);
        }
    }
}

void SalesChart3D::mousePressEvent(QMouseEvent* e)
{
    m_lastMouse = e->pos();
}

void SalesChart3D::mouseMoveEvent(QMouseEvent* e)
{
    const QPoint delta = e->pos() - m_lastMouse;
    m_lastMouse = e->pos();

    m_yaw   += delta.x() * 0.4f;
    m_pitch += delta.y() * 0.4f;

    // Ограничиваем наклон, чтобы не перевернуть сцену.
    m_pitch = qBound(5.0f, m_pitch, 80.0f);

    update();
}
