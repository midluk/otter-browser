/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2013 - 2016 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
**************************************************************************/

#include "ProgressBarWidget.h"
#include "../../../core/Utils.h"
#include "../../../ui/WebWidget.h"

#include <QtGui/QPalette>
#include <QtWidgets/QHBoxLayout>

namespace Otter
{

ProgressBarWidget::ProgressBarWidget(WebWidget *webWidget, QWidget *parent) : QFrame(parent),
	m_webWidget(webWidget),
	m_progressBar(new QProgressBar(this)),
	m_elementsLabel(new QLabel(this)),
	m_totalLabel(new QLabel(this)),
	m_speedLabel(new QLabel(this)),
	m_elapsedLabel(new QLabel(this)),
	m_messageLabel(new QLabel(this)),
	m_time(NULL),
	m_loadingState(WindowsManager::FinishedLoadingState),
	m_geometryUpdateTimer(0)
{
	QHBoxLayout *layout(new QHBoxLayout(this));
	layout->addWidget(m_progressBar);
	layout->addWidget(m_elementsLabel);
	layout->addWidget(m_totalLabel);
	layout->addWidget(m_speedLabel);
	layout->addWidget(m_elapsedLabel);
	layout->addWidget(m_messageLabel);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSizeConstraint(QLayout::SetMinAndMaxSize);

	QPalette palette(m_elementsLabel->palette());
	palette.setColor(QPalette::Background, palette.color(QPalette::Base));

	setPalette(palette);
	setFrameStyle(QFrame::StyledPanel);
	setLineWidth(1);

	palette.setColor(QPalette::Background, palette.color(QPalette::AlternateBase));

	m_progressBar->setFixedWidth(150);
	m_progressBar->setFormat(tr("Document: %p%"));
	m_progressBar->setAlignment(Qt::AlignHCenter);
	m_elementsLabel->setFixedWidth(150);
	m_totalLabel->setFixedWidth(150);
	m_totalLabel->setAutoFillBackground(true);
	m_totalLabel->setPalette(palette);
	m_speedLabel->setFixedWidth(150);
	m_elapsedLabel->setFixedWidth(150);
	m_elapsedLabel->setAutoFillBackground(true);
	m_elapsedLabel->setPalette(palette);

	setAutoFillBackground(true);
	scheduleGeometryUpdate();
	updateLoadingState(webWidget->getLoadingState());
	hide();

	connect(webWidget, SIGNAL(loadMessageChanged(QString)), m_messageLabel, SLOT(setText(QString)));
	connect(webWidget, SIGNAL(loadProgress(int)), m_progressBar, SLOT(setValue(int)));
	connect(webWidget, SIGNAL(loadStatusChanged(int,int,qint64,qint64,qint64)), this, SLOT(updateLoadStatus(int,int,qint64,qint64,qint64)));
	connect(webWidget, SIGNAL(loadingStateChanged(WindowsManager::LoadingState)), this, SLOT(updateLoadingState(WindowsManager::LoadingState)));
}

void ProgressBarWidget::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == m_geometryUpdateTimer)
	{
		killTimer(m_geometryUpdateTimer);

		m_geometryUpdateTimer = 0;

		QRect geometry(m_webWidget->getProgressBarGeometry());

		if (m_webWidget->getLoadingState() == WindowsManager::OngoingLoadingState)
		{
			if (!isVisible())
			{
				connect(m_webWidget, SIGNAL(progressBarGeometryChanged()), this, SLOT(scheduleGeometryUpdate()));
			}

			geometry.translate(0, m_webWidget->pos().y());

			setGeometry(geometry);
			show();
			raise();
		}
		else
		{
			disconnect(m_webWidget, SIGNAL(progressBarGeometryChanged()), this, SLOT(scheduleGeometryUpdate()));

			hide();
		}
	}
	else
	{
		if (m_time)
		{
			int seconds(m_time->elapsed() / 1000);
			int minutes(seconds / 60);

			seconds = (seconds - (minutes * 60));

			m_elapsedLabel->setText(tr("Time: %1").arg(QStringLiteral("%1:%2").arg(minutes).arg(seconds, 2, 10, QLatin1Char('0'))));
		}
		else
		{
			m_elapsedLabel->setText(QString());
		}

		if (m_webWidget->getLoadingState() != WindowsManager::OngoingLoadingState)
		{
			killTimer(event->timerId());
		}
	}
}

void ProgressBarWidget::updateLoadingState(WindowsManager::LoadingState state)
{
	if (state == m_loadingState)
	{
		return;
	}

	if (state == WindowsManager::OngoingLoadingState)
	{
		m_progressBar->setValue(0);
		m_elapsedLabel->setText(tr("Time: %1").arg(QLatin1String("0:00")));

		updateLoadStatus(0, 0, 0, 0, 0);

		m_time = new QTime();
		m_time->start();

		startTimer(1000);

		if (!isVisible())
		{
			scheduleGeometryUpdate();
		}
	}
	else
	{
		if (m_time)
		{
			delete m_time;

			m_time = NULL;
		}

		disconnect(m_webWidget, SIGNAL(progressBarGeometryChanged()), this, SLOT(scheduleGeometryUpdate()));

		hide();
	}

	m_loadingState = state;
}

void ProgressBarWidget::scheduleGeometryUpdate()
{
	if (m_geometryUpdateTimer == 0)
	{
		m_geometryUpdateTimer = startTimer(50);
	}
}

void ProgressBarWidget::updateLoadStatus(int finishedRequests, int startedReuests, qint64 bytesReceived, qint64 bytesTotal, qint64 speed)
{
	Q_UNUSED(bytesTotal)

	m_elementsLabel->setText(tr("Elements: %1/%2").arg(finishedRequests).arg(startedReuests));
	m_totalLabel->setText(tr("Total: %1").arg(Utils::formatUnit(bytesReceived, false, 1)));
	m_speedLabel->setText(tr("Speed: %1").arg(Utils::formatUnit(speed, true, 1)));
}

}
