#pragma once
#include "net_common.h"
#include "net_safequeue.h"

using asio::ip::tcp;

namespace net
{
	namespace tcp
	{
		template <typename T>
		class connection : public std::enable_shared_from_this<connection<T>>
		{
		public:
			enum class owner : uint8_t
			{
				server, 
				client
			};

			connection(owner parent, asio::io_context& io, asio::ip::tcp::socket socket,
				safe_queue<owned_message<T>>& messages)
				: m_socket(std::move(socket)), m_context(io), m_messagesIn(messages)
			{
				m_ownerType = parent;
			}

			void send(const message<T>& msg)
			{
				asio::post(m_context,
					[this, msg]()
					{
						bool writingMessage = !m_messagesOut.empty();
						m_messagesOut.push_back(msg);
						if (!writingMessage)
						{
							write_header();
						}

					});
			}

			void connect_to_server(const asio::ip::tcp::resolver::results_type& endpoints)
			{
				if (m_ownerType == owner::client)
				{
					asio::async_connect(m_socket, endpoints,
						[this](std::error_code ec, asio::ip::tcp::endpoint)
						{
							if (!ec)
							{
								std::cout << "Connected to server\n";
								read_header();
							}
							else
							{
								std::cout << "[ERROR]: Connect to Server fail.\n";
							}
						});
				}
			}

			void connect_to_client(uint32_t uid = 0)
			{
				if (m_ownerType == owner::server)
				{
					if (m_socket.is_open())
					{
						id = uid;
						read_header();
					}
				}
			}

			uint32_t getId()
			{
				return id;
			}

			void disconnect()
			{
				if (is_connected())
					m_socket.close();
			}	

			bool is_connected()
			{
				return m_socket.is_open();
			}

		private:
			
			void write_header()
			{
				auto self = this->shared_from_this();
				asio::async_write(m_socket,
					asio::buffer(&m_messagesOut.front().header, sizeof(message_header<T>)),
					[this, self](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							if (m_messagesOut.front().size() > 0)
							{
								write_body();
							}
							else
							{
								m_messagesOut.pop_front();
								if(!m_messagesOut.empty())
									write_header();
							}
						}
						else
						{
							std::cout << "[ERROR]: Write header fail.\n";
							m_socket.close();
						}
					});
			}

			void write_body()
			{
				auto self = this->shared_from_this();
				asio::async_write(m_socket,
					asio::buffer(m_messagesOut.front().body.data(), m_messagesOut.front().size()),
					[this, self](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							m_messagesOut.pop_front();
							if (!m_messagesOut.empty())
								write_header();
						}
						else
						{
							std::cout << "[ERROR]: Write body fail.\n";
							m_socket.close();
						}

					});
			}

			void read_header()
			{
				auto self = this->shared_from_this();
				asio::async_read(m_socket,
					asio::buffer(&m_tempMessage.header, sizeof(message_header<T>)),
					[this, self](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							if (m_tempMessage.header.size > 0)
							{
								m_tempMessage.body.resize(m_tempMessage.header.size);
								read_body();
							}
							else
							{
								add_to_incoming_msg();
							}
						}
						else
						{
							std::cout << "[ERROR]: Read header fail.\n";
							m_socket.close();
						}
					});
					
			}

			void read_body()
			{
				auto self = this->shared_from_this();
				asio::async_read(m_socket,
					asio::buffer(m_tempMessage.body.data(),
						m_tempMessage.size()),
					[this, self](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							add_to_incoming_msg();
						}
						else
						{
							std::cout << "[ERROR]: Read body fail.\n";
							m_socket.close();
						}
					});
			}

			void add_to_incoming_msg()
			{
				if (m_ownerType == owner::server)
				{
					std::cout << "Message written at server queue\n";
					m_messagesIn.push_back({this->shared_from_this(), m_tempMessage});
				}
				else
					m_messagesIn.push_back({ nullptr, m_tempMessage });
				read_header();
			}

		protected:
			owner m_ownerType = owner::server;
			asio::ip::tcp::socket m_socket;
			asio::io_context& m_context;

			uint32_t id = 0;
			message<T> m_tempMessage;
			safe_queue<owned_message<T>>& m_messagesIn;
			safe_queue<message<T>> m_messagesOut;
		};
	}

	namespace udp
	{

	}
}