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

#include "Tcp.h"
#include <gnuradio/blocks/file_descriptor_source.h>
#include <gnuradio/blocks/file_descriptor_sink.h>
#include <sys/types.h>
#ifdef _WIN
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#define dup(x) _dup(x)
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

TcpIO::TcpIO(const char *addr_str, unsigned short port, bool server, gr::io_signature::sptr in, gr::io_signature::sptr out) :
	gr::hier_block2("TcpIO", in, out)
{
	svfd = 0;
	if (server)
	{
#ifdef _WIN
		fd = _open("tmp-pipe", _O_CREAT | _O_TEMPORARY | _O_SEQUENTIAL | _O_RDWR | _O_BINARY, _S_IREAD | _S_IWRITE);
#else
		svfd = socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = INADDR_ANY;
		memset(&(addr.sin_zero), 0, 8);
		addr.sin_port = htons(port);
		bind(svfd, (struct sockaddr *)&addr, sizeof(addr));
		listen(svfd, 4);
		fd = accept(svfd, 0, 0);
#endif
	}
	else
	{
#ifdef _WIN
		fd = _open("tmp-pipe", _O_CREAT | _O_TEMPORARY | _O_SEQUENTIAL | _O_RDWR | _O_BINARY, _S_IREAD | _S_IWRITE);
#else
		fd = socket(AF_INET, SOCK_STREAM, 0);
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		memset(&(addr.sin_zero), 0, 8);
		addr.sin_port = htons(port);
		::connect(fd, (struct sockaddr *)&addr, sizeof(addr));
#endif
	}
}
TcpIO::~TcpIO()
{
	close(fd);
}
TcpSource::TcpSource(size_t siz, const char *addr_str, unsigned short port, bool server) :
	TcpIO(addr_str, port, server, gr::io_signature::make(0, 0, 0), gr::io_signature::make(1, 1, siz))
{
	gr::blocks::file_descriptor_source::sptr source = gr::blocks::file_descriptor_source::make(siz, dup(fd));
	connect(source, 0, self(), 0);
}
TcpSource::sptr TcpSource::Create(size_t siz, const char *addr_str, unsigned short port, bool server)
{
	return gnuradio::get_initial_sptr(new TcpSource(siz, addr_str, port, server));
}

TcpSink::TcpSink(size_t siz, const char *addr_str, unsigned short port, bool server) :
	TcpIO(addr_str, port, server, gr::io_signature::make(1, 1, siz), gr::io_signature::make(0, 0, 0))
{
	gr::blocks::file_descriptor_sink::sptr sink = gr::blocks::file_descriptor_sink::make(siz, dup(fd));
	connect(self(), 0, sink, 0);
}
TcpSink::sptr TcpSink::Create(size_t siz, const char *addr_str, unsigned short port, bool server)
{
	return gnuradio::get_initial_sptr(new TcpSink(siz, addr_str, port, server));
}
