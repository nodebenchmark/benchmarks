#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
 * Calculate the percentage of instructions that event j used was used in event i (i < j)
 * Read from footprint.out.inst and mapping.out.final from each application directory
 * */

using namespace std;

typedef enum 
{
    V8,
	NATIVE,
	OTHERS
} KERNEL;

int main(int argc, char **argv)
{
	if(argc != 4)
	{
		cout << "Usage: ./func footprint.inst mapping mode\n";
		exit(1);
	}

	char *footprint = argv[1];
	std::ifstream fpfile(footprint);
	char *mapping = argv[2];
	std::ifstream mapfile(mapping);
	std::ofstream heatmap;
	KERNEL mode;
	if(!strcmp(argv[3], "v8"))
	{
  		heatmap.open("reuse.v8.csv");
		mode = V8;
	}
	else if(!strcmp(argv[3], "native"))
	{
  		heatmap.open("reuse.native.csv");
		mode = NATIVE;
	}
	else
	{
		cout << "Wrong reuse mode\n";
		exit(1);
	}

	std::string line;
	map<uint64_t, string> funcMap;
	while (std::getline(mapfile, line))
	{
		uint64_t addr;
		string func_name;
		std::stringstream ss(line);
    	std::string item;

		int item_num = -1;
    	while (std::getline(ss, item, '\t'))
		{
			item_num++;
			if(item_num == 0) addr = stoull(item);
			else if(item_num == 1) func_name = item;
			else exit;
		}

		funcMap[addr] = func_name;
	}
	cout << "Finish reading mapping file" << endl;

	int line_num = -1;
	vector < vector<int> > events;
	while (std::getline(fpfile, line))
	{
		std::stringstream ss(line);
    	std::string item;
		int item_num = -1;
		KERNEL kernel;

		line_num++;
    	while (std::getline(ss, item, ' '))
		{
			item_num++;
			if(item_num > 101) break;
			if(item_num == 0)
			{
				if(line_num != 0)
				{
					uint64_t addr = stoull(item);
					string f_name = funcMap[addr];
					if(f_name.compare("v8") == 0) kernel = V8;
					else if(f_name.compare("invalid_rtn") == 0) kernel = NATIVE;
					else kernel = OTHERS;

					if(kernel != mode) break;
				}
			}
			else
			{
				if(line_num == 0)
				{
					vector<int> event;
					events.push_back(event);
				}
				else events[item_num - 1].push_back(stoi(item));
			}
    	}

		if(line_num % 100000 == 0) cout << line_num << endl;
	}

	int tot_events = events.size();
	int tot_insts = events[0].size();
	cout << "Finished parsing " << tot_events << " events" << endl;
	cout << tot_insts << " instruction per event" << endl;

	heatmap << "e_0";
	for(int i = 1;i < tot_events;i++)
	{
		heatmap << ",e_" << i;
	}
	heatmap << endl;

	for(int i = 0;i < tot_events;i++)
	{
		// handeling j == 0 case specially for formatting purpose
		if(i == 0) heatmap << "1";
		else heatmap << "0";

		for(int j = 1;j < tot_events;j++)
		{
			if(j % 100 == 0) cout << i << " " << j << endl;

			if(i > j) heatmap << ",0";
			else if(i == j) heatmap << ",1";
			else
			{
				int static_matched_insts = 0;
				int static_used_insts = 0;
				unsigned long int dynamic_matched_insts = 0;
				unsigned long int dynamic_used_insts = 0;
				for(int k = 0; k < tot_insts;k++)
				{
					if(events[j][k] != 0)
					{
						static_used_insts++;
						dynamic_used_insts += events[j][k];
						if(events[i][k] != 0)
						{
							static_matched_insts++;
							dynamic_matched_insts += events[j][k];
						}
					}
				}
				heatmap << "," << (long double)dynamic_matched_insts / (long double)dynamic_used_insts;
			}
		}
		heatmap << endl;
	}

  	heatmap.close();
}
