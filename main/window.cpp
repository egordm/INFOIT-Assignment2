//
// Created by egordm on 17-3-2018.
//

#include <opencv/cv.hpp>
#include "window.h"
#include "defines.h"

#if PERFORMANCE_MEASURE
#include <c++/7.3.0/chrono>
#endif

#define STAT_HEIGHT 18
#define STAT_MARGIN 8

Window::Window(const std::string &window_name, InputHelper *source) : window_name(window_name), source(source), debug(true) {
    cv::namedWindow(window_name, CV_WINDOW_NORMAL);//CV_WINDOW_KEEPRATIO
    cv::setMouseCallback(window_name, &on_mouse, this);
}

Window::Window(const std::string &window_name, InputHelper *source, const std::vector<WindowHelper *> &helpers)
        : window_name(window_name), helpers(helpers), source(source), debug(true) {
    cv::namedWindow(window_name, CV_WINDOW_NORMAL);//CV_WINDOW_KEEPRATIO
    cv::setMouseCallback(window_name, &on_mouse, this);
}

void Window::show() {
    cv::Mat res = draw();

    if (statistics.empty() || !debug) {
        cv::imshow(window_name, res);
        return;
    }
    auto stats_bar_height = stat_panel_height();
    cv::Scalar value(255, 255, 255);
    cv::copyMakeBorder(res, res, stats_bar_height, 0, 0, 0, CV_HAL_BORDER_CONSTANT, value);
    for (int i = 0; i < statistics.size(); ++i) {
        cv::putText(res, statistics[i](), cv::Point(10, STAT_HEIGHT * (i + 1)), CV_FONT_NORMAL, 0.46,
                    cv::Scalar::all(0));
    }

    cv::imshow(window_name, res);
}

void Window::on_mouse(int event, int x, int y, int, void *object) {
    if (event == CV_EVENT_LBUTTONDOWN || event == CV_EVENT_RBUTTONDOWN) {
        auto win = ((Window *) object);
        win->on_click(x, y - win->stat_panel_height(), event == CV_EVENT_RBUTTONDOWN);
    }
}

int Window::stat_panel_height() {
    return static_cast<int>(statistics.size() * STAT_HEIGHT + STAT_MARGIN);
}

void Window::add_statistic(const Statistic &stat) {
    statistics.push_back(stat);
}

void Window::init() {
    for (auto helper : helpers) {
        if (debug) {
            auto helper_stats = helper->get_statistics();
            statistics.insert(statistics.end(), helper_stats.begin(), helper_stats.end());

            for (auto trackbar : helper->get_trackbars()) {
                cv::createTrackbar(std::get<0>(trackbar), window_name, std::get<1>(trackbar), std::get<2>(trackbar),
                                   &Window::on_trackbar, this);
            }
        }
    }
}

Window::~Window() {
    for (int i = (int) helpers.size() - 1; i >= 0; --i) delete helpers[i];
}

void Window::on_trackbar(int newValue, void *object) {
    ((Window *) object)->show();
}

void Window::on_click(int x, int y, bool rb) {
    bool redraw = false;
    for (auto helper : helpers) {
        if (!redraw) redraw = helper->on_click(x, y, rb);
        else helper->on_click(x, y, rb);
    }
    if (redraw) show();
}

cv::Mat Window::draw() {
    return draw_on(source->frame);
}

cv::Mat Window::draw_on(const cv::Mat &src) {
#if PERFORMANCE_MEASURE
    std::cout << "Start performance" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
#endif
    cv::Mat ret;
    src.copyTo(ret);

    for (auto helper : helpers) {
#if PERFORMANCE_MEASURE
        auto start_helper = std::chrono::high_resolution_clock::now();
#endif
        ret = helper->draw(ret, src);
#if PERFORMANCE_MEASURE
        auto finish_helper = std::chrono::high_resolution_clock::now();
        std::cout << "\tDraw performance of " << (std::string)typeid(*helper).name() << ": "
                  << std::chrono::duration_cast<std::chrono::nanoseconds>(finish_helper-start_helper).count()/1000000.0 << "ms\n";
#endif
    }

#if PERFORMANCE_MEASURE
    auto finish = std::chrono::high_resolution_clock::now();
    std::cout << "Total Draw performance " << std::chrono::duration_cast<std::chrono::nanoseconds>(finish-start).count()/1000000.0 << "ms\n";
#endif
    return ret;
}
