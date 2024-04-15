#include "main.h"
// список встроенных команд и соот им функции
char *command_str[] = {
  "cd",
  "clr",
  "dir",
  "environ",
  "echo",
  "help",
  "pause",
  "quit"
};

int (*command_func[]) (char **) = {
  &cd,
  &clr,
  &dir,
  &this_environ,
  &echo,
  &help,
  &pause,
  &quit
};

int command_num() {
  return sizeof(command_str) / sizeof(char *);// функция , возвращает кол во команд
}

char *command_comm[] = { // встроенные команды с их описанием
  "cd <directory> - смена текущего каталога по умолчанию на <directory>. Если аргумент <directory> отсутствует, вывести текущий каталог.",
  "clr - очистка экрана.",
  "dir <directory> - вывод содержимого каталога <directory>.",
  "environ - вывод всех переменных среды.",
  "echo <comment> - вывод на экран <comment>, после которого выполня­ется переход на новую строку.",
  "help - вывод руководства пользователя.",
  "pause - приостановка операций оболочки до нажатия клавиши <Enter>.",
  "quit - выход из оболочки."
};




int cd(char **args) { // функция для смены директории
  char *cur_dir;
  if (args[1] == NULL) { // если без аргумента, то выводим текущую директорию
    cur_dir = getcwd(NULL, 0); // получение пути
    if (cur_dir) { // если все удачно - выводим путь, иначе ошибку
      printf("Текущая директория: %s\n", cur_dir);
      free(cur_dir);
    } else {
      perror("cd");
    }
  } else { // если мы все же получили аргумент
    if (chdir(args[1]) != 0) { // изменяем текущий каталог на новый
      perror("cd"); // если возникла ошибка , выводим ее
    } else {
      setenv("PWD", args[1], 1); // устанавливаем переменную окружения PWD в новый рабочий каталог
    }
  }
  return 1;
}


int clr() { // очистка консоли
  printf("\033c"); // команда для очистки
  return 1;
}


int dir(char **args) { // вывод содержимого каталога директории
  if (args[1] == NULL) { // если аргумент пустой
    fprintf(stderr, "Укажите путь директории\n");
  } else {
    DIR *dir;
    struct dirent *entry; // указатели необходимые для работы с директориями

    dir = opendir(args[1]); // попытка открыть директорию
    if (dir == NULL) { // неполучилось
      fprintf(stderr, "Error: %s директории не существует\n", args[1]);
    } else { // уполучилось
      printf("__________________________________________\n");
      while ((entry = readdir(dir)) != NULL) { // вывод каждого элемента в консоль
        printf("%s\n", entry->d_name);
      }
      printf("__________________________________________\n");
      closedir(dir); // закрытие директории
    }
  }
  return 1;
}


int this_environ() { // все переменные окружения
  extern char **environ;  // переменные окружения
  char **env = environ;
  printf("__________________________________________\n");
  while (*env != NULL) {// пока переменные есть, выводим
    printf("%s\n", *env);
    env++;
  }
  printf("__________________________________________\n");
  return 1;
}


int echo(char **args) { // для вывода комментария в консоль
  if (args[1] == NULL) {  // если аргумент пустой выводим ничего, переходим на другую строку
    printf("__________________________________________\n");
    printf("\n");
    printf("__________________________________________\n");
  } else { // если что то есть, то выводим через пробел
    printf("__________________________________________\n");
    for (int i = 1; args[i] != NULL; i++) {
      printf("%s", args[i]);
      if (args[i + 1] != NULL) {
        printf(" ");
      }
    }
    printf("\n");
    printf("__________________________________________\n");
  }
  return 1;
}


int help() { // функция выводящая массив со списком команд и их описания
  printf("__________________________________________\n");
  printf("Добро пожаловать в custom_shell версии 1.0\n");
  printf("__________________________________________\n");
  printf("Список встроенных комманд с описанием:\n");
  printf("--------------------------------------\n");
  for (int i = 0; i < command_num();i++) {
    printf(" %s\n", command_comm[i]);
  }
  printf("--------------------------------------\n");
  return 1;
}


int pause()// пауза, путем бексконечного цикла до перевода строки
{
  printf("Нажмите Enter для продолжения работы...");
  while (getchar() != '\n');
  return 1;
}


int quit()// выход из оболочки
{
  return 0;
}


int launch(char **args)// запуск процессов с использованием fork() для дочернего процесса и exec() для замены программы
{
  pid_t pid, wpid; // необходимы для процессов
  int status;

  pid = fork();//создаем дочерний процесс
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {// замена процессов
      fprintf(stderr, "Команда не найдена %s\n", args[0]);
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {//если новый процесс не создался

    perror("err");
  } else {
    do {//выполнение родительского класса
      wpid = waitpid(pid, &status, WUNTRACED);//ожидаем завершение дочернего
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}


char **split_line(char *line) { // разбиение строки на аргументы
    int buffer_size = BUF_SIZE, pos = 0;; // начальный размер буффера, который по необходимости будет расширятся
    char **tokens = malloc(buffer_size * sizeof(char*)); // массив для хранения токенов
    char *token; // токены
    if (!tokens) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOKENS_DELIMITERS); // разделение строки на токены по делителям с помощью strtok
    while(token != NULL) {
        tokens[pos] = token;// Добавление токенов в наш массив
        pos++;

        if (pos >= buffer_size) { // если достигли макс размера
            buffer_size += BUF_SIZE;
            tokens = realloc(tokens, buffer_size * sizeof(char*)); // перераспределение памяти для токенов
            if (!tokens) {
                fprintf(stderr, "Ошибка выделения памяти\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, TOKENS_DELIMITERS); // последний токен
    }
    tokens[pos] = NULL; // обозначение конца массива
    return tokens;
}


char *read_line() {
    char *line = NULL;
    ssize_t size = 0;
    getline(&line, &size, stdin); // считывание строки
    return line;
}

int execute(char **args) // проверка аргументов и вызов функцй
{
  if (args[0] == NULL) {// команда пустая
    return 1;
  }

  for (int i = 0; i < command_num(); i++) { // двигаемся по всем доступным командам
    if (strcmp(args[0], command_str[i]) == 0) { // при совпадении выполняем функцию
      return (*command_func[i])(args);
    }
  }

  return launch(args); // иначе внешний процесс
}


void loop(void)
{
  char *line;// строка пользователя
  char **args;// аргументы строки
  int status; //статус программы

  do {
    printf("> ");
    line = read_line(); // считывание строки
    args = split_line(line);// считывание аргументов из строки
    status = execute(args);// обработка аргуметов и изменение статуса

    free(line); // освобождение памяти 
    free(args);
  } while (status);
}


int main(int argc, char **argv)//основная программа
{
  char dir[PATH_MAX]; // массив для пути к исполняемому файлу
  realpath(argv[0], dir); // сохранение пути к файлу в buf
  setenv("shell", dir, 1); // установка переменно окружения


  loop();//запуск цикла


  return EXIT_SUCCESS;// выключение
}
