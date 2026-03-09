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
			explicit server(uint16_t port)
				: m_acceptor(m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
			{
			}

			virtual ~server()
			{
				stop();
			}

			bool start()
			{
				try{
					wait_client_connect();
					m_threadContext = std::thread([this] { m_context.run(); });
				}
				catch (std::exception& e)
				{
					std::cerr << "[ERROR] " << e.what() << '\n';
					return false;
				}
				return true;
			}

			void wait_client_connect()
			{
				std::cout << "Waiting for a client connection...\n";
				m_acceptor.async_accept(
					[this](std::error_code ec, asio::ip::tcp::socket socket)
					{
						if (!ec)
						{
							auto newconn = std::make_shared<connection<T>>(
								connection<T>::owner::server, m_context, std::move(socket),
								m_messages
							);

							newconn->connect_to_client(m_idCounter++);
							m_connections.push_back(std::move(newconn));
							
							on_client_connect();
						}
						else
						{
							std::cout << "[ERROR] Server new connections failed\n";
							wait_client_connect();
						}
					});
			}

			void stop()
			{
				m_context.stop();

				if (m_threadContext.joinable())
					m_threadContext.join();

				for (auto& c : m_connections)
				{
					c->is_connected();
				}

				m_connections.clear();
			}
		
			void update()
			{
				while (!m_messages.empty())
				{
					auto msg = m_messages.pop_front();
					on_message(msg.message, msg.remote);
				}
			}

			void message_all(const message<T>& msg, std::shared_ptr<connection<T>> ignoreClient = nullptr)
			{
				message<T> msgAll;

				if (ignoreClient)
					msgAll << "[" << ignoreClient->getId() << "]: " << msg;
				else
					msgAll << msg;

				bool bInvalidClientExists = false;
				for (auto& c : m_connections)
				{
					if (c && c->is_connected())
					{
						if (c != ignoreClient)
							c->send(msgAll);
					}
					else
					{
						c.reset();
						bInvalidClientExists = true;
					}
				}

				if (bInvalidClientExists)
				{
					m_connections.erase(
						std::remove(m_connections.begin(), m_connections.end(), nullptr),
						m_connections.end()
					);
				}
			}

			void message_client(const message<T>& msg, std::shared_ptr<connection<T>> client)
			{
				if (client->is_connected())
				{
					client->send(msg);
				}
				else
				{
					client.reset();
					m_connections.erase(
						std::remove(m_connections.begin(), m_connections.end(), client)
					);
				}
			}

		protected:
			virtual void on_message(message<T>& msg, std::shared_ptr<connection<T>> remote)
			{
			}

			virtual void on_client_connect()
			{
				wait_client_connect();
			}

			virtual void on_client_disconnect()
			{
			}

		protected:
			std::vector<std::shared_ptr<connection<T>>> m_connections;

		private:
			asio::io_context m_context;
			asio::ip::tcp::acceptor m_acceptor;
			std::thread m_threadContext;

			uint64_t m_idCounter = 1;
			safe_queue<owned_message<T>> m_messages;
		};
	}
}