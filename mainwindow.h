#pragma once

#include <QMainWindow>

#include <clcontextwrapper.h>
#include <memory>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

    ~MainWindow();

private:
    void _testOpenCL();
    std::shared_ptr<CLContextWrapper> clContext;
};
