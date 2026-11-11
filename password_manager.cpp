#include<vector>
#include<sstream>
#include<iterator>
#include<sodium.h>
#include<iostream>
#include<fstream>
#include<limits>
#include<string>
#include<vector>
using namespace std;
vector<pair<string,string>> entries;
unsigned char vaultKey[crypto_secretbox_KEYBYTES];
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
bool deriveKey(const string& pw,const unsigned char* salt){
    return crypto_pwhash(vaultKey,sizeof vaultKey,pw.c_str(),pw.length(),salt,
        crypto_pwhash_OPSLIMIT_INTERACTIVE,crypto_pwhash_MEMLIMIT_INTERACTIVE,
        crypto_pwhash_ALG_DEFAULT)==0;
}

void saveVault(){
    string plain;
    for(const auto& e:entries){
        plain+=e.first+" "+e.second+"\n";
    }
    vector<unsigned char> out(crypto_secretbox_NONCEBYTES+plain.size()+crypto_secretbox_MACBYTES);
    randombytes_buf(out.data(),crypto_secretbox_NONCEBYTES);
    crypto_secretbox_easy(out.data()+crypto_secretbox_NONCEBYTES,
        (const unsigned char*)plain.data(),plain.size(),
        out.data(),vaultKey);
    ofstream file("vault.dat",ios::binary);
    file.write((const char*)out.data(),out.size());
}

bool loadVault(){
    entries.clear();
    ifstream file("vault.dat",ios::binary);
    if(!file.is_open()){
        return true;
    }
    vector<unsigned char> data((istreambuf_iterator<char>(file)),istreambuf_iterator<char>());
    if(data.size()<crypto_secretbox_NONCEBYTES+crypto_secretbox_MACBYTES){
        return false;
    }
    size_t cipherLen=data.size()-crypto_secretbox_NONCEBYTES;
    vector<unsigned char> plain(cipherLen);
    if(crypto_secretbox_open_easy(plain.data(),data.data()+crypto_secretbox_NONCEBYTES,
        cipherLen,data.data(),vaultKey)!=0){
        return false;
    }
    string text(plain.begin(),plain.begin()+(cipherLen-crypto_secretbox_MACBYTES));
    istringstream in(text);
    string site,password;
    while(in>>site>>password){
        entries.push_back({site,password});
    }
    return true;
}
void savePassword(const string& site,const string& password){
    entries.push_back({site,password});
    saveVault();
    display("password saved successfully!");
}
void viewPassword(){
    if(entries.empty()){
        display("\n[System] No passwords saved yet.");
        return;
    }
    display("\nSaved Passwords:");
    for(const auto& e:entries){
        cout<<" Site: "<<e.first<<", Password: "<<e.second<<"\n";
    }
}
void searchPassword(const string& target){
    for(const auto& e:entries){
        if(e.first==target){
            cout<<" Site: "<<e.first<<", Password: "<<e.second<<"\n";
            return;
        }
    }
    display("\n[System] No entry found for that site.");
}
void deletePassword(const string& target){
    for(size_t i=0;i<entries.size();i++){
        if(entries[i].first==target){
            entries.erase(entries.begin()+i);
            saveVault();
            display("\n[System] Entry deleted successfully!");
            return;
        }
    }
    display("\n[System] No entry found for that site.");
}
bool siteExists(const string& target){
    for(const auto& e:entries){
        if(e.first==target){
            return true;
        }
    }
    return false;
}

void updatePassword(const string& target,const string& newPassword){
    for(auto& e:entries){
        if(e.first==target){
            e.second=newPassword;
            saveVault();
            display("\n[System] Password updated successfully!");
            return;
        }
    }
    display("\n[System] No entry found for that site.");
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

    unsigned char salt[crypto_pwhash_SALTBYTES];
    randombytes_buf(salt,sizeof salt);
    ofstream saltFile("salt.dat",ios::binary);
    saltFile.write((const char*)salt,sizeof salt);
    saltFile.close();

    if(!deriveKey(pw1,salt)){
        std::cerr<<"\n[System] Error: could not create the encryption key.\n";
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
    ifstream file("master.dat");
    string stored;
    file>>stored;
    file.close();

    unsigned char salt[crypto_pwhash_SALTBYTES];
    ifstream saltFile("salt.dat",ios::binary);
    saltFile.read((char*)salt,sizeof salt);
    if(saltFile.gcount()!=(streamsize)sizeof salt){
        std::cerr<<"\n[System] salt.dat is missing or damaged.\n";
        return false;
    }

    for(int attempt=1;attempt<=3;attempt++){
        string input;
        display("\nEnter master password: ");
        cin>>input;
        if(crypto_pwhash_str_verify(stored.c_str(),input.c_str(),input.length())==0){
            if(!deriveKey(input,salt)){
                std::cerr<<"\n[System] Error: could not create the encryption key.\n";
                return false;
            }
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
if(!loadVault()){
    std::cerr<<"\n[System] Could not decrypt vault.dat. It may be corrupted.\n";
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
    sodium_memzero(vaultKey,sizeof vaultKey);
    return 0;
}