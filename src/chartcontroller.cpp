#include "chartcontroller.h"
#include <QTimer>

ChartController::ChartController(QMainWindow* parent)
{
	parentWindow = parent;
	chartView = nullptr;
	updateTimer = nullptr;
}

void ChartController::createChartView(QMdiArea* area)
{
	if(chartView) return;

	qDebug("Creating chart view...");

	chartView = new CustomChartView();
	chartView->setMinimumSize(640, 480);
	if(dataProviders.size() > 3)
	{
		chartView->addDataProvider(dataProviders.at(3));
		chartView->addDataProvider(dataProviders.at(2));
	}
	area->addSubWindow(chartView);

	connect(chartView, &CustomChartView::destroyed, this, &ChartController::cleanUpView);
	chartView->show();

	updateTimer = new QTimer();
	updateTimer->setSingleShot(false);
	updateTimer->setInterval(100);

	connect(updateTimer, &QTimer::timeout, this, &ChartController::updateChart);
	updateTimer->start();
}

void ChartController::updateChart(void)
{
	if(chartView && dataProviders.size() > 0)
	{
		//chartView->chartData->addPlot();
		//chartView->chartData->setTertiary(data[0]);
		chartView->repaint();
	}
}

void ChartController::cleanUpView()
{
	if(chartView)
	{
		qDebug("Deleting chart view pointer...");
		//don't 'delete' it kuz it should already be deallocated
		chartView = nullptr;
	}
	if(updateTimer)
	{
		qDebug("Deleting chart timer pointer...");
		updateTimer->stop();
		delete updateTimer;
		updateTimer = nullptr;
	}
}

void ChartController::addDataProvider(DataProvider* provider)
{
	dataProviders.append(provider);
}
