#include <stdio.h>
#include <string.h>

int main() {

   char a[100], b[100];

   printf("Enter the first string\n");
   fgets(a, sizeof(a), stdin);

   printf("Enter the second string\n");
   fgets(b, sizeof(b), stdin);

   if (strcmp(a,b) == 0)
      printf("Entered strings are equal.\n");
   else
      printf("Entered strings are not equal.\n");
}