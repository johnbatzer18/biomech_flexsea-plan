#ifndef CUSTOMCHARTVIEW_H
#define CUSTOMCHARTVIEW_H

#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QWidget>
#include <chartdata.h>

#define NUM_POINTS_PER_WIDTH 50

class CustomChartView : public QWidget
{
Q_OBJECT

signals:
    //signal emitted when track data is updated
    void plotTracked(int*,int*);

public:
    explicit CustomChartView(QWidget *parent = 0);
	void addDataProvider(DataProvider* provider);
	bool shouldDrawLegend = true;
	int legendRightOffset = 10;
	int legendTopOffset = 10;

public slots:
    //update chart data by adding new data
    void updateChartData(void);

private:
	ChartData *chartData;
    int pickedCursor;
    double cursorPositionRate1,cursorPositionRate2;

    void initialize(void);

    //set background and draw axis
    void drawAxis(QPainter&);

	void drawSingleFromDataStream(QPainter& painter, const int &streamIndex, const int &offset);
	void drawDoubleFromDataStream(QPainter& painter, const int &streamIndex, const int &offset1, const int &offset2);
	void drawDataProviderSeries(QPainter&, int streamIndex, int numFields);

    //draw cursor(Y Axis)
    void drawCursor(QPainter&);
	void drawLegend(QPainter&);

protected:
    void mouseMoveEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void paintEvent(QPaintEvent*);

public slots:
};

#endif // CUSTOMCHARTVIEW_H
