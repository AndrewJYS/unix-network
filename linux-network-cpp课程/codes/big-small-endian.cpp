#include <iostream>
using namespace std;

union
{
    short a;
    char b[sizeof(short)];
} un1;

union
{
    int a;
    char b[sizeof(int)];
} un2;

int main()
{
    cout << sizeof(short) << " " << sizeof(int) << " " << sizeof(long long) << endl;

    un1.a = 0x0102;
    cout << (int)un1.b[0] << " " << (int)un1.b[1] << " " << un1.a << endl;

    un2.a = 0x01020304;
    cout << (int)un2.b[0] << " " << (int)un2.b[1] << " " << (int)un2.b[2] << " " << (int)un2.b[3] << " " << un2.a << endl;

    /*
    2 4 8
    2 1 258
    4 3 2 1 16909060
    因此是小端序，因为低位存储低位数字
    */

    return 0;
}
