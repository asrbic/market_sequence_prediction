#include "btf.h"

#include "boost/filesystem.hpp"
#include <fstream>

using namespace boost::filesystem;

#define LONG 1
#define SHORT -1
#define HOLD 0

BTF::BTF(string *dirPath)
{
	this->dirPath = dirPath;
}

void BTF::readCSVs(vector<BTF::HLOCV> &histData, string &dirPath)
{

	path p (dirPath.c_str());

	directory_iterator end_itr;

	// cycle through the directory
	for (directory_iterator itr(p); itr != end_itr; ++itr)
	{
		// If it's not a directory, list it. If you want to list directories too, just remove this check.
		if (is_regular_file(itr->path())) {
			// assign current file name to current_file and echo it out to the console.
			string current_file = itr->path().string();
			cout << current_file << endl;
			readCSV(histData, current_file);

		}
	}



}

void BTF::readCSV(vector<BTF::HLOCV> &histData, string &path) {
	ifstream inFile;
	inFile.open(path.c_str());
	std::string line;
	while(std::getline(inFile, line))
	{
		stringstream lineStream(line);
		string symbol;
		string date;
		string open;
		string high;
		string low;
		string close;
		string volume;

		getline(lineStream,symbol,',');
		getline(lineStream,date,',');
		getline(lineStream,open,',');
		getline(lineStream,high,',');
		getline(lineStream,low,',');
		getline(lineStream,close,',');
		getline(lineStream,volume,',');

		HLOCV rec;
		memcpy(&rec.symbol, symbol.c_str(), sizeof(char)*symbol.size());
		memcpy(&rec.date, date.c_str(), sizeof(char)*date.size());
		sscanf(high.c_str(), "%lf", &rec.high);
		sscanf(low.c_str(), "%lf", &rec.low);
		sscanf(open.c_str(), "%lf", &rec.open);
		sscanf(close.c_str(), "%lf", &rec.close);
		sscanf(volume.c_str(), "%lf", &rec.volume);
		cout << rec.toString();
		histData.push_back(rec);
		/*while(getline(lineStream,cell,','))
		{
			result.push_back(cell);
		}*/
	}
	inFile.close();
}

void BTF::runBackTest()
{
	vector<BTF::HLOCV> histData;

	readCSVs(histData, *this->dirPath);

	delete &histData;
}






