#include <fstream>
using namespace std;
int main()
{
    ifstream ifs;
    ifs.open("debug.log");
    std::string str;
    int k = 0;
    while (!ifs.eof())
    {
        char c;
        ifs.read(&c, 1);
        if (!ifs.eof())
        {
            if (c == '#')
            {
                str.push_back('\n');
                str.push_back('\n');
            }
            str.push_back(c);
        }
    }
    ofstream ofs;
    ofs.open("debug.tidy");
    ofs << str;
    ofs.flush();
}