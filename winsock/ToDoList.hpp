#include <iostream>
#include <vector>
class toDoList{
  private:
      std::vector<std::string> todolist = {};
  
      int eunm(){
        std::string str;
          int num = -1;
          while (true){
            num = num + 1;
            str = todolist.at(num);
            std::cout << str<< std::endl;
            if (num > todolist.size()-2){
              return 0;
            }
            
            
            }
      }
      
    public:
        int add(std::string toAdd){
        todolist.push_back(toAdd);
        return 0;}

        std::string getFirstItem(){
        return todolist.at(0);
      }

        int delFirstItem(){
          todolist.erase(todolist.begin());
          return 0;}

        std::string get(){
            std::string msg= todolist.at(0);
            delFirstItem();
            return msg; }
      
        int size(){
            return todolist.size();
      }
};