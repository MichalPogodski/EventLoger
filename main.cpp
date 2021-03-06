#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <unistd.h>
#include<string>
#include<vector>
#include <algorithm>
#include <typeinfo>

using namespace std;


class DataWriter {

    public:
        virtual void WriteData(int ID, string info) = 0;
};



class FileWriter: public DataWriter{

    private:
        FileWriter(){
            path = string(get_current_dir_name()) + "/Data/Events.txt";
        }

    public:
        ofstream txtfile;
        string path;
        static FileWriter* instance;

        static FileWriter* createInstance(){
              if(instance == nullptr)
                  instance = new FileWriter();
              return instance;
          }

        void WriteData(int ID, string info){
            txtfile.open(path, fstream::app);
            txtfile <<ID << ", "<<info <<endl;
            txtfile.close();
        }
};



class DataBaseWriter: public DataWriter{

    private:
        DataBaseWriter(){
            path = string(get_current_dir_name()) + "/Data/DataBaseErrors.txt";
        }

    public:
        ofstream txtfile;
        string path;
        static DataBaseWriter* instance;

        static DataBaseWriter* createInstance() {
              if(instance == nullptr)
                  instance = new DataBaseWriter();
              return instance;
        }

        void WriteData(int ID, string info){
            sqlite3 *db;
            sqlite3_open("Data/Events.db", & db);

            string createQuery = "CREATE TABLE IF NOT EXISTS events (userid INTEGER, info TEXT);";
            sqlite3_stmt *createStmt;
            sqlite3_prepare(db, createQuery.c_str(), createQuery.size(), &createStmt, NULL);
            if (sqlite3_step(createStmt) != SQLITE_DONE){
                cout << "Didn't Create Table!" << endl;
                txtfile.open(path, fstream::app);
                txtfile <<"Didn't Create Table!" << endl;
                txtfile.close();
            }

            string insertQuery = "INSERT INTO events (userid, info) VALUES (" + to_string(ID) + ", '" + info + "');";
            sqlite3_stmt *insertStmt;
            sqlite3_prepare(db, insertQuery.c_str(), insertQuery.size(), &insertStmt, NULL);
            if (sqlite3_step(insertStmt) != SQLITE_DONE){
                cout << "Didn't Insert Item!" << endl;
                txtfile.open(path, fstream::app);
                txtfile <<"Didn't Insert Item! InstertQuety: " << insertQuery <<endl;
                txtfile.close();
            }
        }
};



class Log{

    public:
        vector<DataWriter*> subscribers;

        void LogToSubscribers(int ID, string info){
            for (int i = 0; i < subscribers.size(); ++i) {
                subscribers[i] -> WriteData(ID, info);
            }
        }

        void AddSubscriber(DataWriter* sub){
            subscribers.push_back(sub);
        }

        void EraseSubscriber(DataWriter* sub){
            for (int i = 0; i < subscribers.size(); ++i) {
                if (typeid(subscribers[i]).name() == typeid(sub).name()){
                    subscribers.erase(subscribers.begin()+i);
                }
            }
        }
};



FileWriter* FileWriter::instance = nullptr;
DataBaseWriter* DataBaseWriter::instance = nullptr;

int main(){
    Log EventLoger;
    FileWriter *TxtWriter = FileWriter::createInstance();
    DataBaseWriter *DBWriter = DataBaseWriter::createInstance();

    EventLoger.AddSubscriber(TxtWriter);
    EventLoger.AddSubscriber(DBWriter);

    EventLoger.LogToSubscribers(1234, "first pass");
    EventLoger.EraseSubscriber(TxtWriter);
    EventLoger.LogToSubscribers(12345678, "second pass");


    return 0;
}
