#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <vector>
#include <errno.h>
#include <stdlib.h>
#include <sstream>
#include <sys/times.h>
#include <pthread.h>
#include <iomanip>
#include <fstream>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <algorithm>
#include <cmath>
#include <dirent.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <csignal>
#include <glob.h>
#include <cstring>

int write_path_name(void);
void read_commands(std::string& commands);
int split_intern_commands(std::string& commands, std::vector<std::string>& allcommands, int& input, int& output);
void split_extern_commands(std::string& commands, std::vector<std::string>& allcommands);
void cd(std::vector<std::string>& allcommands, int input, int output, bool conveyer);
void pwd(std::vector<std::string>& allcommands, int input, int output, bool conveyer);
void exit_this_stupido_function(std::vector<std::string>& allcommands, int input, int output, bool conveyer);
void time(std::vector<std::string>& allcommands, int input, int output, bool conveyer);
void extern_command(std::vector<std::string>& allcommands, int input, int output, bool conveyer);
void signal_handler(int signal);
std::vector<std::string> Glob(const std::string& pattern);

int fd[2];
bool main_process = true;
const int amount_of_commands = 4;
void (*pointers_to_functions[])(std::vector<std::string>& allcommands, int input, int output, bool conveyer) = {&cd, &pwd, &time, &exit_this_stupido_function};
std::string inner_command[amount_of_commands] = {"cd", "pwd", "time", "exit"};

int write_path_name(void) {
	char * si_path_name = getcwd(nullptr, 0); //возрващает директорию в сишных строках
	if(si_path_name != nullptr) { //директория получена успешно
		std::string plus_path_name = si_path_name;
		size_t found = 	plus_path_name.find_last_of("/"); //поиск первого "/"
		char uid;
		if(getuid() == 0) //оценка уровня привилегированности пользователя
			uid = '!'; //привилегированный
		else
			uid = '>'; //непривилегированный
		plus_path_name = plus_path_name.substr(found + 1);
		if(plus_path_name == "") {
			plus_path_name = "/";
		}
		std::cout << plus_path_name << " " << uid << " " << std::endl; //вывод названия текущей папки (рабочей директории)
	} else {
		std::cerr << "Sorry, Microsha cannot find out the directory name\n";
	}
	free(si_path_name);
	return 0;
}

void read_commands(std::string& commands) {
	std::getline(std::cin , commands); //считывает до \n, \r, \0, EOF
	return;
}

void split_extern_commands(std::string& commands, std::vector<std::string>& allcommands) {
	while(commands.size() > 0) { //разделяем программы на вектор строк по прямому слешу
		int slash;
		std::string one_command = commands;
		//std::cout << commands << std::endl;
		if((slash = commands.find_first_of('|')) != std::string::npos) {
			one_command = commands.substr(0, slash);
		} else {
			slash = commands.size() - 1;
		}
		allcommands.push_back(one_command);
		commands.erase(0, slash + 1);
	}
	return;
}

int split_intern_commands(std::string& commands, std::vector<std::string>& allcommands, int& input, int&output) {
	std::stringstream str(commands); //создание из строки строкового потока
	std::string one_element;
	while(str >> one_element) //"cout" из commands одной команды в one_element
		allcommands.push_back(one_element);
	
	for(int i = 0; i < allcommands.size() - 1; i++) {
		if(allcommands[i] == "<") {
				if(i == allcommands.size() - 1) {
					perror("Microsha");
					break;
				}
				input = open(allcommands[i + 1].c_str(), O_RDONLY, 0777);
				if(input < 0) {
					perror("Microsha");
					break;
				}
				allcommands.erase(allcommands.begin() + i, allcommands.begin() + i + 2); //удаляем < и файл, следующий за ним
				i--;
		}
		if(allcommands[i] == ">") {
				if(i == allcommands.size() - 1) {
					perror("Microsha");
					break;
				}
				output = open(allcommands[i + 1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
				if(input < 0) {
					perror("Microsha");
					break;
				}
				allcommands.erase(allcommands.begin() + i, allcommands.begin() + i + 2); //удаляем < и файл, следующий за ним
				i--;
		}
	}

	for (int i = 0; i < allcommands.size(); i++) {
        unsigned long kleene_star = std::string::npos, question = std::string::npos;
        if (allcommands[i].find_first_of('*') != std::string::npos || allcommands[i].find_first_of('?') != std::string::npos) {
            std::vector<std::string> glob = Glob(allcommands[i]);
			if(glob.size() == 0) {
				return 0;
			}
            allcommands.erase(allcommands.begin() + i, allcommands.begin() + i + 1);
            allcommands.insert(allcommands.begin() + i, glob.begin(), glob.end());
        }
    }
	return 1;	
}

void time(std::vector<std::string>& allcommands, int input, int output, bool conveyer) {
	bool match = 0;
	allcommands.erase(allcommands.begin(), allcommands.begin() + 1); //удаляем "time"	
	double real_minutes = 0, real_seconds = 0, user_minutes = 0, user_seconds = 0, system_minutes = 0, system_seconds = 0;
	if(allcommands.size() > 0) {
		std::clock_t c_start = std::clock();
		struct tms buf;
		times(&buf); //начало отсчета

		for(int i = 0; i < amount_of_commands; i++) {
			if(allcommands[0] == inner_command[i]) {//если совпадает название команды с одной из внутренних ф-ций
				pointers_to_functions[i](allcommands, input, output, conveyer);
				match = true;
				break;
			}
		}
		if(match == false) {
			extern_command(allcommands, input, output, conveyer);
		}

		std::clock_t c_end = std::clock();
		times(&buf);
		clock_t wall_time = c_end - c_start;
		real_minutes = floor(((double) wall_time) / CLOCKS_PER_SEC / 60 * 1000);
		real_seconds = ((double) wall_time) / CLOCKS_PER_SEC * 1000  - real_minutes * 60;
		user_minutes = floor((double) (buf.tms_utime + buf.tms_cutime) / CLOCKS_PER_SEC / 60 * 1000);
		user_seconds = (double) (buf.tms_utime + buf.tms_cutime) / CLOCKS_PER_SEC * 1000 - user_minutes * 60;
		system_minutes = floor((double) (buf.tms_stime + buf.tms_cstime) / CLOCKS_PER_SEC * 1000 / 60);
		system_seconds = (double) (buf.tms_stime + buf.tms_cstime) / CLOCKS_PER_SEC * 1000 - system_minutes * 60;
	}
	std::cout << "Wall_time: " << std::setiosflags(std::ios::fixed) << std::setprecision(0)<< real_minutes << " minutes " << std::setiosflags(std::ios::fixed) << std::setprecision(3) << real_seconds << " seconds" << std::endl;
	std::cout << "User_time: " << std::setiosflags(std::ios::fixed) << std::setprecision(0) << user_minutes << " minutes " << std::setiosflags(std::ios::fixed) << std::setprecision(3) << user_seconds << " seconds " << std::endl;
	std::cout << "Sys_time: " << std::setiosflags(std::ios::fixed) << std::setprecision(0) << system_minutes << " minutes " << std::setiosflags(std::ios::fixed) << std::setprecision(3) << system_seconds << " seconds" << std::endl;
	return;
}

void pwd(std::vector<std::string>& allcommands, int input, int output, bool conveyer) { //вывести рабочую директорию
	int status_for_fork;
	pid_t pid, wpid;
    pid = fork();
    if (pid == 0) {
        if (output != STDOUT_FILENO)
        {
            if(dup2(output, STDOUT_FILENO) < 0) //замена потоков вывода
            {
                perror("Replacing stdout");
            }
            if(close(output) < 0)
            {
                perror("Closing incoming output");
            }
        }

        char* si_path_name = getcwd(nullptr, 0); //получил рабочую директорию
        if(si_path_name == nullptr) { //проверил на вырожденность
            perror("Microsha ");
        }
        std::string plus_path_name = si_path_name;
        std::cout << plus_path_name << std::endl;
        
        free(si_path_name); //очистили динамическую память
        close(STDOUT_FILENO);
        exit(EXIT_FAILURE); //завершение процесса ребенка
    }
    else
    {
        if (pid < 0)
        {
            perror("Microsha");
        }
        else
        {
            do
            {
                wpid = waitpid(pid, &status_for_fork, WUNTRACED);
            }
            while (!WIFEXITED(status_for_fork) && !WIFSIGNALED(status_for_fork)); //пока не завершит выполнение, или пока не появится сигнал, который либо завершает текущий процесс либо требует вызвать функцию-обработчик
        }
    }
    return;
}

void cd(std::vector<std::string>& allcommands, int input, int output, bool conveyer) {//поменять директорию
	//allcommands[0] = "cd", allcommands[1] = "name_of_the_directory"
	if(allcommands.size() > 1) {
		if(chdir(allcommands[1].c_str()) != 0) {
			perror("Microsha");
			return;
		}
	} else {
		char* buf = getenv("HOME"); //получаем домашнюю директорию
		if (buf == nullptr) {
			std::cerr << "Microsha: HOME directory is undefined\n";
			return;
		}
		std::string path = buf;
		if(chdir(path.c_str()) < 0) {//переходим в домашнюю директорию
			perror("Microsha");
		}
	}
	return;
}

void extern_command(std::vector<std::string>& allcommands, int input, int output, bool conveyer) { //внешняя команда
	std::vector<char *> argv; //форматируем набор команд в требуемый формат
	for (size_t i = 0; i < allcommands.size(); i++)
		argv.push_back((char*)allcommands[i].c_str());
	argv.push_back(NULL);

	int status;
	pid_t pid = fork(), wpid;
	if (pid == 0) {
		if (output != STDOUT_FILENO) {
            if(dup2(output, STDOUT_FILENO) < 0) //замена потоков вывода
            {
                perror("Replacing stdout");
            }
            if(close(output) < 0)
            {
                perror("Closing incoming output");
            }
        }
		if (input != STDIN_FILENO) {
            if(dup2(input, STDIN_FILENO) < 0) //замена потоков вывода
            {
                perror("Replacing stdin");
            }
            if(close(input) < 0)
            {
                perror("Closing incoming input");
            }
        }
		if(execvp(argv[0], &argv[0]) < 0) { //выолнение внешней команды
			perror("Microsha");
			close(STDOUT_FILENO);//close(STDIN_FILENO);
			exit(EXIT_FAILURE);
		}
	} else
	{
		if (pid < 0) {
		    perror("Microsha");
		} else { //следует дописать дополнительные условия
			if(output == STDOUT_FILENO || !conveyer) {
				do {
		        	wpid = waitpid(pid, &status, WUNTRACED);
					//std::cout << "*" << std::endl;
				}
		   		while (!WIFEXITED(status) && !WIFSIGNALED(status)); //пока не завершит выполнение, или пока не появится сигнал, который либо завершает текущий процесс либо требует вызвать функцию-обработчик
			}
		}
	}
	return;
}

void exit_this_stupido_function(std::vector<std::string>& allcommands, int input, int output, bool conveyer) {
	exit(EXIT_SUCCESS);
	return;
}

void signal_handler(int signal) { //собственный обработчик сигнала
	if(main_process == true) {
    	std::cout << std::endl;
    	write_path_name();
	}
}

std::vector<std::string> Glob(const std::string& pattern) {
    glob_t glob_result; //результат совпадений
    memset(&glob_result, 0, sizeof(glob_result));

    int return_value = glob(pattern.c_str(), 0, nullptr, &glob_result);
	 if (return_value == GLOB_NOMATCH) {
        globfree(&glob_result); //в случае ошибки могла записаться всякая ерунда
        std::cout << "No such file or directory" << std::endl;
    }
    if (return_value != 0 && return_value != GLOB_NOMATCH) {
        globfree(&glob_result); //в случае ошибки могла записаться всякая ерунда
        std::cout << "Sorry, glob() had failed and returned value" << return_value << ". Search in manual for more information."<< std::endl;
    }

    std::vector<std::string> matched_files;
    for (int i = 0; i < glob_result.gl_pathc; i++) {
        matched_files.push_back(std::string(glob_result.gl_pathv[i]));
    }
    globfree(&glob_result); //освобождение памяти от результатов работы glob()
    return matched_files;
}

int main(void) {
	std::signal(SIGINT, signal_handler);

	std::cout << std::endl << "Microsha\n" << "Here you can see the list of internal functions:\n" << std::endl;
	for(int i = 0; i < amount_of_commands; i++)
		std::cout << inner_command[i] << std::endl;
    std::cout << std::endl << "External functions could be runned only if you write the correct path and arguments. If you try to run function in a pipeline component, metasimvols as \"<\" and \">\" are prohibited.\n" << std::endl;

    do
	{
		bool conveyer = false;
		bool match = false;
		write_path_name();
		std::string commands;
		try {//проверяем на сигнал
			read_commands(commands);
        }
        catch(std::ios_base::failure e) {
            std::cerr << "Please, enter your line again\n";
        }
		main_process = false;
		std::vector<std::string> allcommands;
		split_extern_commands(commands, allcommands); //разделили на команды
		int in = STDIN_FILENO;
		if(allcommands.size() > 1) {
			conveyer = true;
		}
		for(int j = 0; j < allcommands.size(); j++) {
			if(pipe(fd) < 0) {
				perror("Microsha");
			}
			bool descriptor = (j == allcommands.size() - 1); //последняя команда
			int out = descriptor ? STDOUT_FILENO : fd[1]; //если команда последняя, то передаем на стандартный поток вывода, иначе - в pipe
			std::vector<std::string> splitted_commands;
			if(split_intern_commands(allcommands[j], splitted_commands, in, out) == 0) {
				break; //преобразовали каждую отдельно команду в вектор строк
			}
			for(int i = 0; i < amount_of_commands; i++) {
				if(splitted_commands[0] == inner_command[i]) {//если совпадает название команды с одной из внутренних ф-ций
					pointers_to_functions[i](splitted_commands, in, out, conveyer);
					match = true;
					break;
				}
			}
			if(match == false) {
				extern_command(splitted_commands, in, out, conveyer);
			}
			in = fd[0]; //следующая команда будет получать информацию из предыдущего
			if(close(fd[1]) < 0) {
				perror("Microsha");
			}
		}
		main_process = true;
	}
	//while(getchar() != EOF);
	while(std::cin.eof() != 1);
    return 0;
}

/*C:\Users\dns\Documents\GitHub\Microsha\internal function.cpp*/