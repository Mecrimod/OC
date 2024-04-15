#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#define BUF_SIZE 100 // размер буфера
#define TOKENS_DELIMITERS " \t\r\n\a" // разделители, по которым будет разбиваться строка
//обьясления функций
int cd(char **args);
int clr();
int dir(char **args);
int this_environ();
int echo(char **args);
int help();
int pause();
int quit();
