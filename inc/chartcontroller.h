#ifndef CHARTCONTROLLER_H
#define CHARTCONTROLLER_H

#include <QObject>
#include <QMainWindow>
#include <QMdiArea>
#include <QList>
#include <customchartview.h>
#include <dataprovider.h>
#include <QTimer>

class ChartController: public QObject
{
	Q_OBJECT

public:
	ChartController(){}
	ChartController(QMainWindow*);
	void addDataProvider(DataProvider*);

public slots:
	void createChartView(QMdiArea* area);
	void cleanUpView();

private:
	QMainWindow* parentWindow;
	CustomChartView* chartView;
	QList<DataProvider*> dataProviders;
	QTimer* updateTimer;

private slots:
	void updateChart(void);

};

#endif // CHARTCONTROLLER_H
