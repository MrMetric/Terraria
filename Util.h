#ifndef UTIL_H
#define UTIL_H

#include <errno.h>
#include <vector>
#include <cmath>
#include "Color.h"

class Util
{
	public:
		static bool hasEnding(string const &fullString, string const &ending);
		static string int2ver(int v);
		static string int2lang(int l);
		static string bool2str(bool b);
		static bool makeDir(const char* path, int perm);
		static int listDir(vector<string> &vec, const char* path, string ending);
		static bool fileExists(const char *filename);
		static bool fileDelete(const char *filename);
		static string strToLower(string s);
		static void clearCon();
		static void conTitle(string s);
		static int randInt(int limit);
		static int randInt(int start, int end);
		static int round(float f);
		static double IEEERemainder(double x, double y);
		static bool moveFile(string src, string dst, bool overwrite);
		#ifndef NOCOLOR
		static Color* randColor();
		#endif
};

//----------------------------------------------------------------------------------------
//	Copyright © 2004 - 2012 Tangible Software Solutions Inc.
//	This class can be used by anyone provided that the copyright notice remains intact.
//
//	This class is used to replace some conversions to or from strings.
//----------------------------------------------------------------------------------------

class StringConverterHelper
{
public:
	template<typename T>
	static string toString(const T &subject)
	{
		stringstream ss;
		ss << subject;
		return ss.str();
	}

	template<typename T>
	static T fromString(const string &subject)
	{
		stringstream ss(subject);
		T target;
		ss >> target;
		return target;
	}
};

#endif // UTIL_H
