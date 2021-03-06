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

#include "PHY_I_modulator.h"
#include "TxVLC.h"
#include "vlc_reed_solomon.h"
#include "bbRSEnc.h"
#include "vlc_convolutional_coding.h"
#include "bbCCEnc.h"
#include "bbManchesterEnc.h"
#include "bb4b6bEnc.h"
#include "InterPunct.h"
//#include "bbVLCInterleaver.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/kludge_copy.h>

PHY_I_modulator::PHY_I_modulator(unsigned int _phy_type, unsigned int _phy_modulation, unsigned int _rs_in, unsigned int _rs_out, unsigned int _gf, unsigned int _cc_in, unsigned int _cc_out, unsigned int _raw_length, unsigned int _data_rate) :
	gr::hier_block2("PHY_I_modulator", gr::io_signature::make(1,1,sizeof(int)), gr::io_signature::make(1,1,sizeof(int))),
	phy_type(_phy_type), phy_modulation(_phy_modulation), rs_in(_rs_in), rs_out(_rs_out), GF(_gf), cc_in(_cc_in), cc_out(_cc_out), raw_length(_raw_length), data_rate(_data_rate)
{
	poly[0]=0133; poly[1]=0171;	poly[2]=0165;
	if (rs_in !=0)
	{
		bbRSEnc::sptr rs_enc = bbRSEnc::Create(&GF, &rs_out, &rs_in, &phy_type, &raw_length);
		InterPunct::sptr intlv = InterPunct::Create(GF, rs_out, rs_in, raw_length, rs_enc->out_rs, InterPunct::INTERPUNCT);
		//bbVLCInterleaver::sptr intlv = bbVLCInterleaver::Create(GF, rs_out, rs_in, raw_length, rs_enc->out_rs);
		connect(self(), 0, rs_enc, 0);
		connect(rs_enc, 0, intlv,0);
		if (cc_in!=0)
		{
			bbCCEnc::sptr cc_enc = bbCCEnc::Create(3, 7, poly, intlv->out, data_rate);
			connect(intlv, 0, cc_enc, 0);
			bbManchesterEnc::sptr RLL = bbManchesterEnc::Create(bbManchesterEnc::OOK);
			out_PHY_I_mod = cc_enc->out_cc*2;
			connect(cc_enc,0,RLL,0);
			connect(RLL,0,self(), 0);				
		}
		else
		{
			if (phy_modulation == 0)
			{
				bbManchesterEnc::sptr RLL = bbManchesterEnc::Create(bbManchesterEnc::OOK);
				out_PHY_I_mod = intlv->out*2;
				connect(intlv,0,RLL,0);
				connect(RLL,0,self(), 0);
			}
			else
			{
				bb4b6bEnc::sptr RLL = bb4b6bEnc::Create();
				bbManchesterEnc::sptr RLL2 = bbManchesterEnc::Create(bbManchesterEnc::VPPM);
				out_PHY_I_mod= (intlv->out/4*6)*2;
				connect(intlv,0,RLL,0);
				connect(RLL,0, RLL2,0);
				connect(RLL2,0,self(), 0);
			}			
		}
	}
	else //no reed solomon
	{
		if (phy_modulation == 0)
			{
				bbManchesterEnc::sptr RLL = bbManchesterEnc::Create(bbManchesterEnc::OOK);
				out_PHY_I_mod= raw_length*2;
				connect(self(), 0, RLL, 0);
				connect(RLL,0,self(), 0);
			}
			else
			{
				bb4b6bEnc::sptr RLL = bb4b6bEnc::Create();
				bbManchesterEnc::sptr RLL2 = bbManchesterEnc::Create(bbManchesterEnc::VPPM);
				out_PHY_I_mod= (raw_length/4*6)*2;
				connect(self(),0,RLL,0);
				connect(RLL,0, RLL2,0);
				connect(RLL2,0,self(),0);
			}	
	}
}

PHY_I_modulator::sptr PHY_I_modulator::Create(unsigned int _phy_type, unsigned int _phy_modulation, unsigned int _rs_in, unsigned int _rs_out, unsigned int _gf, unsigned int _cc_in, unsigned int _cc_out, unsigned int _raw_length, unsigned int _data_rate)
{ 
	return gnuradio::get_initial_sptr(new PHY_I_modulator(_phy_type, _phy_modulation, _rs_in, _rs_out, _gf, _cc_in, _cc_out, _raw_length, _data_rate));
}
