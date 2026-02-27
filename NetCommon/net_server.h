#pragma once
#include "net_connection.h"

using asio::ip::tcp;

namespace net
{
	namespace tcp
	{
		template <typename T>
		class server
		{
		public:
			server()
			{}
			
			void wait_client_connect()
			{
				m_acceptor.async_accept(
					[this](std::error_code ec, tcp::socket socket)
					{
						if (!ec)
						{
							m_connections.push_back(
								std::make_shared<connection<T>>(
									owner::server, m_context, std::move(socket),
									m_messages
								)
							);
							read_header();
						}
					}
				)
			}

		private:

			void read_header()
			{

			}

			void read_body()
			{

			}

			void write_header()
			{

			}

			void write_body()
			{

			}


		protected:

			void on_message()
			{
			}
			
			void message_all()
			{
			}

			void message_client()
			{
			}

			void update()
			{
			}

		private:
			asio::io_context m_context;

			safe_queue<owned_message<T>> m_messages;
			tcp::acceptor m_acceptor;
			std::vector <std::shared_ptr<connection<T>> m_connections;
		};
	}
}