/* Программа выводит в stdout разницу между двумя файлами, полученную запуском утилиты diff */
#include <stdio.h>
#include <stdlib.h>

int main()
{
        char* p;
        p = system("diff file1.txt file2.txt");
        printf("%s", p);
        return 0;
}
