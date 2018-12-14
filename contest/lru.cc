#include <iostream>
#include <list>
#include <unordered_map>
#include <vector>
#include <queue>
using namespace std; 

int max_size, m;
int lru_page_fault = 0;

typedef struct Lru {
    list<int> actual;
    unordered_map<int, list<int>::iterator> ma_p;
}Lru;

void add_to_lru(int x, Lru& lru) {
    cout << x;
    if(lru.ma_p.find(x) == lru.ma_p.end()) { //если он отсутсвует в mape
        lru_page_fault++; //счетчик страничных нарушений
        cout << "- ";
        if(lru.actual.size() == max_size) { //лист полон
            int last = lru.actual.back();
            lru.actual.pop_back();// удаляем последний элемент из списка
            lru.ma_p.erase(last);// удаляем последний элемент из map
        }
    } else {
        cout << "+ ";
        lru.actual.erase(lru.ma_p[x]); //удаляем, тем самым поддерживая "least recently"
    }
    lru.actual.push_front(x);
    lru.ma_p[x] = lru.actual.begin();//сохраняем итераторы на эл-ты листа
}
  
int main() 
{ 
    Lru lru;
    cin >> max_size >> m;
    vector<int> pages;  
    for (int i = 0; i < m; i++) {
        int x;
        cin >> x;
        pages.push_back(x);
    }
    for (int i = 0; i < m; i++) {
        add_to_lru(pages[i], lru);
    }
    cout << lru_page_fault << endl;
    return 0; 
} 
