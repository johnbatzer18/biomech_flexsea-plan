#ifndef W_GLPLOT_H
#define W_GLPLOT_H

#include <customchartview.h>
#include <QLabel>
#include <QWidget>

namespace Ui
{
    class W_GLPlot;
}

class W_GLPlot : public QWidget
{
Q_OBJECT

public:
    explicit W_GLPlot(QWidget *parent = 0);
    ~W_GLPlot();

    //refresh plot data
    void refreshGLPlot(void);

private:
    Ui::W_GLPlot *ui;
    int timerId;
    QLabel *trackLabel1,*trackLabel2;
    CustomChartView *chartView;

    //initialize all data
    void initialize(void);

protected:
    void resizeEvent(QResizeEvent*);
    void timerEvent(QTimerEvent*);

public slots:
    //show track data to QLabel after accept track updating signal
    void updateTrack(int*,int*);
};

#endif // W_GLPLOT_H
