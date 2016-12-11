#include "Connection.h"
#include <exception>


Connection::Connection(const std::string& other_url)
	: url(other_url)
{
	host = get_host(url);
	path = get_path(url);
	std::cout << "Connecting to " << url << '\n';
	socket.expires_from_now(std::chrono::milliseconds(10000));
	resolve_connection();
	
	/* Flush out the response header */
	std::string tmp;
	while (std::getline(socket, tmp) && tmp != "\r")
	{
	}
	stream << socket.rdbuf();
}


Connection::~Connection()
{
	socket.flush();
	socket.close();
}

asio::io_service Connection::_io_service;

/* Return 0 if there is a stream to be read */
bool Connection::has_stream()
{
	return (stream.rdbuf()->in_avail());
}

/* Move stream */
std::stringstream Connection::get_stream()
{
	return std::move(stream);
}

std::string inline Connection::get_path(const std::string &in)
{
	auto path = strip_http(in);
	path = path.substr(get_host(path).length());
	if (path.empty())
	{
		path = "/";
	}
	return path;
}

std::string inline Connection::get_host(const std::string &in)
{
	auto host = strip_http(in);
	if (host.find("/"))
	{
		return host.substr(0, host.find("/"));
	}
	else return host;
}

std::string inline Connection::strip_http(const std::string &in) 
{
	std::string url(in);
	if (url.compare(0, 7, "http://") == 0)
	{
		return url.substr(7, url.length());
	}
	else if (url.compare(0, 8, "https://") == 0)
	{
		return url.substr(8, url.length());
	}
	else
	{
		return url;
	}
}


void Connection::send_get()
{
	socket
		<< "GET " << path << " HTTP/1.0\r\n"
		<< "Host: " << host << " \r\n"
		<< "User-Agent: SpideR \r\n"
		<< "Accept: */* \r\n"
		<< "Connection: close\r\n\r\n";
}

/* If the response is a redirect, try to find the new url */
void Connection::resolve_connection()
{
	for (int redirects = 0; redirects < 10; redirects++)
	{
		socket.connect(host, "http");
		send_get();
		std::string http_version;
		unsigned int status_code;
		socket >> http_version;	
		socket >> status_code;
		switch (status_code)
		{
			/* Succestream */
			case 200:
			case 201:
			case 202:
			{
				std::cout << "Connected.\n";
				return;
			}
			
			/* Redirection */
			case 300:
			case 301:
			case 302:
			case 307:
			case 308:
			{
				std::string tmp;
				while (std::getline(socket, tmp) && tmp != "\r")
				{
					if (tmp.find("Location: ") != std::string::npos ||
							tmp.find("a href=") != std::string::npos ||
							tmp.find("url=") != std::string::npos)
					{
						auto pos = tmp.find("http");
						if (pos != std::string::npos)
						{
							url =  tmp.substr(pos);
							host = get_host(url);
							path = get_path(url);
						}
						else if ((pos = tmp.find("/")) && pos != std::string::npos)
						{
							path = tmp.substr(pos);
							url = host + path;
						}
						std::cout << "Redirected to " << url << " ...\n";
						break;
					}
				}
				socket.flush();
				socket.close();
				break;
			}
			
			default:
			{
				std::cout << "Failed..\n";
				return;
			}
		}
	}
}

