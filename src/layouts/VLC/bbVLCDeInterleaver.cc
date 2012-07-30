// $Id$
#include "bbVLCDeInterleaver.h"
#include <gr_io_signature.h>
#include "LayoutVLC.h"

//this block performs the interleaving and the puncturing process
//described in section 10.3
bbVLCDeInterleaver::bbVLCDeInterleaver (unsigned int _GF, unsigned int _N, unsigned int _K, unsigned int _raw_length, unsigned int _pre_length):
	gr_block("bbVLCDeInterleaver", gr_make_io_signature (1,1,sizeof (int)), gr_make_io_signature (1,1,sizeof(int))),
	GF(_GF), N(_N), K(_K), raw_length (_raw_length), pre_length(_pre_length)
{
	//equations in section 10.3 of IEEE 802.15.7
	int S_frame, D,S;
	unsigned int p, i, S_block;
	//raw_length is a number of bits
	//pre_length is a number of the incoming bits
	//out_deint is a number of decimal values	
	S_frame = ceil((double)raw_length/GF);
	S = N * ceil((double)S_frame/K) -  (K- (S_frame%K));
	//tmp = N*ceil((double)(S_frame/K));
	D = ceil(((double)S/N));
	S_block = N * D;
	p = N - (S%N);
	//printf ("El valor de S_frame:%d, el valor de S:%d, el valor de D:%d, S_block:%d, p:%d\n", S_frame,S,D, S_block,p);
	
	interleaving_vector = new int[S_block]; //l vector
	puncturing_vector = new unsigned int[p]; //z vector
	for (i=0; i<S_block; i++)
	{
		interleaving_vector[i]= (i%D)*N + (int) floor(((double)i/D));
		//printf("El valor de interleaving_vector[%d]=%d\n", i,interleaving_vector[i]);
	}
	for (i=0; i<p; i++)
	{
		puncturing_vector[i]= (N-p+1)*D + (i*D)-1;
	}
	out_deint= pre_length/GF;
	len_punct_vector= p;
	//printf("El valor de out_deint:%d\n",out_deint);
	set_output_multiple(out_deint);
}

bbVLCDeInterleaver::~bbVLCDeInterleaver()
{
	if (interleaving_vector || puncturing_vector)
	{
		delete [] interleaving_vector;
		delete [] puncturing_vector;
		interleaving_vector =0;
		puncturing_vector =0;
	}
}
		
bbVLCDeInterleaver::sptr bbVLCDeInterleaver::Create(unsigned int _GF, unsigned int _N, unsigned int _K, unsigned int _raw_length, unsigned int _pre_length)
{
	return sptr(new bbVLCDeInterleaver(_GF, _N, _K, _raw_length, _pre_length));
}
void bbVLCDeInterleaver::forecast(int noutput_items, gr_vector_int &ninput_items_required) 
{
	int ninputs = ninput_items_required.size();
	for (int i=0; i < ninputs; i++)
		ninput_items_required[i]= (noutput_items/out_deint)*pre_length;
}
int bbVLCDeInterleaver::general_work(int noutput_items, gr_vector_int &ninput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items) 
{
	int *iptr= (int *)input_items[0];
	int *optr= (int *)output_items[0];
	int blocks_to_process,l;
	unsigned int GF_words = pre_length/GF, i, j; //this division will be always an integer number
	//printf("\n\n GF_words: %d\n", GF_words);
	int *tmp = new int[GF_words];
	int *tmp2 = new int[GF_words];
	int *tmp3 = new int[GF_words+len_punct_vector];
	//memset(tmp3,0, sizeof(int)*(GF_words+ len_punct_vector));
	blocks_to_process=(noutput_items/out_deint);
	
	while (blocks_to_process > 0)
	{
		memset(tmp3,0, sizeof(int)*(GF_words+ len_punct_vector));
		for (i=0; i< GF_words; i++)
		{
			tmp[i]=LayoutVLC::bi2dec(iptr,GF);
			//printf("El input es[%d]= %d\n",i,tmp[i]);
			iptr = iptr + GF;
		}
		if(len_punct_vector< K)
		{
			j=0; l=0;
			for(i=0; i<GF_words + len_punct_vector; i++)
			{
				if (i != puncturing_vector[l])
					tmp2[j++]=i;
				else
					l++;
			}
			assert(j == GF_words);
			assert(l == len_punct_vector);
			for (i=0; i<j; i++)
			{
				tmp3[tmp2[i]]=tmp[i];
			}
			//deinterleaving properly said
			for(i=0; i<GF_words + len_punct_vector; i++)
				optr[interleaving_vector[i]]=tmp3[i];
		}
		else
		{
			memcpy(tmp3, tmp, sizeof(int)*GF_words);	
			//deinterleaving properly said
			for(i=0; i<GF_words; i++)
				optr[interleaving_vector[i]]=tmp3[i];
		}
		optr += GF_words;		
		blocks_to_process--;
		
	}
	delete [] tmp;
	delete [] tmp2;
	delete [] tmp3;
	consume_each((noutput_items/out_deint)*pre_length);
	return noutput_items;
}
