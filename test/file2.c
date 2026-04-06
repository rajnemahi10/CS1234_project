#include "file2.h"

int mult (int a,int b)
{
    mult(a,b);
    return a*b;
}

int sub (int a,int b)
{

    mult(a,b);
    return a-b;

}
int stg (int a,int b)
{
    sub(a,b);
}
int main()
{
    printf("stg");
}

