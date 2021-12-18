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
	template<int index, typename Func, typename... Types>
	struct iterate_tuple_insert
	{
		static void next(std::tuple<Types...>& t, Func func, const size_t& line, std::string* args)
		{
			iterate_tuple_insert<index - 1, Func, Types...>::next(t, func, line, args);

			try {
				func(index, std::get<index>(t), line, args);
			}
			catch (std::invalid_argument&)
			{
				std::cerr << "Bad value at (" << line << "," << index + 1 << ")!" << std::endl;
			}
		}
	};

	template<typename Func, typename... Types>
	struct iterate_tuple_insert<0, Func, Types...>
	{
		static void next(std::tuple<Types...>& t, Func func, const size_t& line, std::string* args)
		{
			try {
				func(0, std::get<0>(t), line, args);
			}
			catch (std::invalid_argument&)
			{
				std::cerr << "Bad value at (" << line << "," << 1 << ")!" << std::endl;
			}
		}
	};

	template<typename Func, typename... Types>
	void InsertForEach(std::tuple<Types...>& t, Func func, const size_t& line, std::string* args)
	{
		int const t_size = sizeof...(Types);
		iterate_tuple_insert<t_size - 1, Func, Types...>::next(t, func, line, args);
	}

	struct ConvertValue
	{
		template<typename T>
		void operator()(int index, T&& t, const size_t& line, std::string* args)
		{
			std::istringstream(args[index]) >> t;
			std::ostringstream ss;
			ss << t;
			if (ss.str() != args[index])
			{
				throw std::invalid_argument("");
			}
		}
	};
}

//for printing tuple
namespace
{
	template<int index, typename Func, typename... Types>
	struct iterate_tuple_print
	{
		static void next(std::tuple<Types...>& t, Func func, bool endofline)
		{
			iterate_tuple_print<index - 1, Func, Types...>::next(t, func, false);
			func(index, std::get<index>(t), endofline);
		}
	};

	template<typename Func, typename... Types>
	struct iterate_tuple_print<0, Func, Types...>
	{
		static void next(std::tuple<Types...>& t, Func func, bool endofline)
		{
			func(0, std::get<0>(t), endofline);
		}
	};

	template<typename Func, typename... Types>
	void PrintForEach(std::tuple<Types...>& t, Func func)
	{
		int const t_size = sizeof...(Types);
		iterate_tuple_print<t_size - 1, Func, Types...>::next(t, func, true);
	}

	struct PrintValue
	{
		template<typename T>
		void operator()(int index, T&& t, bool endofline = false)
		{
			if (endofline)
			{
				std::cout << t;
			}
			else
			{
				std::cout << t << ", ";
			}
		}
	};
}

template<typename... Types>
auto operator<<(std::ostream& stream, std::tuple<Types...>& row)
{
	PrintForEach(row, PrintValue());
}

template<typename... Types>
class CSVParser
{
public:
	template<typename ...T>
	struct InputIterator
	{
		using iterator_category = std::input_iterator_tag;
		using value_type = std::tuple<T...>;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;

		using iterator = InputIterator<T...>;
		size_t index = 0;
		CSVParser<T...>* iter;
		InputIterator(CSVParser<T...>* pars) : iter(pars) {}

		InputIterator(const iterator&) = default;
		iterator& operator=(const iterator&) = default;
		~InputIterator() = default;

		friend void swap(iterator& a, iterator& b)
		{
			std::swap(a, b);
		}

		friend bool operator==(const iterator& l, const iterator& r)
		{
			return l.iter == r.iter;
		}
		friend bool operator!=(const iterator& l, const iterator& r)
		{
			return !(l == r);
		}

		reference operator*()
		{
			return iter->data[index];
		}

		pointer operator->()
		{
			return std::addressof(**this);
		}

		iterator& operator++(int)
		{
			if (index < data.size()) index++;
			return *this;
		}

	};

	InputIterator<Types...> begin(CSVParser<Types...>& pars)
	{
		InputIterator<Types...> it(0);
		return it;
	}

	InputIterator<Types...> end(CSVParser<Types...>& pars)
	{
		InputIterator<Types...> it(pars.data.size());
		return it;
	}

	CSVParser() 
	{
		data.resize(0);
	}

	CSVParser(std::istream& input)
	{
		data.resize(0);
		Pars(input, 0);
	}

	CSVParser(std::istream& input, const size_t& skip_count)
	{
		data.resize(0);
		Pars(input, skip_count);
	}

	void Pars(std::istream& input, const size_t& skip_count)
	{
		std::string line;
		for (int i = 0; i < skip_count; ++i)
		{
			std::getline(input, line, row_div);
		}
		size_t line_count = 0;
		while (std::getline(input, line, row_div))
		{
			data.resize(line_count + 1);
			std::string* args = new std::string[sizeof...(Types)];
			for (size_t i = 0; i < sizeof...(Types); ++i)
			{
				size_t pos = line.find(column_div);
				std::string arg = line.substr(0, pos);
				args[i] = arg;
				line.erase(0, pos + 1);
			}
			InsertForEach(data[line_count], ConvertValue(), line_count + 1, args);
			line_count++;
		}
	}

	void SetDividers(char row_divider, char column_divider) 
	{
		row_div = row_divider;
		column_div = column_divider;
	}

	std::tuple<Types...>& GetRow(const size_t& index)
	{
		return data[index];
	}

	friend auto operator<<(std::ostream& stream, std::tuple<Types...>& row);
	
private:
	std::vector<std::tuple<Types...>> data;
	char row_div = '\n';
	char column_div = ',';

	
};

