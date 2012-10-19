// $Id$

#include "bb_Header_cp.h"
#include "LayoutVLC.h"
#include <gr_io_signature.h>
#include <math.h>

bb_Header_cp::bb_Header_cp(Type _type, int _raw_length, LayoutVLC *_ly) :
	gr_block("bb_Header_cp", gr_make_io_signature(1, 1, sizeof(int)), gr_make_io_signature(1, 1, sizeof(unsigned char))),
	raw_length(_raw_length), type(_type), ly(_ly), CRCok(0), CRCnok(0)
{
	if (type == PHR)
		length = 48;
	else if (type == PSDU)
		length = raw_length;
	olen = length/8-2;
	set_output_multiple(olen);
	bits = total = 0;
}
bb_Header_cp::sptr bb_Header_cp::Create(Type _type, int _raw_length, LayoutVLC *_ly)
{
	return sptr(new bb_Header_cp(_type, _raw_length, _ly));
}

void bb_Header_cp::forecast(int noutput_items, gr_vector_int &ninput_items_required) 
{
	int ninputs = ninput_items_required.size();
	for (int i=0; i < ninputs; i++)
		ninput_items_required[i] = (noutput_items/olen)*raw_length; //16 = length of the crc
}
#include <QMutex>
QMutex mtx;
int bb_Header_cp::general_work(int noutput_items, gr_vector_int &ninput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items) 
{
	const int *iptr = (const int *)input_items[0];
	unsigned char *optr = (unsigned char *)output_items[0];
	int blocks_to_process = (noutput_items/olen);
	int *tmp = new int[length+LayoutVLC::CRC_LENGTH];
	memset(tmp,0,sizeof(int)*length+LayoutVLC::CRC_LENGTH);
	const uint64_t nread = nitems_read(0);
	std::vector<gr_tag_t> tags;
	get_tags_in_range(tags, 0, nread, nread+ninput_items[0], pmt::pmt_string_to_symbol("BER"));
	if (tags.size())
		ly->EmitChangeMetric((QLabel *)ly->gridErrors->itemAtPosition(0, 1)->widget(), QString::number(pmt::pmt_to_double(tags[0].value)));
	int rtd = 0;
	for (int i = 0; i < blocks_to_process; i++)
	{
		memcpy(tmp, iptr, sizeof(int)*length);
		if (LayoutVLC::CheckCRC(tmp, length)) //crc ok!!
		{
			for (int n = 0; n < length-LayoutVLC::CRC_LENGTH; n += 8)
				optr[rtd++] = (unsigned char)LayoutVLC::bi2dec((int *)iptr+n, 8);
			//printf("%s OK!\n", type == PHR ? "PHR" : "PSDU");
			ly->EmitChangeMetric((QLabel *)ly->gridErrors->itemAtPosition(4,1)->widget(), QString::number(++CRCok));
		}
		else
			ly->EmitChangeMetric((QLabel *)ly->gridErrors->itemAtPosition(3,1)->widget(), QString::number(++CRCnok));
		iptr += raw_length;
	}
	delete [] tmp;
	consume_each(raw_length*blocks_to_process);
	return rtd;
}
