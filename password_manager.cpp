#include<iostream>
#include<fstream>
#include<limits>
#include<string>
using namespace std;
void display(const string& msg){
    cout<<msg<<endl;
}
void displayMenu(){
    display("\nPassword Manager");
    display("1. Add password");
    display("2. View password");
    display("3. EXIT");
    display("Enter your choice: ");
}
void savePassword(const string& site,const string& password){
    std::ofstream file("passwords.txt",std::ios::app);
    if(file.is_open()){
        file<<site<<" "<<password <<"\n";
        file.close();
        display("password saved successfully!");
    }
    else{
    std::cerr<<"Error: Unable to open file for writing.\n";
}
}
int main(){
    int choice;
    string site,password;
    do{
        displayMenu();
            cin>> choice;
            if(cin.fail()){
                cin.clear();
                cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            } 
            switch(choice){
                case 1:
                display("Enter site name: ");
                cin>>site;
                display("Enter password: ");
                cin>>password;
                savePassword(site,password);

                break;
                case 2:

                break;
                case 3:

                break;
                default:

                break;
            }
        

    }while(choice!=3);
    return 0;
}