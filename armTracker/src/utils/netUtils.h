#pragma once

#include "utils.h"


static string getBroadcastIp(){

	startRedirectConsoleToFile("ip.txt");		
	system("ifconfig |grep inet");
	endRedirectConsoleToFile();

	ofBuffer buf = ofBufferFromFile("ip.txt", false);
	string str = buf.getText();
	
	vector <string> results = ofSplitString(str, "broadcast", true, true);
	if( results.size() >= 2){
		return results[1];
	}
	
	return "127.0.0.1";
}

