#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>
#include <stdlib.h>
#include <string.h>

/*
 * Calculate the percentage of instructions that event j used was used in event i (i < j)
 * Read from footprint.out.event from each application directory
 * */

using namespace std;

typedef enum 
{
    DYNAMIC,
    STATIC
} REUSE_MODE;

int main(int argc, char **argv)
{
	if(argc != 3)
	{
		cout << "Usage: ./reuse footprint mode\n";
		exit(1);
	}

	char *footprint = argv[1];
	std::ifstream infile(footprint);
	std::ofstream heatmap;
	REUSE_MODE mode;
	if(!strcmp(argv[2], "dynamic"))
	{
  		heatmap.open("reuse.dynamic.csv");
		mode = DYNAMIC;
	}
	else if(!strcmp(argv[2], "static"))
	{
  		heatmap.open("reuse.static.csv");
		mode = STATIC;
	}
	else
	{
		cout << "Wrong reuse mode\n";
		exit(1);
	}

	int line_num = 0;
	std::string line;
	vector < vector<int> > events;
	while (std::getline(infile, line))
	{
		if(line_num++ == 0) continue;
		//if(line_num < 301 || line_num >= 306) continue;
		if(line_num > 101) break;
		if(line_num % 100 == 0) cout << line_num << endl;

		vector<int> event;
		std::stringstream ss(line);
    	std::string item;

		int item_num = 0;
    	while (std::getline(ss, item, ' ')) {
			if(item_num++ == 0) continue;
    	    event.push_back(stoi(item));
    	}

		events.push_back(event);
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
				if(mode == STATIC) heatmap << "," << (double)static_matched_insts / (double)static_used_insts;
				else heatmap << "," << (long double)dynamic_matched_insts / (long double)dynamic_used_insts;
			}
		}
		heatmap << endl;
	}

  	heatmap.close();
}
