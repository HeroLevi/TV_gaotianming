#include "TCPKernel.h"
#include <iostream>
using namespace std;
int main()
{
	
	IKernel *pKernel = new TCPKernel;
	if(!pKernel->Open())
	{
		cout<<"open fail"<<endl;
		system("pause");
	    return 0;
	}
	cout<<"Server is running......"<<endl;
	
	system("pause");
	return 0;
}