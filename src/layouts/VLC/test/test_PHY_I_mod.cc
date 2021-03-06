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

#include <cppunit/extensions/HelperMacros.h>
#include "bbRSEnc.h"
#include "InterPunct.h"
#include "bbCCEnc.h"
#include "bbManchesterEnc.h"
#include "bb4b6bEnc.h"
#include "bbRSDec.h"
#include "bbCCDec.h"
#include "bbManchesterDec.h"
#include "bb_bit_removal.h"
#include "bbPSDU_generation.h"
#include <gnuradio/file_source.h>
#include <gnuradio/top_block.h>
#include <gnuradio/vector_sink_f.h>
#include <gnuradio/float_to_complex.h>
#include <vector>
#include <gnuradio/float_to_int.h>
#include <gnuradio/char_to_float.h>
#include <gnuradio/int_to_float.h>
#include <gnuradio/null_sink.h>
#include "MSE.h"
#include "bbMatlab.h"

class Test_PHY_I_modulator : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(Test_PHY_I_modulator);
	CPPUNIT_TEST(test_PHR_OOK_mod_0);
	CPPUNIT_TEST(test_PSDU_OOK_mod_0);
	CPPUNIT_TEST(test_PSDU_VPPM_mod_2);
	CPPUNIT_TEST(test_PSDU_OOK_demod_0);
	CPPUNIT_TEST_SUITE_END();
	
	public:
		void test_PHR_OOK_mod_0()
		{
			unsigned int GF = 4, N = 15, K = 7, phy = 0, phr_len = 54, len_int= 30;
			int poly[] = { 0133, 0171, 0165 };
			int N_cc = 3, K_cc = 7, len_cc = 120, data_rate=0; //30*4
			int d_mode=0, len_man=504;
						
			gr_top_block_sptr gt = gr_make_top_block("PHR_OOK_mod_0");
			bbRSEnc::sptr rs = bbRSEnc::Create(&GF, &N, &K, &phy, &phr_len);
			//bbVLCInterleaver::sptr intlv = bbVLCInterleaver::Create(GF, N, K , phr_len, len_int);
			InterPunct::sptr intlv = InterPunct::Create(GF, N, K , phr_len, len_int, InterPunct::INTERPUNCT);
			bbCCEnc::sptr cc = bbCCEnc::Create(N_cc, K_cc, poly, len_cc, data_rate);
			bbManchesterEnc::sptr RLL = bbManchesterEnc::Create(d_mode);
			gr_file_source_sptr fi = gr_make_file_source(sizeof(float), "src/layouts/VLC/test/vecs/PHR_data.dat");
			gr_file_source_sptr fo = gr_make_file_source(sizeof(float), "src/layouts/VLC/test/vecs/PHR_out_manchester.dat");
			MSE::sptr mse = MSE::Create();
			gr_float_to_complex_sptr f2c0 = gr_make_float_to_complex();
			gr_float_to_complex_sptr f2c1 = gr_make_float_to_complex();
			gr_vector_sink_f_sptr vec = gr_make_vector_sink_f(1);
			gr_float_to_int_sptr f2i = gr_make_float_to_int();
			gr_int_to_float_sptr i2f = gr_make_int_to_float(1, 1.0);
			gt->connect(fi, 0, f2i, 0);
			gt->connect(f2i, 0, rs, 0);
			gt->connect(rs, 0, intlv, 0);
			gt->connect(intlv, 0, cc, 0);
			gt->connect(cc, 0, RLL, 0);
			gt->connect(RLL, 0, i2f, 0);
			gt->connect(i2f, 0, f2c0, 0);
			gt->connect(fo, 0, f2c1, 0);
			gt->connect(f2c0, 0, mse, 0);
			gt->connect(f2c1, 0, mse, 1);
			gt->connect(mse, 0, vec, 0);
			gt->run();
			std::vector<float> data = vec->data();
			for (unsigned int i = 0; i < data.size(); i++)
   				CPPUNIT_ASSERT_DOUBLES_EQUAL(0, data[i], 10e-9);
		}
		
		void test_PSDU_OOK_mod_0()
		{
			unsigned int GF = 4, N = 15, K = 7, phy = 0, phr_len = 80, len_int= 45;
			int poly[] = { 0133, 0171, 0165 };
			int N_cc = 3, K_cc = 7, len_cc = 176, data_rate=0; //30*4
			int d_mode=0, len_man=728;
						
			gr_top_block_sptr gt = gr_make_top_block("PSDU_OOK_mod_0");
			bbRSEnc::sptr rs = bbRSEnc::Create(&GF, &N, &K, &phy, &phr_len);
			//bbVLCInterleaver::sptr intlv = bbVLCInterleaver::Create(GF, N, K , phr_len, len_int);
			InterPunct::sptr intlv = InterPunct::Create(GF, N, K , phr_len, len_int, InterPunct::INTERPUNCT);
			bbCCEnc::sptr cc = bbCCEnc::Create(N_cc, K_cc, poly, len_cc, data_rate);
			bbManchesterEnc::sptr RLL = bbManchesterEnc::Create(d_mode);
			gr_file_source_sptr fi = gr_make_file_source(sizeof(float), "src/layouts/VLC/test/vecs/PSDU_data.dat");
			gr_file_source_sptr fo = gr_make_file_source(sizeof(float), "src/layouts/VLC/test/vecs/PSDU_out_manchester.dat");
			MSE::sptr mse = MSE::Create();
			gr_float_to_complex_sptr f2c0 = gr_make_float_to_complex();
			gr_float_to_complex_sptr f2c1 = gr_make_float_to_complex();
			gr_vector_sink_f_sptr vec = gr_make_vector_sink_f(1);
			gr_float_to_int_sptr f2i = gr_make_float_to_int();
			gr_int_to_float_sptr i2f = gr_make_int_to_float(1, 1.0);
			gt->connect(fi, 0, f2i, 0);
			gt->connect(f2i, 0, rs, 0);
			gt->connect(rs, 0, intlv, 0);
			gt->connect(intlv, 0, cc, 0);
			gt->connect(cc, 0, RLL, 0);
			gt->connect(RLL, 0, i2f, 0);
			gt->connect(i2f, 0, f2c0, 0);
			gt->connect(fo, 0, f2c1, 0);
			gt->connect(f2c0, 0, mse, 0);
			gt->connect(f2c1, 0, mse, 1);
			gt->connect(mse, 0, vec, 0);
			gt->run();
			std::vector<float> data = vec->data();
			for (unsigned int i = 0; i < data.size(); i++)
   				CPPUNIT_ASSERT_DOUBLES_EQUAL(0, data[i], 10e-9);
		}
		
		void test_PSDU_VPPM_mod_2()
		{
			unsigned int GF = 4, N = 15, K = 7, phy = 0, phr_len = 80, len_int= 45;
			int poly[] = { 0133, 0171, 0165 };
												
			gr_top_block_sptr gt = gr_make_top_block("PSDU_VPPM_mod_2");
			bbRSEnc::sptr rs = bbRSEnc::Create(&GF, &N, &K, &phy, &phr_len);
			//bbVLCInterleaver::sptr intlv = bbVLCInterleaver::Create(GF, N, K , phr_len, len_int);
			InterPunct::sptr intlv = InterPunct::Create(GF, N, K , phr_len, len_int, InterPunct::INTERPUNCT);
			bb4b6bEnc::sptr RLL = bb4b6bEnc::Create();
			gr_file_source_sptr fi = gr_make_file_source(sizeof(float), "src/layouts/VLC/test/vecs/PSDU_data.dat");
			gr_file_source_sptr fo = gr_make_file_source(sizeof(float), "src/layouts/VLC/test/vecs/PSDU_out_4b6b.dat");
			MSE::sptr mse = MSE::Create();
			gr_float_to_complex_sptr f2c0 = gr_make_float_to_complex();
			gr_float_to_complex_sptr f2c1 = gr_make_float_to_complex();
			gr_vector_sink_f_sptr vec = gr_make_vector_sink_f(1);
			gr_float_to_int_sptr f2i = gr_make_float_to_int();
			gr_int_to_float_sptr i2f = gr_make_int_to_float(1, 1.0);
			gt->connect(fi, 0, f2i, 0);
			gt->connect(f2i, 0, rs, 0);
			gt->connect(rs, 0, intlv, 0);
			gt->connect(intlv, 0, RLL, 0);
			gt->connect(RLL, 0, i2f, 0);
			gt->connect(i2f, 0, f2c0, 0);
			gt->connect(fo, 0, f2c1, 0);
			gt->connect(f2c0, 0, mse, 0);
			gt->connect(f2c1, 0, mse, 1);
			gt->connect(mse, 0, vec, 0);
			gt->run();
			std::vector<float> data = vec->data();
			for (unsigned int i = 0; i < data.size(); i++)
   				CPPUNIT_ASSERT_DOUBLES_EQUAL(0, data[i], 10e-9);
		}
		
		void test_PSDU_OOK_demod_0()
		{
			int d_mode=0;
			int N_cc = 3, K_cc = 7, len_cc = 728, data_rate=0; 
			int poly[] = { 0133, 0171, 0165 };
			unsigned int GF = 4, N = 15, K = 7, psdu_len = 10*8, len_deint = 176;
			unsigned int phy = 0, rs_len = 45;
						
			gr_top_block_sptr gt = gr_make_top_block("PSDU_OOK_demod_0");
			bbManchesterDec::sptr RLL_dec = bbManchesterDec::Create(d_mode,1);
			bbCCDec::sptr cc_dec = bbCCDec::Create(N_cc, K_cc, poly, len_cc, data_rate);
			InterPunct::sptr de_intlv = InterPunct::Create(GF, N, K , psdu_len, len_deint, InterPunct::DEINTERPUNCT);
			bbRSDec::sptr rs_dec = bbRSDec::Create(GF, N, K, phy, rs_len);
			bb_bit_removal::sptr br = bb_bit_removal::Create(rs_dec->out_rs_dec,psdu_len);
			gr_file_source_sptr fi = gr_make_file_source(sizeof(float), "src/layouts/VLC/test/vecs/PSDU_out_manchester.dat");
			gr_file_source_sptr fo = gr_make_file_source(sizeof(float), "src/layouts/VLC/test/vecs/PSDU_data.dat");
			MSE::sptr mse = MSE::Create();
			gr_float_to_complex_sptr f2c0 = gr_make_float_to_complex();
			gr_float_to_complex_sptr f2c1 = gr_make_float_to_complex();
			gr_vector_sink_f_sptr vec = gr_make_vector_sink_f(1);
			gr_float_to_int_sptr f2i = gr_make_float_to_int();
			gr_int_to_float_sptr i2f = gr_make_int_to_float(1, 1.0);
			gt->connect(fi, 0, f2i, 0);
			gt->connect(f2i, 0, RLL_dec, 0);
			gt->connect(RLL_dec, 0, cc_dec,0);
			gt->connect(cc_dec, 0, de_intlv, 0);
			gt->connect(de_intlv, 0, rs_dec, 0);
			gt->connect(rs_dec, 0, br,0);
			gt->connect(br, 0, i2f, 0);
			gt->connect(i2f, 0, f2c0, 0);
			gt->connect(fo, 0, f2c1, 0);
			gt->connect(f2c0, 0, mse, 0);
			gt->connect(f2c1, 0, mse, 1);
			gt->connect(mse, 0, vec, 0);
			gt->run();
			std::vector<float> data = vec->data();
			for (unsigned int i = 0; i < data.size(); i++)
   				CPPUNIT_ASSERT_DOUBLES_EQUAL(0, data[i], 10e-9);
		}
		
		
};
CPPUNIT_TEST_SUITE_REGISTRATION(Test_PHY_I_modulator);
