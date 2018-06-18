#ifndef FOLDERMANAGER_H
#define FOLDERMANAGER_H
#include <dirent.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <vector>

using namespace std;


class FolderManager
{
    public:

        static vector<string> getFileNamesFromFolder(string folderPath);

    private:
        FolderManager();
};

#endif // FOLDERMANAGER_H
