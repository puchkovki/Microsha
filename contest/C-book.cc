#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#include <iostream>
#include <string>

using namespace std;

int Add();
int Add(string key, string value);
int Delete();
int Delete(string key);
int Update();
int Print();

int HASH_SIZE = 4096;
unsigned long long FILE_SIZE = 128*1024*1024;

int amount_of_different_commands = 4;
static string commands_names[] = {"ADD", "DELETE", "UPDATE", "PRINT"};
static int (*pointers_to_functions[]) () = {&Add, &Delete, &Update, &Print};

static int fd;
static unsigned *adress; //адрес начала выделенной памяти
static unsigned freeplace = HASH_SIZE; //адрес смещения от начала(поделить на два?)
static hash<string> hash_func;

int Add() {
    string key, value;
    cin >> key;
    cin >> value;
    if(Add(key, value) == -1) {
        return -1;
    }
    return 0;
}

int Add(string key, string value) {
    unsigned long offset = hash_func(key) % HASH_SIZE; //для ограниченного размера хеш-таблицы
    unsigned* new_adress = adress + offset;
    while(*new_adress != 0) {
        new_adress = adress + *new_adress; //переходим на следующий элемент
        unsigned int length_of_key = *new_adress; //смотрим размер ключа
        char* freaky_string_with_key = new char[length_of_key + 1]; //выполняем проверку ключа на совпадение
        new_adress +=3; //перешли к местоположению строки
        memcpy(freaky_string_with_key, new_adress, length_of_key); //создали фиктивный ключ для сравнения
        freaky_string_with_key[length_of_key] = '\0'; //закончили создание фиктивного ключа
        string normal_string = freaky_string_with_key;
        if(normal_string == key) {
            delete[] freaky_string_with_key;
            return -1;
        }
        new_adress--; //указываем на ячейку с адресом следующего
        delete[] freaky_string_with_key;
    }
    //отсутствует адрес
    *new_adress = freeplace; //указали на первую незаполненную часть файла
    new_adress = adress + freeplace;  //перешли в незаполненную часть файла
    *new_adress = key.size(); //сначала ключ
    new_adress++;
    *new_adress = value.size(); //потом значение
    new_adress++;
    *new_adress = 0; //потом адрес на следующий
    new_adress++;
    string key_value = key + value;
    memcpy(new_adress, key_value.c_str(), key_value.size());
    freeplace += 4 + ((key_value.size()) / sizeof(int)); //установили следующее свободное место
    return 0;
}

int Delete() {
    string key;
    cin >> key;
    if(Delete(key) == -1) {
        return -1;
    }
    return 0;
}

int Delete(string key) {
    unsigned long offset = hash_func(key) % HASH_SIZE;
    unsigned *new_adress = adress + offset;
    unsigned *old_adress = new_adress;
    while(*new_adress != 0) {
        new_adress = adress + *new_adress; //переходим на следующий элемент
        unsigned int length_of_key = *new_adress; //смотрим размер ключа
        char* freaky_string_with_key = new char[length_of_key + 1]; //выполняем проверку ключа на совпадение
        new_adress +=3; //перешли к местоположению строки
        memcpy(freaky_string_with_key, new_adress, length_of_key); //создали фиктивный ключ для сравнения
        freaky_string_with_key[length_of_key] = '\0'; //закончили создание фиктивного ключа
        string normal_string = freaky_string_with_key;
        new_adress--;//возвращаем указатель на адрес следующего
        if(normal_string == key) {
            *old_adress = *new_adress;
            delete[] freaky_string_with_key;
            return 0;
        }
        old_adress = new_adress; //если не совпало, что все двигаем вперед
        delete[] freaky_string_with_key;
    }
    return -1;
}

int Update() {
    string key, value;
    cin >> key;
    cin >> value;
    if(Delete(key) == -1) {
        return -1;
    }
    if(Add(key, value) == -1) {
        return -1;
    }
    return 0;
}

int Print() {
    string key;
    cin >> key;
    unsigned long offset = hash_func(key) % HASH_SIZE;
    unsigned *new_adress = adress + offset;
    while(*new_adress != 0) {
        new_adress = adress + *new_adress; //переходим на следующий элемент
        unsigned int length_of_key = *new_adress; //смотрим размер ключа
        char* freaky_string_with_key = new char[length_of_key + 1]; //выполняем проверку ключа на совпадение
        new_adress +=3; //перешли к местоположению строки
        memcpy(freaky_string_with_key, new_adress, length_of_key); //создали фиктивный ключ для сравнения
        freaky_string_with_key[length_of_key] = '\0'; //закончили создание фиктивного ключа
        string normal_string = freaky_string_with_key;
        new_adress--;//возвращаем указатель на адрес следующего
        if(normal_string == key) {
            cout << normal_string << " ";
            new_adress--;
            unsigned length_of_value = *new_adress;
            new_adress += 2;
            char* freaky_string_with_value = new char[length_of_key + length_of_value + 1];
            memcpy(freaky_string_with_value, new_adress, length_of_key + length_of_value);
            freaky_string_with_value[length_of_key + length_of_value] = '\0';
            string normal_string_reborn = freaky_string_with_value;
            normal_string_reborn.erase(0, length_of_key);
            cout << normal_string_reborn << endl;
            delete[] freaky_string_with_value;
            delete[] freaky_string_with_key;
            return 0;
        }
        delete[] freaky_string_with_key;
    }
    return -1;
}

int main(void) {
    fd = open("keeper", O_RDWR | O_CREAT | O_TRUNC, 0777);
    lseek(fd, FILE_SIZE, SEEK_SET); //устанавливаем размер файла
    write(fd, " ", 1); //устанавливаем размер файла
    adress = (unsigned*) mmap(nullptr, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SEEK_SET);//писать, читать, и одна память на несколько программ (чисто, чтобы был флаг)//вернет адресс начала выделенной памяти
    memset(adress, 0, HASH_SIZE * sizeof(int));
    int amount_of_all_commands;
    cin >> amount_of_all_commands;
    for (int i = 0; i < amount_of_all_commands; i++) {
        string command;
        cin >> command;
        for (int j = 0; j < amount_of_different_commands; j++) {
            if (command == commands_names[j]) {
                if((*pointers_to_functions[j])() < 0) {
                    cout << "ERROR" << endl;
                }
            }
        }
    }
    munmap(adress, FILE_SIZE);
    remove("keeper");
    return 0;
}