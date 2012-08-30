// $Id$

#include "Correlator.h"
#include <volk/volk.h>
#include <gr_io_signature.h>
#include <malloc.h>
                
Correlator::Correlator(int _copy, float _th) :
	gr_block("Correlator", gr_make_io_signature(1, 1, sizeof(float)), gr_make_io_signature(1, 1, sizeof(float))),
	pattern(-1), copy(_copy), cpd(0), th(_th)
{
	float _TDP[4][60] = { { 1,1,1,1,-1,1,-1,1,1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,1,-1,1,-1,-1,1,1,-1,1,1,1,1,1,1,1,-1,1,-1,1,1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,1,-1,1,-1,-1,1,1,-1,1,1,1 },
					  { -1,-1,1,-1,1,1,1,-1,1,1,1,1,1,1,-1,1,1,-1,1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,1,-1,-1,1,-1,1,1,1,-1,1,1,1,1,1,1,-1,1,1,-1,1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,1 },
					  { 1,-1,-1,1,1,-1,-1,-1,-1,-1,1,-1,-1,1,1,-1,1,1,-1,-1,1,1,1,1,1,-1,1,1,-1,-1,1,-1,-1,1,1,-1,-1,-1,-1,-1,1,-1,-1,1,1,-1,1,1,-1,-1,1,1,1,1,1,-1,1,1,-1,-1 },
   					  { -1,1,-1,-1,-1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,1,1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,-1,-1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,1,1,1,-1,-1,1,-1,1,1,-1,1,-1 } };
	memcpy(TDP, _TDP, sizeof(TDP));
}
Correlator::sptr Correlator::Create(int _copy, float _th)
{
	return sptr(new Correlator(_copy, _th));
}
void Correlator::forecast(int no, gr_vector_int &ni)
{
	ni[0] = no+60;
}
void Correlator::Correlate(const float *iptr, float *tD, float *tC, int no)
{
	float *tN = (float *)_aligned_malloc(sizeof(float)*no, 16);
	for (int n = 0; n < no; n++)
	{
		if (is_unaligned()) 
		{
			volk_32f_x2_dot_prod_32f_u(tC+n, iptr+n, tD, 60);
			volk_32f_x2_dot_prod_32f_u(tN+n, iptr+n, iptr, 60);
		}
		else
		{
			volk_32f_x2_dot_prod_32f_a(tC+n, iptr+n, tD, 60);
			volk_32f_x2_dot_prod_32f_a(tN+n, iptr+n, iptr+n, 60);
		}
	}
	volk_32f_x2_multiply_32f_a(tC, tC, tC, no);
	volk_32f_s32f_multiply_32f_a(tN, tN, 30, no);
	volk_32f_x2_divide_32f_a(tC, tC, tN, no);
	_aligned_free(tN);
}
int Correlator::general_work(int no, gr_vector_int &ni, gr_vector_const_void_star &_i, gr_vector_void_star &_o) 
{
	const float *iptr = (const float *)_i[0]; 
	float *optr = (float *)_o[0];
	unsigned int o = 0, rtd = 0;
	if (!cpd)
	{
		unsigned int idx = 0;
		float *C = (float *)_aligned_malloc(sizeof(float)*no*(pattern == -1 ? 4 : 1), 16);
		if (pattern == -1)
		{
			for (int t = 0; t < 4; t++)
			{
				float *tC = (float *)_aligned_malloc(sizeof(float)*no, 16);
				Correlate(iptr, TDP[t], tC, no);
				memcpy(C+t*no, tC, sizeof(float)*no);
				_aligned_free(tC);
			}
		}
		else
			Correlate(iptr, TDP[pattern], C, no);
		volk_32f_index_max_16u_a(&idx, C, (pattern == -1 ? no*4 : no));
		if (C[idx] > 0.7)
		{
			if (pattern == -1)
				pattern = idx/no;
			//printf("Found sample %f at %d (%d)\n", C[idx], idx%no, pattern);
			cpd = copy;
			o = idx%no;
		}
		_aligned_free(C);
	}
	if (cpd)
	{
		rtd = std::min(cpd, no-o);
		memcpy(optr, iptr+o, sizeof(float)*rtd);
		cpd -= rtd;
	}
	if (o+rtd)
		consume_each(o+rtd);
	else //didnt found anything
		consume_each(no);
	return rtd;
}