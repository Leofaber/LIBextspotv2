#include "FolderManager.h"

FolderManager::FolderManager()
{
}

vector<string> FolderManager::getFileNamesFromFolder(string folderpath){

    /*OPEN MAP DIRECTORY*/
    vector<string> fileNames;
	DIR *pdir = NULL;
	struct dirent *pent = NULL;
	//cout <<"Folderpath: "<< folderpath << endl;
	pdir = opendir (folderpath.c_str());
    if (pdir == NULL)                   // if pdir wasn't initialised correctly
        {
            cout << "\n[Folder Manager] Error: files path is wrong!";
 
            exit (3);
        }

     while (pent = readdir (pdir)) // while there is still something in the directory to list
    {
        if (pent == NULL) // if pent has not been initialised correctly
        { // print an error message, and exit the program
            cout << "\n[Folder Manager] Error: folder is empty!";
 
            exit (3);

        }
        // otherwise, it was initialised correctly. Let's print it on the console:
        if(pent->d_name[0]!='.') {
            fileNames.push_back(pent->d_name);
            //cout << pent->d_name << endl;
        }
    }

    // finally, let's close the directory
    closedir (pdir);

   // cin.get (); // pause for input
    return fileNames; // everything went OK

}
