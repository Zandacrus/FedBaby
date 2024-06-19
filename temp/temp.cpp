#include <iostream>
#include <vector>
#include <type_traits>

using namespace std;

template <typename T> constexpr bool isVector(const std::vector<T>&)
{
	return true;
};

template <typename T> constexpr bool isVector(const T&)
{
	return false;
};

template <typename T> void printVector(std::ostream& os, const std::vector<T>& vec, const int tabCount = 0)
{
	T temp;
	
	if constexpr (isVector(temp))
	{
		int tabs;
		
		os << '{';
		
		for(const T& elem : vec)
		{
			os << '\n';
			
			tabs = tabCount+1;
			while (tabs--) os << '\t';
			
			printVector(os, elem, tabCount+1);
			
			os << ',';
		}
		
		os << '\n';
		
		tabs = tabCount;
		while (tabs--) os << '\t';
		
		os << '}';
	}
	else
	{
		os << '{';
		
		for(const T& elem : vec)
		{
			os << ' ' << elem << ',';
		}
		
		os << '}';
	}
}

template <typename T> inline std::ostream& operator<<(std::ostream& os, const std::vector<T>& obj)
{
	printVector(os, obj);
	
	return os;
}

template <typename T> inline std::ostream& operator<<(std::ostream& os, const std::vector<T>&& obj)
{
	printVector(os, obj);
	
	return os;
}

int main()
{
	vector<vector<int>> seq1 = {{1, 2, 3}, {3, 4, 5}};
	vector<int> seq2 = {1, 2, 3};
	
	std::cout << "seq1 = " << seq1;
	std::cout << '\n';
	
	seq1.resize(seq2.size());
	for(int i=0; i<seq1.size(); i++) seq1[i].resize(seq2[i]);
	
	printVector(std::cout, seq1);
	std::cout << '\n';
	
	return sizeof(std::remove_cv_t<std::remove_reference_t<decltype(seq2)>>::value_type);
}