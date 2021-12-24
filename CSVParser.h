#pragma once
#include <iostream>
#include <vector>
#include <exception>
#include <tuple>
#include <string>
#include <sstream>
#include <type_traits>
#include <iterator>
#include <utility>

//for inserting values in tuple
namespace
{
	template<int index, typename... Types>
	struct iterate_tuple_insert
	{
		static void next(std::tuple<Types...>& t, const size_t& line, std::string* args)
		{
			iterate_tuple_insert<index - 1, Types...>::next(t, line, args);

			std::get<index>(t) = ConvertValue<std::get<index>(t)>::convert(std::get<index>(t), index, line, args);
		}
	};

	template<typename... Types>
	struct iterate_tuple_insert<0, Types...>
	{
		static void next(std::tuple<Types...>& t, const size_t& line, std::string* args)
		{
			std::get<0>(t) = ConvertValue<std::get<0>(t)>::convert(std::get<0>(t), 0, line, args);
		}
	};

	template<typename... Types>
	void InsertForEach(std::tuple<Types...>& t, const size_t& line, std::string* args)
	{
		int const t_size = sizeof...(Types);
		iterate_tuple_insert<t_size - 1, Types...>::next(t, line, args);
	}

	template<typename T>
	struct ConvertValue
	{
	public:
		static T convert(T elem, int index, const size_t& line, std::string* args)
		{
			extern T t;
			std::istringstream(args[index]) >> t;
			std::ostringstream ss;
			ss << t;
			if (ss.str() != args[index])
			{
				std::string message = "Bad value at (";
				message += std::to_string(line);
				message += ",";
				message += std::to_string(index + 1);
				message += ")!";
				std::cerr << message;
				throw std::invalid_argument(message.c_str());
			}
			return t;
		}
	};

	template<>
	struct ConvertValue<std::string>
	{
	public:
		static std::string convert(std::string elem, int index, const size_t& line, std::string* args)
		{
			return args[index];
		}
	};
}

//for printing tuple
namespace
{
	template<int index, typename Func, typename... Types>
	struct iterate_tuple_print
	{
		static void next(std::ostream& stream, std::tuple<Types...>& t, Func func, bool endofline)
		{
			iterate_tuple_print<index - 1, Func, Types...>::next(stream, t, func, false);
			func(stream, std::get<index>(t), endofline);
		}
	};

	template<typename Func, typename... Types>
	struct iterate_tuple_print<0, Func, Types...>
	{
		static void next(std::ostream& stream, std::tuple<Types...>& t, Func func, bool endofline)
		{
			func(stream, std::get<0>(t), endofline);
		}
	};

	template<typename Func, typename... Types>
	void PrintForEach(std::ostream& stream, std::tuple<Types...>& t, Func func)
	{
		int const t_size = sizeof...(Types);
		iterate_tuple_print<t_size - 1, Func, Types...>::next(stream, t, func, true);
	}

	struct PrintValue
	{
		template<typename T>
		void operator()(std::ostream& stream, T&& t, bool endofline = false)
		{
			if (endofline)
			{
				stream << t;
			}
			else
			{
				stream << t << ", ";
			}
		}
	};
}

template<typename... Types>
auto operator<<(std::ostream& stream, std::tuple<Types...>& row)
{
	PrintForEach(stream, row, PrintValue());
}

template<typename... Types>
class CSVParser
{
public:
	template<typename ...T>
	class InputIterator
	{
	public:
		using iterator_category = std::input_iterator_tag;
		using value_type = std::tuple<T...>;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator = InputIterator<T...>;
		
		InputIterator(CSVParser<T...>* pars, const long long& line_number)
		{
			index = line_number;
			iter = pars;
			file_position = iter->file_position;
			Parsline();
		}

		InputIterator(const iterator& it)
		{
			this->index = it.index;
			this->file_position = it.file_position;
			this->item = it.item;
			this->iter = it.iter;
		}

		iterator& operator=(const iterator& it)
		{
			this->index = it.index;
			this->file_position = it.file_position;
			this->item = it.item;
			this->iter = it.iter;
		}

		~InputIterator() = default;

		void Parsline()
		{
			iter->stream->seekg(file_position);
			std::string line;
			if (std::getline(*iter->stream, line, iter->row_div))
			{
				file_position = iter->stream->tellg();
			}
			else
			{
				index = -1;
			}

			std::string* args = new std::string[sizeof...(Types)];
			for (size_t i = 0; i < sizeof...(Types); ++i)
			{
				size_t pos = line.find(iter->column_div);
				std::string arg = line.substr(0, pos);
				args[i] = arg;
				line.erase(0, pos + 1);
			}
			InsertForEach(item, index + 1, args);
		}

		friend void swap(iterator& a, iterator& b)
		{
			std::swap(a, b);
		}

		friend bool operator==(iterator& l, iterator& r)
		{
			return l.iter == r.iter && l.index == r.index && l.file_position == r.file_position && l.index == r.index;
		}

		friend bool operator!=(const iterator& l, const iterator& r)
		{
			return !(l == r);
		}

		reference operator*()
		{
			return item;
		}

		pointer operator->()
		{
			return &item;
		}

		iterator& operator++()
		{
			if (index != -1)
			{
				index++;
				Parsline();
			}
			return *this;
		}

	private:
		long long index = 0;
		size_t file_position;
		value_type item;
		CSVParser<T...>* iter;

	};

	InputIterator<Types...> begin()
	{
		InputIterator<Types...> it(this, 0);
		return it;
	}

	InputIterator<Types...> end()
	{
		InputIterator<Types...> it(this, -1);
		return it;
	}

	CSVParser() {}

	CSVParser(std::istream& input_)
	{
		stream = &input_;
		file_position = 0;
	}

	CSVParser(std::istream& input, const size_t& skip_count)
	{
		std::string line;
		for (int i = 0; i < skip_count; ++i)
		{
			std::getline(input, line, row_div);
		}
		stream = &input;
		file_position = input.tellg();
	}

	void SetDividers(char row_divider, char column_divider) 
	{
		row_div = row_divider;
		column_div = column_divider;
	}

	friend auto operator<<(std::ostream& stream, std::tuple<Types...>& row);
	
private:
	size_t file_position;
	char row_div = '\n';
	char column_div = ',';
	std::istream* stream;

	
};

