#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
using namespace std;
string cuvint(stringstream &ss)
{
    string s;
    if (!(ss >> s))
    {
        return "";
    }
    reverse(s.begin(), s.end());
    string rest = cuvint(ss);
    if (rest.empty())
    {
        return s;
    }
    return s + " " + rest;
}
int main()
{
    string line;
    cout << "afiseaza fraza:";
    getline(cin, line);
    stringstream ss(line);
    cout << cuvint(ss);
    return 0;
}