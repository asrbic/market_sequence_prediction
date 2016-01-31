#ifndef BTF_H
#define BTF_H
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

class BTF
{
public:
	BTF(std::string *dirPath);
	void runBackTest();

	struct Position {
		char symbol[4];
		int date;
		double price;
		int volume;
		int direction;
	};
	struct HLOCV {
		char symbol[5];
		char date[9];
		double high;
		double low;
		double open;
		double close;
		double volume;

		HLOCV(char &symbol,
			char &date,
			double high,
			double low,
			double open,
			double close,
			double volume)
		{
			*this->symbol=symbol;
			*this->date=date;
			this->high=high;
			this->low=low;
			this->open=open;
			this->close=close;
			this->volume=volume;
		}
		HLOCV(){}

		std::string toString()
		{
			stringstream asdf;
			asdf << symbol << " "
				<< date << " "
				<< high << " "
				<< low << " "
				<< open << " "
				<< close << " "
				<< volume << endl;
			return asdf.str();
		}
	};

private:
	void readCSVs(std::vector<HLOCV> &histData, std::string &dirPath);
	void readCSV(std::vector<HLOCV> &histData, std::string &path);
	std::string *dirPath;

};

#endif // BTF_H
