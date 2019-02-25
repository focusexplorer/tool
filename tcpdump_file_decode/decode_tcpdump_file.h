
#include<fstream>
#include<string>
#include<sstream>
#include<assert.h>
#include <arpa/inet.h>
using namespace std;
typedef void (*ftype_every_line)(std::string tag,const char*buf,int len);
std::string format_time(int s,int us)
{

    	time_t now=s;
    	struct tm* tm=localtime(&now);
    	static char buf[100];
    	snprintf(buf,sizeof(buf),"%04d%02d%02d-%02d%02d%02d %06d",
    			tm->tm_year+1900,
    			tm->tm_mon+1,
    			tm->tm_mday,
    			tm->tm_hour,
    			tm->tm_min,
    			tm->tm_sec,
    			us);
    	return buf;
}
std::string sip(unsigned int ip)
{
	unsigned char*b=(unsigned char*)&ip;
	char buf[128];
	snprintf(buf,sizeof(buf),"%hhu.%hhu.%hhu.%hhu",b[0],b[1],b[2],b[3]);
	return buf;
}
void run_decode(const char*file_name,ftype_every_line fel)
{
	ifstream infile(file_name);
	if(!infile)
	{
		printf("%s open error\n",file_name);
		return;
	}
	char buf[1024];
	
	//skip head
	infile.read(buf,24);

	while(infile)
	{
		std::stringstream ss;
		int timestamp=0;
		int microsecond=0;
		int plen=0;
		int t=0;
		infile.read((char*)&timestamp,sizeof(timestamp));
		infile.read((char*)&microsecond,sizeof(microsecond));
		infile.read((char*)&plen,sizeof(plen));
		infile.read((char*)&t,sizeof(t));
		if(plen!=t)
		{
			printf("format not as expected\n");
			break;
		}
		ss<<format_time(timestamp,microsecond)<<'\t';
		
		char mih[16+20];
		infile.read(mih,sizeof(mih));
		char uh[8];
		infile.read(uh,sizeof(uh));
		{
			short t=htons(*(short*)(mih+16+2));
			//ss<<t<<'\t';
			//ss<<plen<<'\t';
			assert(t==plen-16);
			unsigned int src_ip=*(unsigned*)(mih+16+12);
			unsigned int dst_ip=*(unsigned*)(mih+16+16);
			unsigned short src_port=*(unsigned short*)(uh);
			unsigned short dst_port=*(unsigned short*)(uh+2);
			t=htons(*(short*)(uh+4));
			assert(t==plen-16-20);
			ss<<sip(src_ip)<<':'<<src_port<<'\t';
			ss<<sip(dst_ip)<<':'<<dst_port<<'\t';
		}
		int datalen=plen-sizeof(mih)-sizeof(uh);
		infile.read(buf,datalen);
		fel(ss.str(),buf,datalen);
	}
}
