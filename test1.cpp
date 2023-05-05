#include<stdio.h>
void byteorder()
{

union test1
{
    short value;
    char union_bytes[sizeof(short)];
        /* data */
}test1;
test1.value = 0x0102;
if(test1.union_bytes[0] == 1 && test1.union_bytes[1] == 2)
{
    printf("big\n");
} 
else if(test1.union_bytes[0] == 2 && test1.union_bytes[1] == 1){
    printf("short\n");
}
else 
{
    printf("unknown");
}  
}
int main()
{
    byteorder();
    return 0;
}