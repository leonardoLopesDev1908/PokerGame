#pragma once
#include "net_connection.h"

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

			virtual ~client()
			{
				disconnect();
			}

			void connect(const std::string& host, const uint16_t port)
			{
				try {
					asio::ip::tcp::resolver resolver(m_context);
					asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

					m_connection = std::make_shared<connection<T>>(
						owner::client, m_context,
						std::move(m_socket)
					);

					m_connection->connect_to_server(endpoints);

					m_threadContext = std::thread([this] {m_context.run();});
				}
				catch (std::exception& e)
				{
					std::cerr < "[ERROR]: " << e.what() << '\n';
				}
			}

			void send(const message<T>& msg)
			{
				m_connection->send(msg);
			}

		protected:

			void disconnect()
			{
				if(is_connected())
					m_connection->disconnect();

				if (m_threadContext.joinable())
					m_threadContext.join();

				m_context.stop();

				m_connection.reset();
			}

			void is_connected()
			{
				m_connection.is_connected();
			}

		private:
			asio::ip::tcp::socket m_socket;
			asio::ip::tcp::resolver::results_type& endpoints;
			asio::io_context& m_context;

			std::thread m_threadContext;
			std::shared_ptr<connection<T>> m_connection;
		};
	}

	namespace udp
	{

	}

}