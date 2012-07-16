#include "compat.h"
#include "TxVLC.h"
#include <gr_io_signature.h>
#include "LayoutVLC.h"
#include <vector>
#include <QtGlobal>
#include <iostream>

#include <gr_int_to_float.h>
#include "bbPHR_generation.h"
#include "bbPSDU_generation.h"
#include "bbRSEnc.h"
#include "bbVLCInterleaver.h"
#include "bbCCEnc.h"
#include "bbManchesterEnc.h"
#include "bb4b6bEnc.h"
#include "PHY_I_modulator.h"
#include "bbVLC_info_assembler.h"
#include "bbVLC_Frame_Generator.h"

#include "bbMatlab.h"


TxVLC::TxVLC(LayoutVLC * _ly) :
	gr_hier_block2("TxVLC", gr_make_io_signature(0, 0, 0), gr_make_io_signature(1, 1, sizeof(float))),
	ly(_ly)
{
	//variable initialization, to make easier the pass of parameters. Prepared for PHY I 
	init_var();
	int phr_words;
	int psdu_words;
	int out_PHY_I_phr;
	int out_PHY_I_psdu;
	
	/*gr_null_source_sptr null = gr_make_null_source(sizeof(int));
	bbManchesterEnc::sptr phr_RLL = bbManchesterEnc::Create(0);
	gr_int_to_float_sptr i2f = gr_make_int_to_float(1, 1.0);
	connect(null,0,phr_RLL,0);
	connect(phr_RLL,0,i2f,0);
	connect(i2f,0,self(),0);*/
	gr_int_to_float_sptr i2f = gr_make_int_to_float(1, 1.0);
	
	//GENERATION OF PHR, DATA
	bbPHR_generation::sptr PHR_gen = bbPHR_generation::Create(vlc_var.tx_mode, vlc_var.PSDU_raw_length/8, vlc_var.PHR_raw_length, vlc_var.MCSID);	
	bbPSDU_generation::sptr PSDU_gen = bbPSDU_generation::Create("src/layouts/VLC/input_data.txt", vlc_var.PSDU_raw_length/8);
	poly = new int[3];
	poly[0]=0133; poly[1]=0171;	poly[2]=0165;
	
	//PHR CHAIN
	bbRSEnc::sptr phr_rs_encoder = bbRSEnc::Create(vlc_var.GF, vlc_var._rs_code.pre_rs_out , vlc_var._rs_code.pre_rs_in ,vlc_var.phy_type , vlc_var.PHR_raw_length);
	switch(vlc_var.phy_type)
	{
		case 0:
			phr_words = (int)  ceil(((double)vlc_var.PHR_raw_length/(vlc_var.GF*vlc_var._rs_code.pre_rs_in)))*vlc_var._rs_code.pre_rs_out;
		break;
		case 1:
			phr_words = phr_rs_encoder->out_rs / vlc_var.GF;
		break;
	}
	bbVLCInterleaver::sptr phr_interleaver = bbVLCInterleaver::Create(vlc_var.GF, vlc_var._rs_code.pre_rs_out , vlc_var._rs_code.pre_rs_in , vlc_var.PHR_raw_length, phr_words);
	bbCCEnc::sptr phr_cc_encoder = bbCCEnc::Create(3, 7, poly, phr_interleaver->out_int, 0);
	bbManchesterEnc::sptr phr_RLL = bbManchesterEnc::Create(0,phr_cc_encoder->out_cc); //always on/off keying
	out_PHY_I_phr= phr_cc_encoder->out_cc*2;
		
	//PHR connections
	connect(PHR_gen,0,phr_rs_encoder,0);
	connect(phr_rs_encoder,0,phr_interleaver,0);
	connect(phr_interleaver,0,phr_cc_encoder,0);
	connect(phr_cc_encoder, 0, phr_RLL,0);
	//connect(phr_RLL, 0, i2f,0);
	//connect(i2f,0,self(),0);
	
	//PSDU CHAIN
	if (vlc_var._rs_code.rs_in!=0)
	{
		bbRSEnc::sptr psdu_rs_encoder = bbRSEnc::Create(vlc_var.GF, vlc_var._rs_code.rs_out , vlc_var._rs_code.rs_in ,vlc_var.phy_type,vlc_var.PSDU_raw_length);
		connect(PSDU_gen, 0, psdu_rs_encoder, 0);
		switch (vlc_var.phy_type)
		{
			case 0:
				psdu_words = (int) ceil(((double)vlc_var.PSDU_raw_length/(vlc_var.GF*vlc_var._rs_code.rs_in)))*vlc_var._rs_code.rs_out;
			break;
			case 1:
				psdu_words = psdu_rs_encoder->out_rs / vlc_var.GF;
			break;
		}
		bbVLCInterleaver::sptr psdu_interleaver = bbVLCInterleaver::Create(vlc_var.GF, vlc_var._rs_code.rs_out , vlc_var._rs_code.rs_in , vlc_var.PSDU_raw_length, psdu_words);		
		connect(psdu_rs_encoder, 0, psdu_interleaver, 0);
		if(vlc_var._cc_code.cc_in !=0) 
		{
			bbCCEnc::sptr psdu_cc_encoder = bbCCEnc::Create(3, 7, poly, psdu_interleaver->out_int, vlc_var.operating_mode);
			connect(psdu_interleaver, 0, psdu_cc_encoder, 0);
			if(vlc_var.mod_type ==0)
			{
				bbManchesterEnc::sptr psdu_RLL = bbManchesterEnc::Create(0,psdu_cc_encoder->out_cc);
				out_PHY_I_psdu= psdu_cc_encoder->out_cc*2;
				bbVLC_info_assembler::sptr INFO_ass = bbVLC_info_assembler::Create(3,vlc_var.psdu_units, out_PHY_I_phr,out_PHY_I_psdu);
				bbVLC_Frame_Generator::sptr FRAME_gen = bbVLC_Frame_Generator::Create(vlc_var.flp_length,vlc_var.tx_mode, vlc_var.psdu_units,out_PHY_I_phr,out_PHY_I_psdu,3);
				connect(psdu_cc_encoder,0,psdu_RLL,0);
				connect(phr_RLL,0,INFO_ass,0);
				connect(psdu_RLL,0,INFO_ass,1);
				connect(INFO_ass,0, FRAME_gen,0);
				connect(FRAME_gen,0,i2f,0);
				connect(i2f,0,self(),0);
			}
			else
			{
				bb4b6bEnc::sptr psdu_RLL = bb4b6bEnc::Create();
				out_PHY_I_psdu= psdu_cc_encoder->out_cc/4*6;
				bbVLC_info_assembler::sptr INFO_ass = bbVLC_info_assembler::Create(3,vlc_var.psdu_units, out_PHY_I_phr,out_PHY_I_psdu);
				bbVLC_Frame_Generator::sptr FRAME_gen = bbVLC_Frame_Generator::Create(vlc_var.flp_length,vlc_var.tx_mode, vlc_var.psdu_units,out_PHY_I_phr,out_PHY_I_psdu,3);
				connect(psdu_cc_encoder,0,psdu_RLL,0);
				connect(phr_RLL,0,INFO_ass,0);
				connect(psdu_RLL,0,INFO_ass,1);
				connect(INFO_ass,0, FRAME_gen,0);
				connect(FRAME_gen,0,i2f,0);
				connect(i2f,0,self(),0);
			}
		}
		else
		{
			if(vlc_var.mod_type ==0)
			{
				bbManchesterEnc::sptr psdu_RLL = bbManchesterEnc::Create(0,psdu_interleaver->out_int);
				out_PHY_I_psdu= psdu_interleaver->out_int*2;
				bbVLC_info_assembler::sptr INFO_ass = bbVLC_info_assembler::Create(3,vlc_var.psdu_units, out_PHY_I_phr,out_PHY_I_psdu);
				bbVLC_Frame_Generator::sptr FRAME_gen = bbVLC_Frame_Generator::Create(vlc_var.flp_length,vlc_var.tx_mode, vlc_var.psdu_units,out_PHY_I_phr,out_PHY_I_psdu,3);
				connect(psdu_interleaver,0,psdu_RLL,0);
				connect(phr_RLL,0,INFO_ass,0);
				connect(psdu_RLL,0,INFO_ass,1);
				connect(INFO_ass,0, FRAME_gen,0);
				connect(FRAME_gen,0,i2f,0);
				connect(i2f,0,self(),0);
			}
			else
			{
				bb4b6bEnc::sptr psdu_RLL = bb4b6bEnc::Create();
				out_PHY_I_psdu=psdu_interleaver->out_int/4*6;
				bbVLC_info_assembler::sptr INFO_ass = bbVLC_info_assembler::Create(3,vlc_var.psdu_units, out_PHY_I_phr,out_PHY_I_psdu);
				bbVLC_Frame_Generator::sptr FRAME_gen = bbVLC_Frame_Generator::Create(vlc_var.flp_length,vlc_var.tx_mode, vlc_var.psdu_units,out_PHY_I_phr,out_PHY_I_psdu,3);
				connect(psdu_interleaver,0,psdu_RLL,0);
				connect(phr_RLL,0,INFO_ass,0);
				connect(psdu_RLL,0,INFO_ass,1);
				connect(INFO_ass,0, FRAME_gen,0);
				connect(FRAME_gen,0,i2f,0);
				connect(i2f,0,self(),0);
			}	
		}
	}
	else
	{
		if(vlc_var.mod_type ==0)
		{
			bbManchesterEnc::sptr psdu_RLL = bbManchesterEnc::Create(0,vlc_var.PSDU_raw_length);
			out_PHY_I_psdu= vlc_var.PSDU_raw_length*2;
			bbVLC_info_assembler::sptr INFO_ass = bbVLC_info_assembler::Create(3,vlc_var.psdu_units, out_PHY_I_phr,out_PHY_I_psdu);
			bbVLC_Frame_Generator::sptr FRAME_gen = bbVLC_Frame_Generator::Create(vlc_var.flp_length,vlc_var.tx_mode, vlc_var.psdu_units,out_PHY_I_phr,out_PHY_I_psdu,3);
			connect(PSDU_gen,0,psdu_RLL,0);
			connect(phr_RLL,0,INFO_ass,0);
			connect(psdu_RLL,0,INFO_ass,1);
			connect(INFO_ass,0, FRAME_gen,0);
			connect(FRAME_gen,0,i2f,0);
			connect(i2f,0,self(),0);
		}
		else
		{
			bb4b6bEnc::sptr psdu_RLL = bb4b6bEnc::Create();
			out_PHY_I_psdu=vlc_var.PSDU_raw_length/4*6;
			bbVLC_info_assembler::sptr INFO_ass = bbVLC_info_assembler::Create(3,vlc_var.psdu_units, out_PHY_I_phr,out_PHY_I_psdu);
			bbVLC_Frame_Generator::sptr FRAME_gen = bbVLC_Frame_Generator::Create(vlc_var.flp_length,vlc_var.tx_mode, vlc_var.psdu_units,out_PHY_I_phr,out_PHY_I_psdu,3);
			connect(PSDU_gen,0,psdu_RLL,0);
			connect(phr_RLL,0,INFO_ass,0);
			connect(psdu_RLL,0,INFO_ass,1);
			connect(INFO_ass,0, FRAME_gen,0);
			connect(FRAME_gen,0,i2f,0);
			connect(i2f,0,self(),0);
		}	
	}
	
	
	if (0)
	{
		bbMatlab::sptr gm = bbMatlab::Create("m.txt", sizeof(float));
		connect(i2f, 0, gm, 0);
		//connect(uc2f,0,gm,0);
	}
	
}


TxVLC::sptr TxVLC::Create(LayoutVLC * _ly)
{
	return sptr(new TxVLC(_ly));
}

void TxVLC::init_var()
{
	vlc_var.MCSID=new int[6];
	
	vlc_var.flp_length=ly->varVLC->sp_flp_length->value();
	
	for (int i=0; i<2; i++)
	{
		if (ly->varVLC->rb_phy_type[i]->isChecked())
			vlc_var.phy_type = i; //phy_type=0 PHY I, otherwise PHY II
		if (ly->varVLC->rb_phy_modulation[i]->isChecked())
			vlc_var.mod_type = i; //mod_type=0 OOK, otherwise PHY II
	}
	if ((vlc_var.phy_type == 0) && (vlc_var.mod_type==0))
		vlc_var.PHR_raw_length = 32 + 16 + 6 + 0 * 32; 
		//32 bits (Table 81) + 16 bits CRC + 6 Tail bits. The 0 is because for the moment
		//dimming capabilities are not developed
	else
		vlc_var.PHR_raw_length = 32 + 16 + 0 * 32; //

	
	vlc_var.tx_mode = ly->varVLC->cb_tx_mode->currentIndex();
	switch (vlc_var.tx_mode)
	{
		case 0:
			vlc_var.psdu_units = ly->varVLC->sp_psdu_units[0]->value();
		break;
		default:
			vlc_var.psdu_units = ly->varVLC->sp_psdu_units[1]->value();
		break;
	}
	switch (vlc_var.phy_type)
	{
		case 0: //PHY I
			vlc_var.GF=4;
			vlc_var.PSDU_raw_length = ly->varVLC->sp_frame_size[0]->value()*8;
			switch (vlc_var.mod_type) // OOK
			{
				case 0:
					vlc_var._rs_code.pre_rs_in = 7;
					vlc_var._rs_code.pre_rs_out = 15;
					vlc_var._cc_code.pre_cc_in = 1;
					vlc_var._cc_code.pre_cc_out = 4;
					vlc_var.clock_rate = 200e3;
					vlc_var.operating_mode = ly->varVLC->cb_phy_op_mode[0]->currentIndex();
					switch (ly->varVLC->cb_phy_op_mode[0]->currentIndex())
					{
					    case 0:
							vlc_var._rs_code.rs_in = 7;
							vlc_var._rs_code.rs_out = 15;
							vlc_var._cc_code.cc_in = 1;
							vlc_var._cc_code.cc_out = 4;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=0; vlc_var.MCSID[2]=0;
							vlc_var.MCSID[3]=0; vlc_var.MCSID[4]=0; vlc_var.MCSID[5]=0;
							break;
						case 1:
							vlc_var._rs_code.rs_in = 11;
							vlc_var._rs_code.rs_out = 15;
							vlc_var._cc_code.cc_in = 1;
							vlc_var._cc_code.cc_out = 3;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=0; vlc_var.MCSID[2]=0;
							vlc_var.MCSID[3]=0; vlc_var.MCSID[4]=0; vlc_var.MCSID[5]=1;
							break;
						case 2:
							vlc_var._rs_code.rs_in = 11;
							vlc_var._rs_code.rs_out = 15;
							vlc_var._cc_code.cc_in = 2;
							vlc_var._cc_code.cc_out = 3;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=0; vlc_var.MCSID[2]=0;
							vlc_var.MCSID[3]=0; vlc_var.MCSID[4]=1; vlc_var.MCSID[5]=0;
							break;
						case 3:
							vlc_var._rs_code.rs_in = 11;
							vlc_var._rs_code.rs_out = 15;
							vlc_var._cc_code.cc_in = 0;
							vlc_var._cc_code.cc_out = 0;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=0; vlc_var.MCSID[2]=0;
							vlc_var.MCSID[3]=0; vlc_var.MCSID[4]=1; vlc_var.MCSID[5]=1;
							break;
						case 4:
							vlc_var._rs_code.rs_in = 0;
							vlc_var._rs_code.rs_out = 0;
							vlc_var._cc_code.cc_in = 0;
							vlc_var._cc_code.cc_out = 0;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=0; vlc_var.MCSID[2]=0;
							vlc_var.MCSID[3]=1; vlc_var.MCSID[4]=0; vlc_var.MCSID[5]=0;
							break;
					}
					break;
				case 1:
					vlc_var._rs_code.pre_rs_in = 2;
					vlc_var._rs_code.pre_rs_out = 15;
					vlc_var._cc_code.pre_cc_in = 0;
					vlc_var._cc_code.pre_cc_out = 0;
					vlc_var._cc_code.cc_in=0;
					vlc_var._cc_code.cc_out=0;
					vlc_var.clock_rate = 800e3; //trick to use the manchester encoder
					vlc_var.operating_mode = ly->varVLC->cb_phy_op_mode[1]->currentIndex();
					switch (ly->varVLC->cb_phy_op_mode[1]->currentIndex())
					{
						case 0:
							vlc_var._rs_code.rs_in = 2;
							vlc_var._rs_code.rs_out = 15;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=0; vlc_var.MCSID[2]=0;
							vlc_var.MCSID[3]=1; vlc_var.MCSID[4]=0; vlc_var.MCSID[5]=1;
							break;
						case 1:
							vlc_var._rs_code.rs_in = 4;
							vlc_var._rs_code.rs_out = 15;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=0; vlc_var.MCSID[2]=0;
							vlc_var.MCSID[3]=1; vlc_var.MCSID[4]=1; vlc_var.MCSID[5]=0;
							break;
						case 2:
							vlc_var._rs_code.rs_in = 7;
							vlc_var._rs_code.rs_out = 15;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=0; vlc_var.MCSID[2]=0;
							vlc_var.MCSID[3]=1; vlc_var.MCSID[4]=1; vlc_var.MCSID[5]=1;
							break;
						case 3:
							vlc_var._rs_code.rs_in = 0;
							vlc_var._rs_code.rs_out = 0;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=0; vlc_var.MCSID[2]=1;
							vlc_var.MCSID[3]=0; vlc_var.MCSID[4]=0; vlc_var.MCSID[5]=0;
							break;
					}
					break;
			}
			break;
		case 1: //PHY II
			vlc_var.GF=8;
			vlc_var._cc_code.pre_cc_in = 0;
			vlc_var._cc_code.pre_cc_out = 0;
			vlc_var._cc_code.cc_in = 0;
			vlc_var._cc_code.cc_out = 0;
			vlc_var._rs_code.pre_rs_in = 32;
			vlc_var._rs_code.pre_rs_out = 64;
			vlc_var.PSDU_raw_length = ly->varVLC->sp_frame_size[1]->value()*8;
			switch (vlc_var.mod_type)
			{
				case 0: //OOK
					vlc_var.operating_mode = ly->varVLC->cb_phy_op_mode[2]->currentIndex();
					switch (ly->varVLC->cb_phy_op_mode[2]->currentIndex())
					{
						case 0:
							vlc_var._rs_code.pre_rs_in = 32;
							vlc_var._rs_code.pre_rs_out = 64;
							vlc_var.clock_rate=15e6;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=1; vlc_var.MCSID[2]=0;
							vlc_var.MCSID[3]=1; vlc_var.MCSID[4]=0; vlc_var.MCSID[5]=1;
							break;
						case 1:
							vlc_var._rs_code.pre_rs_in = 128;
							vlc_var._rs_code.pre_rs_out = 160;
							vlc_var.clock_rate=15e6;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=1; vlc_var.MCSID[2]=0;
							vlc_var.MCSID[3]=1; vlc_var.MCSID[4]=1; vlc_var.MCSID[5]=0;
							break;
						case 2:
							vlc_var._rs_code.pre_rs_in = 64;
							vlc_var._rs_code.pre_rs_out = 128;
							vlc_var.clock_rate=30e6;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=1; vlc_var.MCSID[2]=0;
							vlc_var.MCSID[3]=1; vlc_var.MCSID[4]=1; vlc_var.MCSID[5]=1;
							break;
						case 3:
							vlc_var._rs_code.pre_rs_in = 128;
							vlc_var._rs_code.pre_rs_out = 160;
							vlc_var.clock_rate=30e6;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=1; vlc_var.MCSID[2]=1;
							vlc_var.MCSID[3]=0; vlc_var.MCSID[4]=0; vlc_var.MCSID[5]=0;
							break;
						case 4:
							vlc_var._rs_code.pre_rs_in = 32;
							vlc_var._rs_code.pre_rs_out =64;
							vlc_var.clock_rate=60e6;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=1; vlc_var.MCSID[2]=1;
							vlc_var.MCSID[3]=0; vlc_var.MCSID[4]=0; vlc_var.MCSID[5]=1;
							break;
						case 5:
							vlc_var._rs_code.pre_rs_in = 128;
							vlc_var._rs_code.pre_rs_out = 160;
							vlc_var.clock_rate=60e6;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=1; vlc_var.MCSID[2]=1;
							vlc_var.MCSID[3]=0; vlc_var.MCSID[4]=1; vlc_var.MCSID[5]=0;
							break;
						case 6:
							vlc_var._rs_code.pre_rs_in = 32;
							vlc_var._rs_code.pre_rs_out = 64;
							vlc_var.clock_rate=120e6;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=1; vlc_var.MCSID[2]=1;
							vlc_var.MCSID[3]=0; vlc_var.MCSID[4]=1; vlc_var.MCSID[5]=1;
							break;
						case 7:
							vlc_var._rs_code.pre_rs_in = 128;
							vlc_var._rs_code.pre_rs_out = 164;
							vlc_var.clock_rate=120e6;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=1; vlc_var.MCSID[2]=1;
							vlc_var.MCSID[3]=1; vlc_var.MCSID[4]=0; vlc_var.MCSID[5]=0;
							break;
						case 8:
							vlc_var._rs_code.pre_rs_in = 0;
							vlc_var._rs_code.pre_rs_out = 0;
							vlc_var.clock_rate=120e6;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=1; vlc_var.MCSID[2]=1;
							vlc_var.MCSID[3]=1; vlc_var.MCSID[4]=0; vlc_var.MCSID[5]=1;
							break;
					}
					break;
				case 1: //VPPM
					vlc_var.operating_mode = ly->varVLC->cb_phy_op_mode[3]->currentIndex();
					switch (ly->varVLC->cb_phy_op_mode[3]->currentIndex())
					{
						case 0:
							vlc_var._rs_code.pre_rs_in = 32;
							vlc_var._rs_code.pre_rs_out = 64;
							vlc_var.clock_rate=3.75e6;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=1; vlc_var.MCSID[2]=0;
							vlc_var.MCSID[3]=0; vlc_var.MCSID[4]=0; vlc_var.MCSID[5]=0;
							break;
						case 1:
							vlc_var._rs_code.pre_rs_in = 128;
							vlc_var._rs_code.pre_rs_out = 164;
							vlc_var.clock_rate=3.75e6;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=1; vlc_var.MCSID[2]=0;
							vlc_var.MCSID[3]=0; vlc_var.MCSID[4]=0; vlc_var.MCSID[5]=1;
							break;
						case 2:
							vlc_var._rs_code.pre_rs_in = 32;
							vlc_var._rs_code.pre_rs_out = 64;
							vlc_var.clock_rate=7.5e6;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=1; vlc_var.MCSID[2]=0;
							vlc_var.MCSID[3]=0; vlc_var.MCSID[4]=1; vlc_var.MCSID[5]=0;
							break;
						case 3:
							vlc_var._rs_code.pre_rs_in = 128;
							vlc_var._rs_code.pre_rs_out = 164;
							vlc_var.clock_rate=7.5e6;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=1; vlc_var.MCSID[2]=0;
							vlc_var.MCSID[3]=0; vlc_var.MCSID[4]=1; vlc_var.MCSID[5]=1;
							break;
						case 4:
							vlc_var._rs_code.pre_rs_in = 0;
							vlc_var._rs_code.pre_rs_out = 0;
							vlc_var.clock_rate=7.5e6;
							vlc_var.MCSID[0]=0; vlc_var.MCSID[1]=1; vlc_var.MCSID[2]=0;
							vlc_var.MCSID[3]=1; vlc_var.MCSID[4]=0; vlc_var.MCSID[5]=0;
							break;

					}
					break;
			}
			break;
	}

}

TxVLC::~TxVLC()
{
	if(poly && vlc_var.MCSID)
	{
		delete [] poly;
		delete [] vlc_var.MCSID;
	}
			
	//stop();
}

void TxVLC::stop()
{
	/*if (rxth)
	{
		rxth->terminate();
		rxth->wait();
		delete rxth;
		rxth = NULL;
	}*/
}

