//============================================================================
// Name        : UdpForward.cpp
// Author      : chuanjiang.zh
// Version     :
// Copyright   : Copyright 2017 100wits.com
// Description : Hello World in C, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "Socket.h"
#include "OptionParser.h"
#include "TStringUtil.h"
#include "TByteBuffer.h"
#include <vector>
#include "TFileUtil.h"
#include "TThread.h"


static const int RECV_BUFFER_SIZE = 4096000;

class Application : public comn::Thread
{
public:
	Application():
		m_bufSize(RECV_BUFFER_SIZE),
		m_count(),
		m_dropRate(),
		m_dropped()
	{
		comn::Socket::startup();

		m_buffer.ensure(1024 * 1024 * 2);
	}

	~Application()
	{
		close();
		comn::Socket::cleanup();
	}

	bool open(const std::string& ip, int port)
	{
		printf("open %s:%d, recv bufsize:%d\n", ip.c_str(), port, m_bufSize);

		if (!m_socket.open(SOCK_DGRAM))
		{
			return false;
		}

		m_socket.setRecvBufferSize(m_bufSize);

		comn::SockAddr addr(ip, port);
		int ret = m_socket.bind(addr);
		return (ret == 0);
	}

	void close()
	{
		m_socket.close();

		m_count = 0;
	}

	bool add(const std::string& addr)
	{
		printf("target:%s\n", addr.c_str());

		std::string ip;
		int port = 0;
		comn::StringUtil::split(addr, ':', ip, port);

		if (ip.empty())
		{
			return false;
		}

		if (port <= 0)
		{
			return false;
		}

		comn::SockAddr sockAddr(ip, port);
		m_addrArray.push_back(sockAddr);

		return true;
	}

	void setRecvBufSize(int size)
	{
		m_bufSize = size;
	}

	void setDumpFile(const std::string& filename)
	{
		m_filename = filename;
	}

	void setDropRate(double rate)
	{
		m_dropRate = rate;
	}

	bool shouldDrop()
	{
		if (m_dropRate <= 0)
		{
			return false;
		}

		if (m_count <= 0)
		{
			return false;
		}
		return ((double)m_dropped / m_count) < m_dropRate;
	}

protected:
	void forward(unsigned char* data, int length)
	{
		for (size_t i = 0; i < m_addrArray.size(); ++ i)
		{
			comn::SockAddr& addr = m_addrArray[i];
			m_socket.sendTo((char*)data, length, 0, addr);
		}
	}

	virtual int run()
	{
		while (!m_canExit)
		{
			if (!m_socket.checkReadable(1000))
			{
				continue;
			}

			comn::SockAddr addr;
			int length = m_socket.receiveFrom((char*)m_buffer.data(), m_buffer.capacity(), 0, addr);
			if (length <= 0)
			{
				continue;
			}

			//printf("recv. count:%lld, length:%d\n", m_count, length);

			if (!m_filename.empty())
			{
				comn::FileUtil::write(m_buffer.data(), length, m_filename.c_str(),
					m_count > 0 ? true : false);
			}

			if (shouldDrop())
			{
				m_dropped++;
			}
			else
			{
				forward(m_buffer.data(), length);
			}

			m_count++;
		}
		return 0;
	}

protected:

	comn::Socket	m_socket;
	comn::ByteBuffer	m_buffer;

	typedef std::vector< comn::SockAddr >	SockAddrArray;
	SockAddrArray	m_addrArray;
	int	m_bufSize;

	std::string	m_filename;
	int64_t m_count;

	double	m_dropRate;
	int64_t	m_dropped;


};


int main(int argc, char** argv)
{
	OptionParser optionParser("udpforward [option] sink_address", "1.0");
	optionParser.enableHelp();
	optionParser.enableVersion();

	optionParser.addOption('p', "port", true, "", "local port.");
	optionParser.addOption('b', "bufsize", true, "", "recv buffer size of socket");
	optionParser.addOption('f', "file", true, "", "dump file. such as out.ts");
	optionParser.addOption('d', "drop", true, "", "rate of drop packet. must be in [0.0-1.0]");

	if (!optionParser.parse(argc, argv))
	{
		optionParser.usage();
		return 0;
	}

	if (optionParser.hasHelpOption())
	{
		optionParser.usage();
		return 0;
	}

	if (optionParser.hasVersionOption())
	{
		optionParser.version();
		return 0;
	}

	int port = 1000;
	optionParser.getOption("port", port);

	int bufsize = RECV_BUFFER_SIZE;
	optionParser.getOption("bufsize", bufsize);

	std::string filename;
	optionParser.getOption("file", filename);

	double dropRate = 0.01;
	optionParser.getOption("drop", dropRate);

	if (optionParser.getArgCount() == 0)
	{
		printf("need sink address.\n");
		optionParser.usage();
		return 0;
	}

	Application app;
	app.setRecvBufSize(bufsize);
	app.setDumpFile(filename);
	app.setDropRate(dropRate);

	if (!app.open("0.0.0.0", port))
	{
		printf("failed to open on port: %d\n", port);
		return 0;
	}

	for (size_t i = 0; i < optionParser.getArgCount(); ++ i)
	{
		std::string addr = optionParser.getArg(i);
		app.add(addr);
	}

	app.start();

	std::cout << "enter q or quit to exit.\n";
	std::cout << ">";
	std::string line;
	while (true)
	{
		std::getline(std::cin, line);

		if (line == "q" || line == "quit")
		{
			break;
		}
		else if (comn::StringUtil::startsWith(line, "drop"))
		{
			std::string cmd;
			comn::StringUtil::split(line, ' ', cmd, dropRate);
			app.setDropRate(dropRate);
		}
		else
		{
			//
		}

		std::cout << ">";
	}

	app.stop();
	app.close();

	return EXIT_SUCCESS;
}
