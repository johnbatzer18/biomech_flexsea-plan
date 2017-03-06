#include "w_glplot.h"
#include "ui_w_glplot.h"

W_GLPlot::W_GLPlot(QWidget *parent) : QWidget(parent), ui(new Ui::W_GLPlot)
{
    ui->setupUi(this);
    initialize();
}

W_GLPlot::~W_GLPlot()
{
    delete ui;
}

void W_GLPlot::refreshGLPlot(void)
{
    chartView->updateChartData();
}

void W_GLPlot::initialize(void)
{
    trackLabel1=new QLabel(this);
    trackLabel1->setFont(QFont("Courier",16));
    trackLabel1->setFrameStyle(QFrame::Box);
    trackLabel1->setGeometry(10,20,100,190);

    trackLabel2=new QLabel(this);
    trackLabel2->setFont(QFont("Courier",16));
    trackLabel2->setFrameStyle(QFrame::Box);
    trackLabel2->setGeometry(10,220,100,190);

    chartView=new CustomChartView(this);
    chartView->updateChartData();
    chartView->setGeometry(120,20,400,300);
    connect(chartView,SIGNAL(plotTracked(int*,int*)),this,SLOT(updateTrack(int*,int*)));

    timerId=startTimer(200);
}

void W_GLPlot::resizeEvent(QResizeEvent*)
{
    int margin=0;
    QRect chartViewGeometry=chartView->geometry();
    chartView->setGeometry(chartViewGeometry.x(),chartViewGeometry.y(),width()-chartViewGeometry.x()-margin,height()-chartViewGeometry.y()-margin);
}

void W_GLPlot::timerEvent(QTimerEvent *e)
{
    if(e->timerId()!=timerId)return;
    chartView->updateChartData();
}

void W_GLPlot::updateTrack(int *track1, int *track2)
{
    QString trackString1,trackString2;
    for(int i=0;i<PLOT_SERIES;i++){
        trackString1+=(i?"\n":"")+QString::number(track1[i]);
        trackString2+=(i?"\n":"")+QString::number(track2[i]);
    }
    trackLabel1->setText(trackString1);
    trackLabel2->setText(trackString2);
}
