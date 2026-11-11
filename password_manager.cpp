#include<sodium.h>
#include<iostream>
#include<fstream>
#include<limits>
#include<string>
#include<vector>
using namespace std;
void display(const string& msg){
    cout<<msg<<endl;
}
void displayMenu(){
    display("\nPassword Manager");
    display("1. Add password");
    display("2. View password");
    display("3. Search password");
    display("4. Update password");
    display("5. Delete password");
    display("6. EXIT");
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
void viewPassword(){
    std::ifstream file("passwords.txt");
    if(file.is_open()){
        string site,password;
        display("\nSaved Passwords:");
        while(file>>site>>password){
            cout<<" Site: "<<site <<", Password: "<<password<<"\n";
        }
        file.close();
    } else{
        std::cerr<<"\n[System] No passwords saved yet.\n";
    }
}
void searchPassword(const string& target){
    std::ifstream file("passwords.txt");
    if(!file.is_open()){
        std::cerr<<"\n[System No passwords saved yet.\n]";
        return;
    }
    string site,password;
    bool found=false;
    while(file>>site>>password){
        if(site==target){
            cout<<" Site: "<<site<<", Password: "<<password<<"\n";
            found=true;
        }
    }
    file.close();
    if(!found){
        display("\n[System] No entry found for that site.");
    }
}
void deletePassword(const string& target){
    std::ifstream in("passwords.txt");
    if(!in.is_open()){
        std::cerr<<"\n[System] No passwords saved yet.\n";
        return;
    }
    vector<pair<string,string>> entries;
    string site,password;
    bool found=false;
    while(in>>site>>password){
        if(site==target){
            found=true;
            continue;
        }
        entries.push_back({site,password});
    }
    in.close();

    if(!found){
        display("\n[System] No entry found for that site.");
        return;
    }

    std::ofstream out("passwords.txt");
    for(const auto& e:entries){
        out<<e.first<<" "<<e.second<<"\n";
    }
    out.close();
    display("\n[System] Entry deleted successfully!");
}
bool siteExists(const string& target){
    std::ifstream file("passwords.txt");
    if(!file.is_open()){
        return false;
    }
    string site,password;
    while(file>>site>>password){
        if(site==target){
            return true;
        }
    }
    return false;
}

void updatePassword(const string& target,const string& newPassword){
    std::ifstream in("passwords.txt");
    if(!in.is_open()){
        std::cerr<<"\n[System] No passwords saved yet.\n";
        return;
    }
    vector<pair<string,string>> entries;
    string site,password;
    bool found=false;
    while(in>>site>>password){
        if(site==target){
            password=newPassword;
            found=true;
        }
        entries.push_back({site,password});
    }
    in.close();

    if(!found){
        display("\n[System] No entry found for that site.");
        return;
    }

    std::ofstream out("passwords.txt");
    for(const auto& e:entries){
        out<<e.first<<" "<<e.second<<"\n";
    }
    out.close();
    display("\n[System] Password updated successfully!");
}
bool masterExists(){
    std::ifstream file("master.dat");
    return file.good();
}

bool setupMaster(){
    string pw1,pw2;
    display("\n[System] First time setup: create a master password.");
    while(true){
        display("Create master password (minimum 8 characters): ");
        cin>>pw1;
        if(pw1.length()<8){
            display("\n[System] Too short. Try again.");
            continue;
        }
        display("Confirm master password: ");
        cin>>pw2;
        if(pw1!=pw2){
            display("\n[System] Passwords do not match. Try again.");
            continue;
        }
        break;
    }

    char hash[crypto_pwhash_STRBYTES];
    if(crypto_pwhash_str(hash,pw1.c_str(),pw1.length(),
        crypto_pwhash_OPSLIMIT_INTERACTIVE,
        crypto_pwhash_MEMLIMIT_INTERACTIVE)!=0){
        std::cerr<<"\n[System] Error: could not hash the password.\n";
        return false;
    }

    std::ofstream file("master.dat");
    if(!file.is_open()){
        std::cerr<<"\n[System] Error: could not save the master password.\n";
        return false;
    }
    file<<hash;
    file.close();
    display("\n[System] Master password created!");
    return true;
}

bool login(){
    std::ifstream file("master.dat");
    string stored;
    file>>stored;
    file.close();

    for(int attempt=1;attempt<=3;attempt++){
        string input;
        display("\nEnter master password: ");
        cin>>input;
        if(crypto_pwhash_str_verify(stored.c_str(),input.c_str(),input.length())==0){
            display("\n[System] Login successful!");
            return true;
        }
        display("\n[System] Wrong password. Attempts left: "+to_string(3-attempt));
    }
    return false;
}
int main(){
    int choice;
    string site,password;
    if(sodium_init()<0){
    std::cerr<<"[System] Failed to start libsodium.\n";
    return 1;
}
if(!masterExists()){
    if(!setupMaster()){
        return 1;
    }
} else if(!login()){
    display("\n[System] Too many failed attempts. Exiting.");
    return 1;
}
    do{
        displayMenu();
            cin>> choice;
            if(cin.fail()){
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(),'\n');
                display("\n[System] Please enter a number.");
                continue;
            } 
            switch(choice){
            case 1:
                display("Enter site name: ");
                cin>>site;
                if(siteExists(site)){
                display("\n[System] This site already has a password. Use option 4 to update it.");
                break;
                }
                display("Enter password: ");
                cin>>password;
                savePassword(site,password);
                break;
            case 2:
                viewPassword();
                break;
            case 3:
                display("Enter site name to search: ");
                cin>>site;
                searchPassword(site);
                break;
            case 4:
                display("Enter site name to update: ");
                cin>>site;
                display("Enter new password: ");
                cin>>password;
                updatePassword(site,password);
                break;
            case 5:{
                display("Enter site name to delete: ");
                cin>>site;
                display("Are you sure? (y/n): ");
                char confirm;
                cin>>confirm;
                if(confirm=='y' || confirm=='Y'){
                    deletePassword(site);
                } else{
                    display("\n[System] Deletion cancelled.");
                }
                break;
            }
            case 6:
                display("\n[System] Exiting...");
                break;
            default:
                display("\n[System] Invalid choice . Please try again.");
                break;
            }
        

    }while(choice!=6);
    return 0;
}