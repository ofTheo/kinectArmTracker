#pragma once
#include "ofMain.h"

static fpos_t f_pos;
static int fd;

static void startRedirectConsoleToFile(string filename){
	fgetpos(stdout, &f_pos);
	fd = dup(fileno(stdout));			
	freopen (ofToDataPath(filename).c_str(), "w", stdout);
}

static void endRedirectConsoleToFile(){
	fflush(stdout);
	dup2(fd, fileno(stdout));
	close(fd);
	fsetpos(stdout, &f_pos);
}
