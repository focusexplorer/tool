#include<iostream>
#include"decode_tcpdump_file.h"
using namespace std;
void line(std::string tag,const char *buf,int len)
{
	cout<<tag<<endl;
}
int main()
{
	run_decode("./t.cap",line);
	return 0;
}
