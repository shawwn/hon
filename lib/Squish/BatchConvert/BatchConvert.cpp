#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <stdio.h>
#include <time.h>

int texQuality = 0;
int oggQuality = 0;
char BASE_PATH[MAX_PATH] = {0};
char TARGET_PATH[MAX_PATH] = {0};

void FileCallback(char* full, char *dir, WIN32_FIND_DATA *data)
{
	char *ext;
	char target[MAX_PATH];
	// Determine our target path
	if(dir[strlen(dir)-1] != '\\')
		sprintf(target, "%s%s\\%s", TARGET_PATH, dir+strlen(BASE_PATH), data->cFileName);
	else
		sprintf(target, "%s%s%s", TARGET_PATH, dir+strlen(BASE_PATH), data->cFileName);
	// Convert image files to S2T files
	if((ext = strstr(full, ".tga")) || (ext = strstr(full, ".png")) || (ext = strstr(full, ".jpg")))
	{
		char temp[MAX_PATH*2];
		char* target_ext = strstr(target, ext);
		strcpy(target_ext, ".s2t");

		printf("Converting: %s\n\n", full);
		sprintf(temp, "texenc.exe \"%s\" \"%s\"%s", full, target, (texQuality != 0) ? " hq" : "");
		printf("\n");
		system(temp);
	}
	// Convert sound files to OGG files
	else if((ext = strstr(full, ".wav")))
	{
		char temp[MAX_PATH*2];
		char* target_ext = strstr(target, ext);
		strcpy(target_ext, ".ogg");

		printf("Converting: %s\n\n", full);
		sprintf(temp, "oggenc2.exe -q%d --output=\"%s\" \"%s\"", oggQuality, target, full);
		printf("\n");
		system(temp);
	}
	// Everything else should just be copied over
	else
	{
		printf("Copying: %s\n", full);
		CopyFile(full, target, 0);
	}
}

// Recreate every base directory at the target
void DirCallback(char* full, char *dir, WIN32_FIND_DATA *data)
{
	int strlength = strlen(dir);
	char target[MAX_PATH];
	// Determine our target path
	if(dir[strlength-1] != '\\')
		sprintf(target, "%s%s\\%s", TARGET_PATH, dir+strlen(BASE_PATH), data->cFileName);
	else
		sprintf(target, "%s%s\\%s", TARGET_PATH, dir+strlen(BASE_PATH), data->cFileName);
	// Create the directory in the new directory structure
	CreateDirectory(target, NULL);
}

void dirwalk(char *dir, void(*ffnc)(char *, char *, WIN32_FIND_DATA *), void(*dfnc)(char *, char *, WIN32_FIND_DATA *), int recursive)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	char DirSpec[MAX_PATH];   // directory specification
	char DirSpec2[MAX_PATH];  // directory specification of the found file
	// Setup the directory
	strncpy (DirSpec, dir, strlen(dir)+1);
	strncat (DirSpec, "\\*", 3);
	// Skip the first and second files, they're always . and ..
	hFind = FindFirstFile(DirSpec, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return;
	FindNextFile(hFind, &FindFileData);
	// Begin the real loop
	while(FindNextFile(hFind, &FindFileData) != 0)
	{
		sprintf(DirSpec2, "%s\\%s", dir, FindFileData.cFileName);
		if(strstr(FindFileData.cFileName, ".svn") || strstr(FindFileData.cFileName, "CVS"))
			continue;
		// Directories are only implictly written when you write files
		if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// Execute the directory function, and recurse if asked to
			if(dfnc)
				dfnc(DirSpec2, dir, &FindFileData);
			if(recursive)
				dirwalk(DirSpec2, ffnc, dfnc, recursive);
		}
		else
		{
			// Execute the file function
			if(ffnc)
				ffnc(DirSpec2, dir, &FindFileData);
		}
	}
	FindClose(hFind);
}

int main(int argc, char *argv[])
{
	// Check for arguements
	if(argc < 3)
	{
		printf("Syntax: filename.exe <basepath> <targetpath> [ogg quality] [tex hq?]\n");
		return 0;
	}
	if(argc >= 4)
		oggQuality = atoi(argv[3]);
	if(argc >= 5)
		texQuality = (atoi(argv[4]) != 0);

	// Setup the base and target paths
	strcpy(BASE_PATH, argv[1]);
	strcpy(TARGET_PATH, argv[2]);
	// Create the target directory in case it doesn't exist
	CreateDirectory(TARGET_PATH, NULL);
	// Begin walking the base directory and do the actual work
	dirwalk(BASE_PATH, FileCallback, DirCallback, 1);

	// Exit message
	printf("\nAll files have been successfully converted!\n");
	printf("Please check ", time);
	printf(TARGET_PATH);
	printf(" for the new files...\n\n");

	// Bye!
	return 0;
}