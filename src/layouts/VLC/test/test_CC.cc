#include <cppunit/extensions/HelperMacros.h>
#include "bbCCEnc.h"
#include <gr_file_source.h>
#include <gr_top_block.h>
#include <gr_vector_sink_f.h>
#include <gr_float_to_complex.h>
#include <vector>
#include <gr_float_to_int.h>
#include <gr_char_to_float.h>
#include <gr_int_to_float.h>
#include "MSE.h"
#include "bbMatlab.h"
#include <stdio.h>

class TestConvolutionalCode : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(TestConvolutionalCode);
	CPPUNIT_TEST(test_Encode_1_4_PHR);
	CPPUNIT_TEST(test_Encode_1_4_PSDU);	
	CPPUNIT_TEST(test_Encode_1_3_PSDU);	
	CPPUNIT_TEST(test_Encode_2_3_PSDU);	
	CPPUNIT_TEST_SUITE_END();
	
	public:
		void test_Encode_1_4_PHR()
		{
			int *poly;
			poly = new int[3];
			poly[0]=0133; poly[1]=0171;	poly[2]=0165;
			int N = 3, K = 7, len = 120, data_rate=0; //30*4
			gr_top_block_sptr gt = gr_make_top_block("CCEncode_1_4_PHR");
			bbCCEnc::sptr cc = bbCCEnc::Create(N, K, poly, len, data_rate);
			gr_file_source_sptr fi = gr_make_file_source(sizeof(float), "src/layouts/VLC/test/vecs/PHR_in_cc_1_4.dat");
			gr_file_source_sptr fo = gr_make_file_source(sizeof(float), "src/layouts/VLC/test/vecs/PHR_out_cc_1_4.dat");
			MSE::sptr mse = MSE::Create();
			gr_float_to_complex_sptr f2c0 = gr_make_float_to_complex();
			gr_float_to_complex_sptr f2c1 = gr_make_float_to_complex();
			gr_vector_sink_f_sptr vec = gr_make_vector_sink_f(1);
			gr_float_to_int_sptr f2i = gr_make_float_to_int();
			gr_int_to_float_sptr i2f = gr_make_int_to_float(1, 1.0);
			gt->connect(fi, 0, f2i, 0);
			gt->connect(f2i, 0, cc, 0);
			gt->connect(cc, 0, i2f, 0);
			gt->connect(i2f, 0, f2c0, 0);
			gt->connect(fo, 0, f2c1, 0);
			gt->connect(f2c0, 0, mse, 0);
			gt->connect(f2c1, 0, mse, 1);
			gt->connect(mse, 0, vec, 0);
			gt->run();
			std::vector<float> data = vec->data();
			for (unsigned int i = 0; i < data.size(); i++)
   				CPPUNIT_ASSERT_DOUBLES_EQUAL(0, data[i], 10e-9);
   			delete [] poly;
		}
		
		void test_Encode_1_4_PSDU()
		{
			int *poly;
			poly = new int[3];
			poly[0]=0133; poly[1]=0171;	poly[2]=0165;
			int N = 3, K = 7, len = 176, data_rate=0; //44*4
			gr_top_block_sptr gt = gr_make_top_block("CCEncode_1_4_PSDU");
			
			bbCCEnc::sptr cc = bbCCEnc::Create(N, K, poly, len, data_rate);
			gr_file_source_sptr fi = gr_make_file_source(sizeof(float), "src/layouts/VLC/test/vecs/PSDU_in_cc_1_4.dat");
			gr_file_source_sptr fo = gr_make_file_source(sizeof(float), "src/layouts/VLC/test/vecs/PSDU_out_cc_1_4.dat");
			MSE::sptr mse = MSE::Create();
			gr_float_to_complex_sptr f2c0 = gr_make_float_to_complex();
			gr_float_to_complex_sptr f2c1 = gr_make_float_to_complex();
			gr_vector_sink_f_sptr vec = gr_make_vector_sink_f(1);
			gr_float_to_int_sptr f2i = gr_make_float_to_int();
			gr_int_to_float_sptr i2f = gr_make_int_to_float(1, 1.0);
			gt->connect(fi, 0, f2i, 0);
			gt->connect(f2i, 0, cc, 0);
			gt->connect(cc, 0, i2f, 0);
			gt->connect(i2f, 0, f2c0, 0);
			gt->connect(fo, 0, f2c1, 0);
			gt->connect(f2c0, 0, mse, 0);
			gt->connect(f2c1, 0, mse, 1);
			gt->connect(mse, 0, vec, 0);
			gt->run();
			std::vector<float> data = vec->data();
			printf("Hola\n");	
			for (unsigned int i = 0; i < data.size(); i++)
   				CPPUNIT_ASSERT_DOUBLES_EQUAL(0, data[i], 10e-9);
   			delete [] poly;
		}
		
		void test_Encode_1_3_PSDU()
		{
			int *poly;
			poly = new int[3];
			poly[0]=0133; poly[1]=0171;	poly[2]=0165;
			int N = 3, K = 7, len = 112, data_rate=1; //33*4
			gr_top_block_sptr gt = gr_make_top_block("CCEncode_1_3_PSDU");
			
			bbCCEnc::sptr cc = bbCCEnc::Create(N, K, poly, len, 1);
			gr_file_source_sptr fi = gr_make_file_source(sizeof(float), "src/layouts/VLC/test/vecs/PSDU_in_cc_1_3.dat");
			gr_file_source_sptr fo = gr_make_file_source(sizeof(float), "src/layouts/VLC/test/vecs/PSDU_out_cc_1_3.dat");
			MSE::sptr mse = MSE::Create();
			gr_float_to_complex_sptr f2c0 = gr_make_float_to_complex();
			gr_float_to_complex_sptr f2c1 = gr_make_float_to_complex();
			gr_vector_sink_f_sptr vec = gr_make_vector_sink_f(1);
			gr_float_to_int_sptr f2i = gr_make_float_to_int();
			gr_int_to_float_sptr i2f = gr_make_int_to_float(1, 1.0);
			gt->connect(fi, 0, f2i, 0);
			gt->connect(f2i, 0, cc, 0);
			gt->connect(cc, 0, i2f, 0);
			gt->connect(i2f, 0, f2c0, 0);
			gt->connect(fo, 0, f2c1, 0);
			gt->connect(f2c0, 0, mse, 0);
			gt->connect(f2c1, 0, mse, 1);
			gt->connect(mse, 0, vec, 0);
			gt->run();
			std::vector<float> data = vec->data();
			for (unsigned int i = 0; i < data.size(); i++)
   				CPPUNIT_ASSERT_DOUBLES_EQUAL(0, data[i], 10e-9);
   			delete [] poly;
		}
		
		void test_Encode_2_3_PSDU()
		{
			int *poly;
			poly = new int[3];
			poly[0]=0133; poly[1]=0171;	poly[2]=0165;
			int N = 3, K = 7, len = 112, data_rate=2; //33*4
			gr_top_block_sptr gt = gr_make_top_block("CCEncode_2_3_PSDU");
			
			bbCCEnc::sptr cc = bbCCEnc::Create(N, K, poly, len, data_rate);
			gr_file_source_sptr fi = gr_make_file_source(sizeof(float), "src/layouts/VLC/test/vecs/PSDU_in_cc_2_3.dat");
			gr_file_source_sptr fo = gr_make_file_source(sizeof(float), "src/layouts/VLC/test/vecs/PSDU_out_cc_2_3.dat");
			MSE::sptr mse = MSE::Create();
			gr_float_to_complex_sptr f2c0 = gr_make_float_to_complex();
			gr_float_to_complex_sptr f2c1 = gr_make_float_to_complex();
			gr_vector_sink_f_sptr vec = gr_make_vector_sink_f(1);
			gr_float_to_int_sptr f2i = gr_make_float_to_int();
			gr_int_to_float_sptr i2f = gr_make_int_to_float(1, 1.0);
			gt->connect(fi, 0, f2i, 0);
			gt->connect(f2i, 0, cc, 0);
			gt->connect(cc, 0, i2f, 0);
			gt->connect(i2f, 0, f2c0, 0);
			gt->connect(fo, 0, f2c1, 0);
			gt->connect(f2c0, 0, mse, 0);
			gt->connect(f2c1, 0, mse, 1);
			gt->connect(mse, 0, vec, 0);
			gt->run();
			std::vector<float> data = vec->data();
			for (unsigned int i = 0; i < data.size(); i++)
   				CPPUNIT_ASSERT_DOUBLES_EQUAL(0, data[i], 10e-9);
   			delete [] poly;
		}
};
CPPUNIT_TEST_SUITE_REGISTRATION(TestConvolutionalCode);