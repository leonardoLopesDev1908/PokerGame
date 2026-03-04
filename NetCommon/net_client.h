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
					std::cout << "Trying to connect...\n";

					asio::ip::tcp::resolver resolver(m_context);
					auto endpoints = resolver.resolve(host, std::to_string(port));

					m_connection = std::make_shared<connection<T>>(
						connection<T>::owner::client, m_context,
						asio::ip::tcp::socket(m_context), m_messages
					);

					m_connection->connect_to_server(endpoints);

					m_threadContext = std::thread([this] {m_context.run();});
				}
				catch (std::exception& e)
				{
					std::cerr << "[ERROR]: " << e.what() << '\n';
				}
			}

			void send(const message<T>& msg)
			{
				if (is_connected())
				{
					m_connection->send(msg);
				}
			}

			bool is_connected()
			{
				if(m_connection)
					return m_connection->is_connected();
				return false;
			}

			void disconnect()
			{
				m_context.stop();

				if (m_threadContext.joinable())
					m_threadContext.join();

				if(is_connected())
					m_connection->disconnect();

				m_connection.reset();
			}


			safe_queue<owned_message<T>>& incoming()
			{
				return m_messages;
			}

		protected:
			asio::io_context m_context;
			std::thread m_threadContext;
			std::shared_ptr<connection<T>> m_connection;
		
		private:
			safe_queue<owned_message<T>> m_messages;
		};
	}

	namespace udp
	{

	}

}