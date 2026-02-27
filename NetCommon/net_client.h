#pragma once
#include "net_connection.h"

using asio::ip::tcp;

namespace net
{
	namespace tcp
	{
		template <typename T>
		class client
		{
		public:

			client()
			{}

			connect(const std::string& host, const uint16_t port)
			{
				tcp::resolver resolver(m_context);
				tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

				m_connection = std::make_shared<connection<T>>(
					owner::client, m_context,
					std::move(m_socket))
					);
					m_connection->connect_to_server(endpoints);

				m_threadContext = std::thread([this] {m_context.run();});
			}

			void send(const message<T>& msg)
			{
				m_connection->send(msg);
			}

		protected:

			void disconnect()
			{
				m_connection->disconnect();
				if (m_threadContext.joinable())
					m_threadContext.join();
			}

			void is_connected()
			{
				m_connection.is_connected();
			}

		private:
			tcp::socket m_socket;
			tcp::resolver::results_type& endpoints;
			asio::io_context m_context;

			std::thread m_threadContext;
			std::shared_ptr<connection<T>> m_connection;
		};
	}

	namespace udp
	{

	}

}