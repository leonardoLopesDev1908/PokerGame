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
			enum owner
			{
				server, 
				client
			};

		public:
			connection(owner parent, asio::io_context& io, tcp::socket socket,
				safe_queue<owned_message<T> messages)
				: m_socket(std::move(socket)), m_context(io), m_messagesIn(message)
			{
				ownerType = parent;
			}

			void send(const message<T>& msg)
			{
				asio::post(m_context,
					[this, msg]()
					{
						bool writingMessage = !m_messagesOut.empty();
						m_messagesOut.push_back(msg);
						if (!writingMessage)
							write_header();

					});
			}

			void connect_to_server(const tcp::resolver::results_type& endpoints)
			{
				asio::async_connect(m_socket, endpoints,
					[this](std::error_code ec, tcp::endpoints)
					{
						if (!ec)
						{
							write_header();
						}
					});
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
			
			void read_header()
			{
				auto self = shared_from_this();
				asio::async_read(m_socket,
					asio::buffer(m_tempMessage.header, m_tempMessage.header.size),
					[this, self](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							if (length > 0)
							{
								m_messagesIn.push_back(m_tempMessage);
								read_body();
							}
						}
						else
						{
							m_socket.close();
						}
					});
					
			}

			void read_body()
			{
				auto self = shared_from_this();
				async::read(m_socket,
					asio::buffer(m_messagesIn.front().message.body.data(),
						m_messagesIn.front().message.size()),
					[this, self](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							m_messagesIn.pop_front();
							read_header();
						}
						else
						{
							m_socket.close();
						}
					});
			}

			void write_header()
			{
				auto self = shared_from_this();
				asio::async_write(m_socket,
					asio::buffer(m_messagesOut.front().message.header,
						m_messagesOut.front().message.header.size),
					[this, self](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							write_body();
						}
						else
						{
							m_socket.close();
						}
					});
			}

			void write_body()
			{
				auto self = shared_from_this();
				asio::async_write(m_socket,
					asio::buffer(m_messagesOut.front().body.data(), m_messagesOut.front().size()),
					[this, self](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							m_messagesOut.pop_front();
							write_header();
						}
						else
						{
							m_socket.close();
						}

					});
			}

		protected:
			owner m_ownerType = owner::server;
			tcp::socket m_socket;
			asio::io_context& m_context;

			uint32_t id = 0;
			owned_message<T> m_tempMessage;
			safe_queue<message<T>> m_messagesOut;
			safe_queue<owned_message<T>> m_messagesIn;
		};
	}

	namespace udp
	{

	}
}