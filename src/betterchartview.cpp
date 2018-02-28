/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'plan-gui' Graphical User Interface
	Copyright (C) 2016 Dephy, Inc. <http://dephy.com/>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************
	[Lead developper] Jean-Francois (JF) Duval, jfduval at dephy dot com.
	[Origin] Based on Jean-Francois Duval's work at the MIT Media Lab
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors]
*****************************************************************************
	[This file] betterchartview: An improve chart view with more functionnality
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2018-02-25 | sbelanger | Initial GPL-3.0 release
	*
****************************************************************************/

#include "betterchartview.h"
#include <QtGui/QMouseEvent>
#include <QDebug>

BetterChartView::BetterChartView(QChart *chart, QWidget *parent) :
	QChartView(chart, parent),
	m_isTouching(false)
{
	setRubberBand(QChartView::RectangleRubberBand);
}

bool BetterChartView::viewportEvent(QEvent *event)
{
	if (event->type() == QEvent::TouchBegin) {
		// By default touch events are converted to mouse events. So
		// after this event we will get a mouse event also but we want
		// to handle touch events as gestures only. So we need this safeguard
		// to block mouse events that are actually generated from touch.
		m_isTouching = true;

		// Turn off animations when handling gestures they
		// will only slow us down.
		chart()->setAnimationOptions(QChart::NoAnimation);
	}
	return QChartView::viewportEvent(event);
}

void BetterChartView::mousePressEvent(QMouseEvent *event)
{
	if((event->buttons() & Qt::MiddleButton) != 0)
	{
		chart()->zoomReset();
	}

	if (m_isTouching)
		return;
	QChartView::mousePressEvent(event);
}

void BetterChartView::mouseDoubleClickEvent(QMouseEvent *event )
{
	if ( event->button() == Qt::LeftButton )
	{
		chart()->zoomReset();
	}
	QChartView::mouseDoubleClickEvent(event);
}

void BetterChartView::mouseMoveEvent(QMouseEvent *event)
{
	if (m_isTouching)
		return;
	QChartView::mouseMoveEvent(event);
}

void BetterChartView::mouseReleaseEvent(QMouseEvent *event)
{
	if (m_isTouching)
		m_isTouching = false;

	QChartView::mouseReleaseEvent(event);
}

void BetterChartView::wheelEvent(QWheelEvent* event)
{
	if(event->angleDelta().y() > 0)
	{
		chart()->zoomIn();
	}
	else if(event->angleDelta().y() < 0)
	{
		chart()->zoomOut();
	}

}

void BetterChartView::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Plus:
		chart()->zoomIn();
		break;
	case Qt::Key_Minus:
		chart()->zoomOut();
		break;
	case Qt::Key_Left:
		chart()->scroll(-10, 0);
		break;
	case Qt::Key_Right:
		chart()->scroll(10, 0);
		break;
	case Qt::Key_Up:
		chart()->scroll(0, 10);
		break;
	case Qt::Key_Down:
		chart()->scroll(0, -10);
		break;
	default:
		QGraphicsView::keyPressEvent(event);
		break;
	}

}
