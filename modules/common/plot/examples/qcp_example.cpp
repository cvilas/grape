//=================================================================================================
// Copyright (C) 2026 GRAPE Contributors
//=================================================================================================

/// Demonstrates realtime plotting using Qt6 and QCustomPlot — mirrors example.cpp

#include <QApplication>
#include <QTimer>
#include <chrono>
#include <cmath>
#include <numbers>

#include "qcustomplot.h"

//=================================================================================================
/// Plot example using QCustomPlot. Mirrors example.cpp, but adapted to Qt6 and QCustomPlot's API.
///
auto main(int argc, char* argv[]) -> int {
  const QApplication app(argc, argv);

  static constexpr auto WINDOW_WIDTH = 960;
  static constexpr auto WINDOW_HEIGHT = 600;
  static constexpr auto X_RANGE_S = 65.0;
  static constexpr auto Y_RANGE = 1.1;
  static constexpr auto DATA_INTERVAL_MS = 1;
  static constexpr auto RENDER_INTERVAL_MS = 16;  // ~60 fps

  QCustomPlot plot;
  plot.setWindowTitle("Plot demo");
  plot.resize(WINDOW_WIDTH, WINDOW_HEIGHT);
  plot.xAxis->setLabel("Time (s)");
  plot.yAxis->setLabel("Amplitude");
  plot.yAxis->setRange(-Y_RANGE, Y_RANGE);
  plot.legend->setVisible(true);

  using clock = std::chrono::steady_clock;
  const auto t0 = clock::now();

  // FPS counter: QCPItemText overlay updated via afterReplot signal
  auto* fps_label = new QCPItemText(&plot);  // NOLINT(cppcoreguidelines-owning-memory)
  fps_label->setPositionAlignment(Qt::AlignTop | Qt::AlignLeft);
  fps_label->position->setType(QCPItemPosition::ptAxisRectRatio);
  fps_label->position->setCoords(0.01, 0.01);
  fps_label->setFont(QFont("monospace", 9));
  fps_label->setText("FPS: --");
  auto last_replot = clock::now();
  QObject::connect(&plot, &QCustomPlot::afterReplot, [&]() {
    const auto now = clock::now();
    const double fps = 1.0 / std::chrono::duration<double>(now - last_replot).count();
    last_replot = now;
    fps_label->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));
  });

  // Sine — solid line (mirrors LineStyle::Line)
  QCPGraph* const sine_graph = plot.addGraph();
  sine_graph->setName("Sine");
  sine_graph->setPen(QPen(Qt::blue, 1.5));

  // Cosine — step style (mirrors LineStyle::Step)
  QCPGraph* const cosine_graph = plot.addGraph();
  cosine_graph->setName("Cosine");
  cosine_graph->setPen(QPen(Qt::red, 1.5));
  cosine_graph->setLineStyle(QCPGraph::lsStepLeft);

  // Beat — line with dot markers (mirrors LineStyle::Line + PointStyle::Dot)
  QCPGraph* const beat_graph = plot.addGraph();
  beat_graph->setName("Beat");
  beat_graph->setPen(QPen(Qt::darkGreen, 1.5));
  beat_graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));

  // Data generation at 1 kHz (mirrors the jthread in example.cpp)
  QTimer data_timer;
  QObject::connect(&data_timer, &QTimer::timeout, [&]() {
    const double tn = std::chrono::duration<double>(clock::now() - t0).count();
    using std::numbers::pi;
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
    sine_graph->addData(tn, std::sin(2.0 * pi * 0.5 * tn));
    cosine_graph->addData(tn, std::cos(2.0 * pi * 0.3 * tn) * 0.7);
    beat_graph->addData(tn, (std::sin(2.0 * pi * 1.0 * tn) + std::sin(2.0 * pi * 1.1 * tn)) * 0.4);
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
  });
  data_timer.start(DATA_INTERVAL_MS);

  // Render at ~60 fps (mirrors the plot.render() loop in example.cpp)
  QTimer render_timer;
  QObject::connect(&render_timer, &QTimer::timeout, [&]() {
    const double t_now = std::chrono::duration<double>(clock::now() - t0).count();
    const double t_min = t_now - X_RANGE_S;
    sine_graph->data()->removeBefore(t_min);
    cosine_graph->data()->removeBefore(t_min);
    beat_graph->data()->removeBefore(t_min);
    plot.xAxis->setRange(t_min, t_now);
    plot.replot();
  });
  render_timer.start(RENDER_INTERVAL_MS);

  plot.setOpenGl(true);
  plot.show();
  return QApplication::exec();
}
