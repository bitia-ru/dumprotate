#include "dumprotate.h"
#include <string.h>

int parse_args(Dumprotate* drd, int argc, char** argv) {
    char currentFlag = '0';
    for (int i = 1; i < argc; i++)
    {
        if (currentFlag == '0')
        {
            if ((strlen(argv[i]) != 2) || (argv[i][0] != '-'))
            {
                return 1;
            }
            switch (argv[i][1])  
            {  
               case 's':  
                  currentFlag = 's';  
                  break;  
               case 'n':  
                  currentFlag = 'n'; 
                  break;
               case 'e':  
                  currentFlag = 'e'; 
                  break;
               case 'c':  
                  currentFlag = 'c'; 
                  break;
               case 'd':  
                  currentFlag = 'd'; 
                  break;
               case 'h':  
                  return 0;
               default:  
                  return 1;
            } 
        }
        else
        {
            switch (currentFlag)  
            {  
               case 's':  
                  if (argv[i][strlen(argv[i])-1] > 9)
                  {
                      switch (argv[i][strlen(argv[i])-1])
                      {
                          case 'k':
                          case 'K':
                              argv[i][strlen(argv[i])-1] = 0;
                              printf("%d\n",atoi(argv[i])*1024);
                              break;
                          case 'm':
                          case 'M':
                              break;
                          case 'g':
                          case 'G':
                              break;
                          default:  
                              return 1;
                      }
                  } 
                  break;  
               case 'n':  
                  currentFlag = 'n'; 
                  break;
               case 'e':  
                  currentFlag = 'e'; 
                  break;
               case 'c':  
                  currentFlag = 'c'; 
                  break;
               case 'd':  
                  currentFlag = 'd'; 
                  break;
            } 
            currentFlag = '0'; 
        }
    }
    return 0;
}
