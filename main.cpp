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



// Abstract class. Interface for subclasses
class DataWriter {

    public:
        virtual void WriteData(int ID, string info) = 0;
};



// Write data to txt file
class FileWriter: public DataWriter{

    private:
        FileWriter(){
            path = string(get_current_dir_name()) + "/Data/Events.txt";
        }

    public:
        ofstream txtfile;
        string path;
        static FileWriter* instance;

        // creating instance of the class with private constructor
        static FileWriter* createInstance(){
              if(instance == nullptr)
                  instance = new FileWriter();
              return instance;
          }

        // Write data to txt file
        void WriteData(int ID, string info){
            txtfile.open(path, fstream::app);
            txtfile <<ID << ", "<<info <<endl;
            txtfile.close();
        }
};



// Write data to DataBase
class DataBaseWriter: public DataWriter{

    private:
        DataBaseWriter(){
            path = string(get_current_dir_name()) + "/Data/DataBaseErrors.txt";
        }

    public:
        ofstream txtfile;
        string path;
        static DataBaseWriter* instance;

        // creating instance of the class with private constructor
        static DataBaseWriter* createInstance() {
              if(instance == nullptr)
                  instance = new DataBaseWriter();
              return instance;
        }

        // Manage DataBase
        void WriteData(int ID, string info){
            sqlite3 *db;
            sqlite3_open("Data/Events.db", & db);

            // Create table
            string createQuery = "CREATE TABLE IF NOT EXISTS events (userid INTEGER, info TEXT);";
            sqlite3_stmt *createStmt;
            sqlite3_prepare(db, createQuery.c_str(), createQuery.size(), &createStmt, NULL);

            // Log all failures to txt file (/data/DataBaseErrors.txt)
            if (sqlite3_step(createStmt) != SQLITE_DONE){
                cout << "Didn't Create Table!" << endl;
                txtfile.open(path, fstream::app);
                txtfile <<"Didn't Create Table!" << endl;
                txtfile.close();
            }


            // instert data
            string insertQuery = "INSERT INTO events (userid, info) VALUES (" + to_string(ID) + ", '" + info + "');";
            sqlite3_stmt *insertStmt;
            sqlite3_prepare(db, insertQuery.c_str(), insertQuery.size(), &insertStmt, NULL);

            // Log all failures to txt file (/data/DataBaseErrors.txt)
            if (sqlite3_step(insertStmt) != SQLITE_DONE){
                cout << "Didn't Insert Item!" << endl;
                txtfile.open(path, fstream::app);
                txtfile <<"Didn't Insert Item! InstertQuety: " << insertQuery <<endl;
                txtfile.close();
            }
        }
};



// log data about events
class Log{

    public:
        // vector of data's subscribers
        vector<DataWriter*> subscribers;

        // Write data with every subscriber
        void LogToSubscribers(int ID, string info){
            for (int i = 0; i < subscribers.size(); ++i) {
                subscribers[i] -> WriteData(ID, info);
            }
        }

        // add new subscriber
        void AddSubscriber(DataWriter* sub){
            subscribers.push_back(sub);
        }

        // erase specific subscriber
        void EraseSubscriber(DataWriter* sub){
            for (int i = 0; i < subscribers.size(); ++i) {
                if (typeid(subscribers[i]).name() == typeid(sub).name()){
                    subscribers.erase(subscribers.begin()+i);
                }
            }
        }
};



// global variables - nullptr as a pointer of subclasses instances
FileWriter* FileWriter::instance = nullptr;
DataBaseWriter* DataBaseWriter::instance = nullptr;

int main(){
    Log EventLogger;
    FileWriter *TxtWriter = FileWriter::createInstance();
    DataBaseWriter *DBWriter = DataBaseWriter::createInstance();

    // adding subsriber to EventLogger's vector
    EventLogger.AddSubscriber(TxtWriter);
    EventLogger.AddSubscriber(DBWriter);

    // writing data (to DataBase and txt file)
    EventLogger.LogToSubscribers(1234, "first pass");
    // erasing TxtWriter from subscribers
    EventLogger.EraseSubscriber(TxtWriter);
    // writing data (to DataBase)
    EventLogger.LogToSubscribers(12345678, "second pass");


    return 0;
}
