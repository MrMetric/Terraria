#include "Util.h"

// from http://stackoverflow.com/questions/874134/find-if-string-endswith-another-string-in-c
bool Util::hasEnding(string const &fullString, string const &ending)
{
	if(fullString.length() >= ending.length())
	{
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	return false;
}

string Util::int2ver(int v)
{
	switch(v)
	{
		case 37: return "1.1.1";
		case 39: return "1.1.2";
		default: return "?";
	}
}

string Util::int2lang(int l)
{
	switch(l)
	{
		case 1: return "English";
		case 2: return "German"; // Dutch?
		case 3: return "Italian";
		case 4: return "French";
		case 5: return "Spanish";
		default: return "Undefined";
	}
}

string Util::bool2str(bool b)
{
	return(b?"true":"false");
}

bool Util::makeDir(const char* path, int perm)
{
	#ifdef _WIN32
	if(mkdir(path) == -1)
	#else
	if(mkdir(path, perm) == -1)
	#endif
	{
		if(errno != EEXIST)
		{
			cerr << "Error creating \"" << path << "\": " << strerror(errno) << endl;
			return false;
		}
	}
	return true;
}

int Util::listDir(vector<string> &vec, const char* path, string ending)
{
	DIR *dp;
	struct dirent *dirp;
	if((dp = opendir(path)) == NULL)
	{
		cout << "Error opening \"" << path << "\": " << strerror(errno) << endl;
		return 0;
	}

	int num = 0;

	while((dirp = readdir(dp)) != NULL)
	{
		if(Util::hasEnding(dirp->d_name, ".wld"))
		{
			num++;
			vec.push_back(string(dirp->d_name));
		}
	}
	closedir(dp);

	return num;
}


// from http://www.cplusplus.com/forum/general/1796/
bool Util::fileExists(const char *filename)
{
	ifstream ifile(filename);
	return ifile;
}

bool Util::fileDelete(const char *filename)
{
	if(!remove(filename))
	{
		cerr << "Error deleting " << filename << ": " << strerror(errno);
		return false;
	}
	return true;
}

string Util::strToLower(string s)
{
	for(unsigned int i = 0; i < s.size(); i++)
	{
		s[i] = tolower(s[i]);
	}
	return s;
}

void Util::clearCon()
{

}

void Util::conTitle(string s)
{
	// TODO: Make this
}

int Util::randInt(int limit)
{
	srand(time(0));
	int divisor = RAND_MAX / (limit + 1);
	int ret;

	do
	{
		ret = rand() / divisor;
	} while(ret > limit);

	return ret;
}

int Util::randInt(int start, int end)
{
	return Util::randInt(end - start) + start;
}

int Util::round(float f)
{
	return (f > 0.0)?floor(f + 0.5):ceil(f - 0.5);
}

double Util::IEEERemainder(double x, double y)
{
	double regularMod = fmod(x, y);
	if(regularMod == 0)
	{
		if(x < 0)
		{
			return -0.0;
		}
	}
	double alternativeResult;
	alternativeResult = regularMod - (abs(y) * sin(x));
	if(abs(alternativeResult) == abs(regularMod))
	{
		double divisionResult = x/y;
		double roundedResult = Util::round(divisionResult);
		if(abs(roundedResult) > abs(divisionResult))
		{
			return alternativeResult;
		}
		else
		{
			return regularMod;
		}
	}
	if(abs(alternativeResult) < abs(regularMod))
	{
		return alternativeResult;
	}
	else
	{
		return regularMod;
	}
}

bool Util::moveFile(string src, string dst, bool overwrite)
{
	if(overwrite) remove(dst.c_str());
	else if(Util::fileExists(dst.c_str()))
	{
		cerr << "Error renaming \"" << src << "\" to \"" << dst << "\" (the file already exists)" << endl;
		return false;
	}
	rename(src.c_str(), dst.c_str());
	return true;
}

#ifndef NOCOLOR
Color* Util::randColor()
{
	int num = 0;
	int num2 = 0;
	int num3 = 0;
	while(num + num3 + num2 <= 150)
	{
		num = Util::randInt(255);
		num2 = Util::randInt(255);
		num3 = Util::randInt(255);
	}
	return new Color(num, num2, num3, 255);
}
#endif
