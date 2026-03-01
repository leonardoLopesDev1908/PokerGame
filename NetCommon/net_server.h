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
				: m_acceptor(asio::io_context)
			{}
			
			void wait_client_connect()
			{
				m_acceptor.async_accept(
					[this](std::error_code ec, asio::ip::tcp::socket socket)
					{
						if (!ec)
						{
							auto newconn = std::make_shared<connection<T>>(
								connection::owner::server, m_context, std::move(socket),
								m_messages
							);
							m_connections.push_back(newconn);
						}
					});
			}

		private:


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
			asio::ip::tcp::acceptor m_acceptor;
			std::vector<std::shared_ptr<connection<T>>> m_connections;
		};
	}
}