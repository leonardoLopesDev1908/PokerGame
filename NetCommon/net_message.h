#pragma once
#include "net_common.h"

namespace net
{
	namespace tcp
	{
		template <typename T>
		struct message_header
		{
			T id{};
			uint32_t size = 0;
		};

		template <typename T>
		struct message
		{
			message_header<T> header;
			std::vector<char> body;

			size_t size() const
			{
				return body.size();
			}

			friend std::ostream& operator << (std::ostream& os, const message<T>& data)
			{
				os << "ID: " << data.header.id << " Size: " << data.header.size << "\n";
				return os;
			}

			template <typename DataType>
			friend message<T>& operator <<(message<T>& msg, const DataType& data)
			{
				static_assert(std::is_standard_layout<DataType>::value, "Invalid data type\n");

				size_t i = msg.body.size();
				msg.body.resize(i + sizeof(DataType));

				std::memcpy(msg.body.data() + i, &data, sizeof(DataType));
				
				msg.header.size = msg.size();

				return msg;
			}

			friend message<T>& operator <<(message<T>& msg, const std::string& data)
			{
				msg << (uint32_t)data.size();
				size_t i = msg.body.size();

				msg.body.resize(i + data.size());
				std::memcpy(msg.body.data() + i, data.data(), data.size());

				msg.header.size = msg.size();
				return msg;
			}

			friend message<T>& operator <<(message<T>& msg, const message<T>& other)
			{
				size_t i = other.size() + msg.size();
				size_t j = msg.size();

				msg.body.resize(i);
				std::memcpy(msg.body.data() + j, other.body.data(), other.size());
				
				msg.header.size = msg.size();
				return msg;
			}


			template <typename DataType>
			friend message<T>& operator >>(message<T>& msg, DataType& data)
			{
				static_assert(std::is_standard_layout<DataType>::value, "Invalid data type\n");

				size_t i = msg.body.size() - sizeof(DataType);

				std::memcpy(&data, msg.body.data() + i, sizeof(DataType));
				msg.body.resize(i);

				msg.header.size = msg.size();
				return msg;
			}

			friend message<T>& operator>> (message<T>& msg, std::string& data)
			{
				uint32_t size = 0;
				msg >> size;
				data.resize(size);

				size_t i = msg.body.size() - size;
				std::memcpy(data.data(), msg.body.data() + i, size);

				msg.body.resize(i);
				msg.header.size = msg.size();
				return msg;
			}
		};

		template <typename T>
		class connection;

		template <typename T>
		struct owned_message
		{
			std::shared_ptr<connection<T>> remote = nullptr;
			message<T> message;
		};
	}

	namespace udp
	{

	}
	
}