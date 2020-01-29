#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
using namespace std;

typedef struct chunkNode{
	int numCollisions;
	string file1;
	string file2;
} chunkNode;

string convertToString(string file, string filePath);
int wordsLeft(string wholeFile);
void checkForCheaters(vector<string> files, int n, int threshold, string filePath);
string chunk(string s, int chunkSize);
string deleteFirstWord(string s);
int getHashValue(string s);


/*function... might want it in some class?*/
int getdir (string dir, vector<string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
        files.push_back(string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}

/**
 * main driver function
 * @param files a vector of files to check for plagiarism
 * @param chunkSize the desired chunk size
 * @param threshold the number of collisions to determine a plagiarized paper
 */
void checkForCheaters(vector<string> files, int chunkSize, int threshold, string filePath)
{
    std::vector<int> hashTable[150001];                         //hash table set up as an array of vectors
    //int outputArray[files.size()][files.size()];                //array of collided files
	vector<chunkNode> sortingVector;
    	
	int **outputArray = new int*[files.size()];
	for(int i = 0; i < files.size(); ++i)
		outputArray[i] = new int[files.size()];

    for(int i = 0; i < files.size(); i++)                       //initializing output array by setting everything to 0
        for(int j = 0; j < files.size(); j++)
            outputArray[i][j] = 0;

    files.erase(files.begin(), files.begin()+2);           //erases the first two documents because they're weird and we don't like them

    for(int i = 0; i < files.size(); i++)                       //hashing each file in the file vector and putting it into the hash table
    {
        vector<string> chunks;                                  //temporary vector of chunks
        string wholeFile = convertToString(files[i], filePath);           //convert the whole file to string

        while (wordsLeft(wholeFile) >= chunkSize)               //putting chunks into the chunks vector
        {
            string temp = chunk(wholeFile, chunkSize);
            chunks.push_back(temp);
            wholeFile = deleteFirstWord(wholeFile);
        }
//cout << chunks[0] << endl;
//cout << getHashValue(chunks[0]);
	for(int chunk = 0; chunk < chunks.size(); chunk++)                             //hashing
        {
            int index = getHashValue(chunks[chunk]);
            if(hashTable[index].size() == 0 || hashTable[index].at(hashTable[index].size()-1) != i)	//check for collisions within the file
                hashTable[index].push_back(i);                  //putting into the hash table
        }
    }

    for(int fileVector = 0; fileVector < 150001; fileVector++)                     //putting data from hash table into output table
    {
        if(hashTable[fileVector].size() >= 2)
        {
            for(int i = 0; i < hashTable[fileVector].size(); i++)
            {
                for(int j = i+1; j < hashTable[fileVector].size(); j++)
                {
                    outputArray[hashTable[fileVector].at(i)][hashTable[fileVector].at(j)] = outputArray[hashTable[fileVector].at(i)][hashTable[fileVector].at(j)] + 1;
                }
            }
        }
    }

    for (int i = 0; i < files.size(); i++)                          //iterating through output table and printing the important data
    {
        for(int j = 0; j < files.size(); j++)
        {
            if(outputArray[i][j] > threshold)
            {
		    chunkNode temp;
		    temp.numCollisions = outputArray[i][j];
		    temp.file1 = files[i];
		    temp.file2 = files[j];
		    sortingVector.push_back(temp);
               // cout << outputArray[i][j] << ":   " <<files[i] << ", " << files[j] << endl;
            }
        }
    }

    for(int i = 0; i < sortingVector.size(); i++)			//sorting output vector
    {
	    for(int j = 0; j < sortingVector.size(); j++)
	    {
		if(sortingVector[i].numCollisions > sortingVector[j].numCollisions)
		{
			chunkNode temp = sortingVector[j];
			sortingVector[j] = sortingVector[i];
			sortingVector[i] = temp;
		}
	    }
    }
    
    for(int i = 0; i < sortingVector.size(); i++)			//printing the output vector
    {
	cout << sortingVector[i].numCollisions << ":  " << sortingVector[i].file1 << ", " << sortingVector[i].file2 << endl;
    }

	for(int i = 0; i < files.size(); i++)
		delete [] outputArray[i];
	delete [] outputArray;
}

/**
 * generates a hash key using the input string
 * @param s the input string
 * @return a hash key
 */
int getHashValue(string s)
{
    int hashKey = 0;
    int exp = 1;
    for(int i = 0; i < s.length(); i++)
    {
        hashKey = hashKey + ((int) (s[i] * exp) % 150001);
        exp = (exp * 7) % 150001;                               //hash algorithm
    }
    return hashKey % 150001;
}

/**
 * Deletes the first word of the given string
 * @param s the input string
 * @return the string without the first word
 */
string deleteFirstWord(string s)
{
    while(s[0] != ' ')
    {
        s.erase(0,1);
    }
    s.erase(0,1);
    return s;
}

/**
 * Chunks a long string into chunkSize chunks of words
 * @param s the input string
 * @param chunkSize the desired size of the chunks
 * @return a chunkSize chunk of words off the front of the input string
 */
string chunk(string s, int chunkSize)
{
    string chunk;
    int numWords = 0;
    for(int i = 0; i < s.length(); i++)
    {
        if(s[i] == ' ')                             //counts the number of words by the spaces
            numWords++;
        else
        {
            if(s[i] >= 65 && s[i] <= 90)            //converts upper case to lower case
            {
                char temp = s[i];
                temp += 32;
                chunk += temp;
            }
            else if(s[i] >= 97 && s[i] <= 122)      //normal case (lowercase letters)
                chunk += s[i];
            else if(s[i] >= 48 && s[i] <= 57)       //allow numbers in chunks
                chunk += s[i];
        }
        if(numWords == chunkSize)                   //reached desired chunk size
            return chunk;
    }
    return chunk;
}

/**
 * counts how many words are left in the input string
 * @param s the input string
 * @return the number of words left in the string
 */
int wordsLeft(string s)
{
    int count = 0;
    for(int i = 0; i < s.length(); i++)
    {
        if(s[i] == ' ')
            count++;
    }
    count++;
    return count;
}

/**
 * converts the file into a string
 * @param fileName the name of the input file
 * @return a string containing the contents of the file
 */
string convertToString(string fileName, string filePath)
{
    //string f = "C:\\Users\\ethan\\Documents\\312 labs\\Lab8\\med_doc_set\\" + fileName;			//windows
    string f = filePath + '/' + fileName;
    ifstream myFile(f.c_str());
    string str;
    if(myFile)
    {
        ostringstream s;
        s << myFile.rdbuf();
        str = s.str();
    }
    myFile.close();
    return str;
}

int main(int argc, char *argv[])
{
    string filePath = argv[1];
    int chunkSize = atoi(argv[2]);
    int threshold = atoi(argv[3]);

    //string dir = string("C:\\Users\\ethan\\Documents\\312 labs\\Lab8\\med_doc_set");

    vector<string> files = vector<string>();
    //getdir(dir,files);
    getdir(filePath, files);
    //checkForCheaters(files, 6, 100, filePath);
    checkForCheaters(files, chunkSize, threshold, filePath);

    return 0;
}
