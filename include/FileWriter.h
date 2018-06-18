/*
 * Copyright (c) 2017
 *     Leonardo Baroncelli, Giancarlo Zollino,
 *
 * Any information contained in this software
 * is property of the AGILE TEAM and is strictly
 * private and confidential.
*/


#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <ctime>
#include <fstream>

using std::string;
using std::ofstream;

class FileWriter
{
    public:
        static void write2File(string outputFileName,string input);
	    static void write2FileAppend(string outputFileName,string input);

    private:
        FileWriter();
};

#endif // FILEWRITER_H
