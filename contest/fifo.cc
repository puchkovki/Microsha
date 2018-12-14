#include <iostream>
#include <list>
#include <unordered_map>
#include <vector>
#include <queue>
using namespace std; 

int max_size, m;
int fifo_page_fault = 0;

typedef struct Fifo {
    list<int> actual;
    unordered_map<int, list<int>::iterator> ma_p; //просто для более быстрого поиска
}Fifo;

void add_to_fifo(int x, Fifo& fifo) {
    cout << x;
    if(fifo.ma_p.find(x) == fifo.ma_p.end()) { //если он отсутсвует в mape
        fifo_page_fault++; //счетчик страничных нарушений
        cout << "- ";
        if(fifo.actual.size() == max_size) { //лист полон
            int last = fifo.actual.back();
            fifo.actual.pop_back();// удаляем последний элемент из списка
            fifo.ma_p.erase(last);// удаляем последний элемент из map
        }
        fifo.actual.push_front(x);
        fifo.ma_p[x] = fifo.actual.begin();
    } else {
        cout << "+ ";
    }
}

int main() 
{ 
    Fifo fifo;
    cin >> max_size >> m;
    vector<int> pages;  
    for (int i = 0; i < m; i++) {
        int x;
        cin >> x;
        pages.push_back(x);
    }
    for (int i = 0; i < m; i++) {
        add_to_fifo(pages[i], fifo);
    }
    cout << fifo_page_fault << endl;
    return 0; 
} 
