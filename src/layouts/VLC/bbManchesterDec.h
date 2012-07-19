// $Id$
#ifndef _INC_BBMANCHESTERDEC_H_
#define _INC_BBMANCHESTERDEC_H_

#include <gr_block.h>

class bbManchesterDec : public gr_block
{
	public:
		typedef boost::shared_ptr<bbManchesterDec> sptr;
		static sptr Create(int,int);
		int general_work(int, gr_vector_int &,gr_vector_const_void_star&, gr_vector_void_star&);
		void forecast(int, gr_vector_int &);
		~bbManchesterDec();
	
	private:
		bbManchesterDec(int,int);
		int d_mode;
		int d_flag_cc; //1: after manchester, there is the cc decoder
};

#endif //_INC_BBMANCHESTERDEC_H_
