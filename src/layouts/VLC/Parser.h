// $Id$

#ifndef _INC_PARSER_H_
#define _INC_PARSER_H_

#include <gr_block.h>
#include "compat.h"

PACK(
struct
{
	unsigned FV:2;
	unsigned rvd:4;
	unsigned FT:3;
	unsigned SE:1;
	unsigned FP:1;
	unsigned AR:1;
	unsigned DAM:2;
	unsigned SAM:2;
} , FrameControl );

PACK(
struct
{
	unsigned B:1;
	unsigned CN:3;
	unsigned MCS:6;
	unsigned PL:16;
	unsigned DO:1;
	unsigned rvd:5;
} , PhyHdr );

class Parser : public gr_block
{
	public:
		typedef enum { PHR , PSDU } Type;
		typedef boost::shared_ptr<Parser> sptr;
		static sptr Create(Type, int = 0);
		int general_work(int, gr_vector_int &, gr_vector_const_void_star &, gr_vector_void_star &);
		
	private:
		Parser(Type, int);
		unsigned int ic;
		Type type;
		void PHRParser(unsigned int);
		void PSDUParser(unsigned char *);
		unsigned int PHRData;
		int psdu_len;
		unsigned char MHR[5];
};

#endif //_INC_PARSER_H_
