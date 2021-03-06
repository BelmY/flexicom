/* 
 * This file is part of the FlexiCom distribution (https://github.com/polhenarejos/flexicom).
 * Copyright (c) 2012-2020 Pol Henarejos, at Centre Tecnologic de Telecomunicacions de Catalunya (CTTC).
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/* $Id$ */
/* $Format:%ci$ ($Format:%h$) by $Format:%an%$ */

#include "Layout80211b.h"
#include <gnuradio/uhd/usrp_source.h>
#include "Rx80211b.h"
#include "MainWindow.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include <iostream>

const char *Layout80211b::name = "802.11b";
static int _d_ = RegisterLayout(Layout80211b::Create);

int channels [] = { 
	2412, 2417, 2422, 2427, 2432, 2437, 2442, 
	2447, 2452, 2457, 2462, 2467, 2472, 2484
};

Layout80211b::Layout80211b(MainWindow *_mw, int _radioID) :
	LayoutFactory(_mw, _radioID)
{
	QObject::connect(mw->panel->rb_layout[radioID]->bt, SIGNAL(toggled(bool)), this, SLOT(RadioPressed(bool)));
}
const char *Layout80211b::Name()
{
	return name;
}
LayoutFactory::sptr Layout80211b::Create(MainWindow *_mw, int _radioID)
{
	return LayoutFactory::sptr(new Layout80211b(_mw, _radioID));
}
void Layout80211b::Run()
{
	grTop = gr::make_top_block(std::string(name));
	QString addr = QString("addr0=%1").arg(mw->panel->le_ip[0]->text().remove(' '));
	for (int i = 1; i < mw->panel->sp_devs->value(); i++)
		addr.append(",addr%1=%2").arg(i).arg(mw->panel->le_ip[i]->text().remove(' '));
	if (mw->panel->rb_chain[RB_RX]->isChecked())
	{
		usrp = gr::uhd::usrp_source::make(addr.toStdString(), uhd::stream_args_t("fc32","sc8"));
		usrp->set_samp_rate(10e6);
		usrp->set_center_freq(channels[cb_chans->currentIndex()]*1e6);
		usrp->set_gain(mw->panel->sp_gain->value());
		rx = Rx80211b::Create(this);
		grTop->connect(usrp, 0, rx, 0);
		grTop->start();
	}
}
void Layout80211b::Stop()
{
	rx->stop();
	grTop->stop();
	grTop->wait();
	grTop.reset();
}
void Layout80211b::RadioPressed(bool check)
{
	if (check)
	{
		mw->AddCustomTab(CreateTabOpts(), tr("Options"));
		mw->panel->rb_chain[RB_TX]->setHidden(true);
		mw->panel->rb_chain[RB_RX]->setHidden(false);
		mw->panel->rb_chain[RB_RX]->setChecked(true);
		ReadSettings(mw->s);
		QObject::connect(mw, SIGNAL(SaveSettings(QSettings *)), this, SLOT(SaveSettings(QSettings *)));
		DrawPlots();
	}
	else
	{
		//SaveSettings(mw->s);
		mw->RemoveCustomTabs();
		QObject::disconnect(mw, SIGNAL(SaveSettings(QSettings *)), this, SLOT(SaveSettings(QSettings *)));
		RemovePlots();
	}
}
QWidget *Layout80211b::CreateTabOpts()
{
	QWidget *p = new QWidget;
	QGroupBox *gBox = new QGroupBox(tr("Channels"));
	QGridLayout *vBox = new QGridLayout;
	cb_chans = new QComboBox(p);
	for (uint i = 0; i < sizeof(channels)/sizeof(int); i++)
		cb_chans->addItem(QString("Channel %1: %2 MHz").arg(i+1).arg(channels[i]));
	vBox->addWidget(cb_chans);
	gBox->setLayout(vBox);
	QGridLayout *grid = new QGridLayout(p);
	grid->addWidget(gBox, 0, 0);
	return p;
}
void Layout80211b::SaveSettings(QSettings *s)
{
	s->setValue("80211b/chan", cb_chans->currentIndex());
}
void Layout80211b::ReadSettings(QSettings *s)
{
	cb_chans->setCurrentIndex(s->value("80211b/chan").toInt());
}
void Layout80211b::DrawPlots()
{
	pl_osc = new QwtPlot;
	//mw->AddCustomPlot(pl_osc, 0, 0);
}
void Layout80211b::RemovePlots()
{
	//mw->RemoveCustomPlots();
}
