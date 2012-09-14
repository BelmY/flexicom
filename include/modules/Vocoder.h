// $Id$

#ifndef _INC_VOCODER_H_
#define _INC_VOCODER_H_

#include <gr_sync_decimator.h>
#include <gr_sync_interpolator.h>

class VocoderEncoder : public gr_sync_decimator 
{
	public:
		typedef boost::shared_ptr<VocoderEncoder> sptr;
		static sptr Create();
		~VocoderEncoder();
		
	private:
		VocoderEncoder();
		int work (int, gr_vector_const_void_star &, gr_vector_void_star &);
		struct gsm_state *dgsm;
		int reset;
};

class VocoderDecoder : public gr_sync_interpolator
{
	public:
		typedef boost::shared_ptr<VocoderDecoder> sptr;
		static sptr Create();
		~VocoderDecoder();
		
	private:
		VocoderDecoder();
		int work (int, gr_vector_const_void_star &, gr_vector_void_star &);
		struct gsm_state *dgsm;
};

#endif //_INC_VOCODER_H_