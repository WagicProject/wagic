// ZFS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


// ZFS headers
#include "zfsystem.h"



void DoSomething(std::istream & File)
{
	// Output the file via cout (note: rdbuf() method is a std C++ method, not zfs specific)
	std::cout << File.rdbuf() << std::endl;
}



int main(int argc, char * argv[])
{
	using namespace std;
	using zip_file_system::filesystem;
	using zip_file_system::izfstream;

	// Create and initialize the Zip File System (basepath, file_extension, makedefault)
	// and output the its status via cout
	filesystem FileSystem("base_data", "cpk", true);
	cout << FileSystem << endl;

	// Try to open a zipped file (Careful! The openmode is always 'ios::in | ios::binary'.)
	izfstream File("testfile.txt");

	if (! File)
		cout << "ERROR: Cannot open file!" << endl;

	// Call some function expecting an istream object
	DoSomething(File);

	// The End.
	cout << "\nPress ENTER to continue." << endl;
	cin.get();
}

