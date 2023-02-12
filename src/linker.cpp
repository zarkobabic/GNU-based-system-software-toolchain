#include <fstream>
#include <iostream>
#include "../inc/linker.hpp"
#include <map>
#include <string.h>
#include <set>
#include <iomanip>



using namespace std;

map<string, LinkerSimbolTableEntry*>* linkerSymbolTable = new map<string, LinkerSimbolTableEntry*>();
vector<FilesSectionsEntry*>* filesSection = new vector<FilesSectionsEntry*>;
int LinkerSimbolTableEntry::indexID = -1;
std::set <string> u_set;
std::set <string> d_set;

void print_hex_representation(const std::vector<char>& v) {
  for (char c : v) {
    printf("%02x ", static_cast<unsigned int>(c & 0xff));
  }
  std::puts("");
}


void printLinkerSymbolTable(){
    map<string, LinkerSimbolTableEntry*>::iterator itSection;
    for (itSection = linkerSymbolTable->begin(); itSection != linkerSymbolTable->end(); itSection++){
        std::cout<< "Name: "<< itSection->first<<endl;
        std::cout<< "index: " << itSection->second->index<<endl;
        std::cout<< "Pocetna adresa:[za sekcije], vrednost[za simbole] " << itSection->second->value<<endl;
        std::cout<< "Tip: " <<itSection->second->type<<endl;
        std::cout<< "Velicina [za sekcije]: " << itSection->second->secSize<<endl;
        std::cout<<" Sadrzaj sekcije:"<<endl;
        std::cout<<" Sada pripada sekciji: "<<itSection->second->newSection<<endl;
        if(itSection->second->type == -1){
            print_hex_representation(*itSection->second->bytes);
        }
        std::cout<<endl;
        
    }
}

void toHex(string outputName){

    ofstream txtFile;
    ofstream outfile;

    string textName = outputName.substr(0, outputName.size() - 3);
    textName = textName + "txt";

    outfile.open(outputName, std::ios::binary);
    txtFile.open(textName);


    int i = 0;
    int sumSize = 0;

    //dohvatamo velicinu svih sekcija
    map<string, LinkerSimbolTableEntry*>::iterator itSection;
    for (itSection = linkerSymbolTable->begin(); itSection != linkerSymbolTable->end(); itSection++){
        if(itSection->second->type == -1){
            sumSize += itSection->second->secSize;
        }
    }

    int out_address_counter = 0;
    while( i < sumSize){
        map<string, LinkerSimbolTableEntry*>::iterator itSection;
        for (itSection = linkerSymbolTable->begin(); itSection != linkerSymbolTable->end(); itSection++){
            if(itSection->second->type == -1 && itSection->second->value == i){

                


                for (char c : *itSection->second->bytes) {
                    
                    
                    // printf("%02x ", static_cast<unsigned int>(c & 0xff));

                    //posalji u objektni fajl
                    

                    if(txtFile.is_open()){
                        if(out_address_counter % 8 == 0){
                            if(out_address_counter > 0){
                                txtFile << endl;
                            }
                            txtFile << hex << setfill('0') << setw(4)<< out_address_counter <<": ";
                        }
                        txtFile << hex << setfill('0') << setw(2)<< static_cast<unsigned int>(c & 0xff)<< " ";
                        out_address_counter++;
                    }


                    if (outfile.is_open()){
                        outfile.write(reinterpret_cast<char*>(&c), sizeof(char));
                    }
                }
        
                i += itSection->second->secSize;
            }
        }
    }

    outfile.close();
    txtFile.close();
}



int main(int argc, char *argv[]){
    
    int start_input = 1;
    int valid = 0;
    string outputName = "default_name.hex";
    
    while(start_input < argc){
        if (strcmp(argv[start_input], "-o") == 0) {
          outputName = argv[start_input + 1];
          start_input += 2;
        }
        else if(strcmp(argv[start_input], "-hex") == 0){
            valid = 1;
            start_input += 1;
        }
        else if(argv[start_input][0] == '-'){
            start_input += 1;
        }
        else{
            break;
        }
    }
    if(!valid){
        cerr<< "Greska ne pojavljuje se hex ili relocatible u naredbi uopste"<<endl;
        exit(-1);
    }
    if(start_input == argc){
        cerr<<"Greska nema ulaznih parametara" <<endl;
        exit(-1);
    }

    int memoryAlocator = 0;
    vector<ifstream>* files = new vector<ifstream>;
    
    
    //redom uzima asemblerske fajlove i obradjuje
    for(int i = 0; i < argc - start_input; i++){

        
        ifstream infile(argv[i + start_input], std::ios::binary);

        if (infile.is_open()){
            
            int numSections;
            infile.read(reinterpret_cast<char*>(&numSections), sizeof(int));//prima broj sekcija

            for(int j = 0; j < numSections; j++){


                int indexOld, secSize;
                infile.read(reinterpret_cast<char*>(&indexOld), sizeof(int));
                infile.read(reinterpret_cast<char*>(&secSize), sizeof(int));
                
                string secName;
                size_t stringSize;

                infile.read(reinterpret_cast<char*>(&stringSize), sizeof(std::size_t));
                secName.resize(stringSize);
                infile.read(&secName[0], stringSize);

                size_t bytesSize;
                infile.read(reinterpret_cast<char*>(&bytesSize), sizeof(std::size_t)); //ovde se cita velicina masinskog koda sekcije koji cemo sutra primiti



                
                //Mapiranje i racunanje velicine
                map<string, LinkerSimbolTableEntry*>::iterator existingSec = linkerSymbolTable->find(secName);

                if (existingSec != linkerSymbolTable->end()){


                    //pronadjen simbol te sekcije, postoji vec neka sekcija koja se isto tako zove, treba nadovezati ovu na nju i ispomerati ostale ako mora
                    int addressToAddNewSec = existingSec->second->value + existingSec->second->secSize;

                    //Ovde mora u tabeli simbola da se pomeri ka gore ako treba
                    map<string, LinkerSimbolTableEntry*>::iterator itSection;
                    for (itSection = linkerSymbolTable->begin(); itSection != linkerSymbolTable->end(); itSection++){
                        if(itSection->second->type == -1){ //jeste sekcija
                            if(itSection->second->value >= addressToAddNewSec){
                                itSection->second->value += secSize; //podignemo je gore za secSize

                                for(int l = 0; l < filesSection->size(); l++){
                                    if((*filesSection)[l]->name == itSection->first){
                                        (*filesSection)[l]->start_addr += secSize;
                                    }
                                }


                            }
                        }
                    }




                    //azuriramo velicinu u tabeli simbola za spojenu sekciju


                    existingSec->second->bytes->resize(existingSec->second->bytes->size() + bytesSize);
                    infile.read(existingSec->second->bytes->data() + existingSec->second->bytes->size() - bytesSize, bytesSize);

                    existingSec->second->secSize += secSize;

                    //azuriramo memory alocator jer gde god da se uglavi sledeca prva slobodna lok ce biti stari memoryAlocator + secSize
                    memoryAlocator += secSize;
                    
                    
                    //dodajemo nasu sekciju u FilesSection
                    //dodavanje u FilesSections
                    FilesSectionsEntry* newEntry = new FilesSectionsEntry(secName, indexOld, i, secSize, addressToAddNewSec);
                    filesSection->push_back(newEntry);
                }
                else{
                    //nije pronadjen simbol te sekcije, ne postoji sekcija sa istim nazivom, dodati naziv u tabelu simbola a odmah alocirati memoriju
                    //dodavanje u tabelu simbola
                    LinkerSimbolTableEntry *helper = new LinkerSimbolTableEntry(memoryAlocator, -1, secSize);        
                    //citanje sadrzaja sekcije u upisivanje u tablu simbola linkera
                    helper->bytes = new vector<char>;
            
                    helper->bytes->resize(bytesSize);
                    infile.read(helper->bytes->data(), bytesSize);

                    // print_hex_representation(*helper->bytes);

                    linkerSymbolTable->insert({secName, helper});

                    
                    //dodavanje u FilesSections
                    FilesSectionsEntry* newEntry = new FilesSectionsEntry(secName, indexOld, i, secSize, memoryAlocator);
                    filesSection->push_back(newEntry);
                    
                    memoryAlocator += secSize;  
                }
               
                
            }

            
            files->push_back(std::move(infile));
        }
    }





    //redom uzima asemblerske fajlove i obradjuje tabele simbola sad
    for(int i = 0; i < argc - start_input; i++){
         
        if ((*files)[i].is_open()){
            
            int numSymbols;
            (*files)[i].read(reinterpret_cast<char*>(&numSymbols), sizeof(int));//prima broj sekcija

            for(int j = 0; j < numSymbols; j++){

                //citamo ime simbola
                string symbolName;
                size_t stringSize;

                (*files)[i].read(reinterpret_cast<char*>(&stringSize), sizeof(std::size_t));
                symbolName.resize(stringSize);
                (*files)[i].read(&symbolName[0], stringSize);


                //citamo ostale info za simbol
                int index, whatSection;
                short value;
                int type;

                (*files)[i].read(reinterpret_cast<char*>(&index), sizeof(int));
                (*files)[i].read(reinterpret_cast<char*>(&whatSection), sizeof(int));
                (*files)[i].read(reinterpret_cast<char*>(&value), sizeof(short));
                (*files)[i].read(reinterpret_cast<char*>(&type), sizeof(int));


                

                //obrada sta god treba (skupovi bla bla)

                

                //Provera dvostruke definicije ili izostanka definicije sa skupovima
                if(type == 1){ //GLOBAL
                    if(d_set.count(symbolName)){ //greska dvostruka def globalnog simbola
                        cerr<<"Greska, dvostruka definicija simbola: "<< symbolName<<endl;
                        exit(-1);
                    }
                    else{ //dodajemo u skup definisanih
                        d_set.insert(symbolName);
                    }

                    if(u_set.count(symbolName)){ //Ako je trenutno oznacen kao nedefinisan
                        u_set.erase(symbolName);
                    }
                }
                else if(type == 2){ //EXTERN
                    if(!d_set.count(symbolName)){ //Nije definisan u d_set, dodajem u u_set
                        u_set.insert(symbolName);
                    }
                }



                //ako je sekcija odbaci, ako je lokalni odbaci, ako je extern odbaci

                if(type == 1){ //Samo ako je globalni simbol dodajemo ga u novu tabelu simbola
                    
                    int newSection;
                    //nalazimo novu vrednost njegovu i kojoj sada novoj sekciji pripada
                    for(int l = 0; l < filesSection->size(); l++){
                        if((*filesSection)[l]->indexOld == whatSection && (*filesSection)[l]->fromFile == i){
                            value = value + (*filesSection)[l]->start_addr;

                            //nalazimo koja je to sekcija sad u tabeli simbola
                            map<string, LinkerSimbolTableEntry*>::iterator nowIsThisSection = linkerSymbolTable->find((*filesSection)[l]->name);
                            if (nowIsThisSection != linkerSymbolTable->end()){
                                newSection = nowIsThisSection->second->index;
                            }

                        }
                    }


                    LinkerSimbolTableEntry *newSymbol = new LinkerSimbolTableEntry(value, type, 0, newSection);
                    linkerSymbolTable->insert({symbolName, newSymbol}); 
                }
                

            }      

        }

    }

    //Ovde je zavrseno citanje tabela simbola svih, treba proveriti da li u_set ostao prazan ako nije greska nedefinisanog
    if(!u_set.empty()){
        cerr<< "Postoje nerazreseni simboli, i to su: ";
        for (const auto& elem : u_set) {
            cerr << elem << " ";
        }
        exit(-1);
    }




  //Cita relokacije

    for(int i = 0; i < argc - start_input; i++){
         
        if ((*files)[i].is_open()){
            
            int numSections;
            (*files)[i].read(reinterpret_cast<char*>(&numSections), sizeof(int));//prima broj sekcija

            for(int j = 0; j < numSections; j++){

                
                int numReloc;
                (*files)[i].read(reinterpret_cast<char*>(&numReloc), sizeof(int));//prima broj relokacija

                for(int k = 0; k < numReloc; k++){
                    //ovde prima relokacije i obradjuje ih
                    int symbolId, patchSectionId, patchLocationCounter, allTypes;
                    short addend;

                    string symbolName;
                    std::size_t stringSize;

                    (*files)[i].read(reinterpret_cast<char*>(&stringSize), sizeof(std::size_t));
                    symbolName.resize(stringSize);
                    (*files)[i].read(&symbolName[0], stringSize);




                    (*files)[i].read(reinterpret_cast<char*>(&symbolId), sizeof(int));
                    (*files)[i].read(reinterpret_cast<char*>(&patchSectionId), sizeof(int));
                    (*files)[i].read(reinterpret_cast<char*>(&patchLocationCounter), sizeof(int));
                    (*files)[i].read(reinterpret_cast<char*>(&addend), sizeof(short));
                    (*files)[i].read(reinterpret_cast<char*>(&allTypes), sizeof(int));
                    // all Types -> globalna apsolutna 0, globalna relativna 1, lokalna aps 2, lokalna relativna 3


                    //razresavamo tu relokaciju
                    //treba ga castovati u short posle
                    
                    int newStartAddressSectionFromRelocation;
                    short combinedSectionAddressFromSymbolTable;
                    int addressToWriteRelocated;
                    vector<char>* binaryCodeVector;

                    //Dobijamo adrese pocetka sekcije iz relokacije i spojene sekcije u kojoj se ona nalazi
                    for(int l = 0; l < filesSection->size(); l++){
                        if((*filesSection)[l]->indexOld == patchSectionId && (*filesSection)[l]->fromFile == i){
                            newStartAddressSectionFromRelocation = (*filesSection)[l]->start_addr;

                            //nalazimo koja je to sekcija sad u tabeli simbola
                            map<string, LinkerSimbolTableEntry*>::iterator nowIsThisSection = linkerSymbolTable->find((*filesSection)[l]->name);
                            if (nowIsThisSection != linkerSymbolTable->end()){
                                combinedSectionAddressFromSymbolTable = nowIsThisSection->second->value;
                                binaryCodeVector = nowIsThisSection->second->bytes;
                            }
                        }
                    }

                    addressToWriteRelocated = patchLocationCounter + newStartAddressSectionFromRelocation - combinedSectionAddressFromSymbolTable;

                    switch(allTypes){
                        case 0:{ //apsolutna globalna

                            map<string, LinkerSimbolTableEntry*>::iterator targetSymbol = linkerSymbolTable->find(symbolName);
                            if (targetSymbol != linkerSymbolTable->end()){
                                short valToWrite = targetSymbol->second->value;
                                (*binaryCodeVector)[addressToWriteRelocated] = (char)(valToWrite & 0xff);
                                (*binaryCodeVector)[addressToWriteRelocated + 1] = (char)((valToWrite >> 8) & 0xff);                             
                            }
                            break;
                        }
                        case 1:{ //PC relativna globalna 

                            map<string, LinkerSimbolTableEntry*>::iterator targetSymbol = linkerSymbolTable->find(symbolName);
                            if (targetSymbol != linkerSymbolTable->end()){

                                unsigned short unsignedSymbolValue = static_cast<unsigned short>(targetSymbol->second->value);
                                unsigned short unsignedAddend = static_cast<unsigned short>(addend);
                                unsigned short unsignedAddressToWriteRelocate = static_cast<unsigned short>(addressToWriteRelocated);

                                int toAddre = unsignedSymbolValue + unsignedAddend - (newStartAddressSectionFromRelocation + patchLocationCounter);

                                (*binaryCodeVector)[addressToWriteRelocated] = (toAddre & 0xff);
                                (*binaryCodeVector)[addressToWriteRelocated + 1] = ((toAddre >> 8) & 0xff);                             
                            }  
                            break;  
                        }
                        case 2:{ //apsolutna lokalna

                            //dohvatamo adresu sekcije u kojoj je simbol definisan
                            for(int l = 0; l < filesSection->size(); l++){
                                if((*filesSection)[l]->indexOld == symbolId && (*filesSection)[l]->fromFile == i){
                                    short valToWrite = (*filesSection)[l]->start_addr + addend;
                                    (*binaryCodeVector)[addressToWriteRelocated] = (char)(valToWrite & 0xff);
                                    (*binaryCodeVector)[addressToWriteRelocated + 1] = (char)((valToWrite >> 8) & 0xff);                             
                                }
                            }
                            break;
                        }
                        case 3:{ //PC relativna lokalna
                            //dohvatamo adresu sekcije u kojoj je simbol definisan
                            for(int l = 0; l < filesSection->size(); l++){
                                if((*filesSection)[l]->indexOld == symbolId && (*filesSection)[l]->fromFile == i){
                                    short valToWrite = (*filesSection)[l]->start_addr - (newStartAddressSectionFromRelocation + patchLocationCounter) + addend;
                                    (*binaryCodeVector)[addressToWriteRelocated] = (char)(valToWrite & 0xff);
                                    (*binaryCodeVector)[addressToWriteRelocated + 1] = (char)((valToWrite >> 8) & 0xff);                             
                                }
                            }
                            break;
                        }


                    }
                }
            }
        }
    }

  

    //Kako izgleda stanje na kraju
    
    // for(int i = 0; i < filesSection->size(); i++){
    //     std::cout << (*filesSection)[i]->name << " " << (*filesSection)[i]->indexOld << " " <<(*filesSection)[i]->fromFile << " " <<(*filesSection)[i]->secSize << " " <<(*filesSection)[i]->start_addr<<endl;
    // }

    // printLinkerSymbolTable();

    toHex(outputName);

//


    for(int i = 0; i < files->size(); i++){
        // (*files)[i].read(reinterpret_cast<char*>(&br), sizeof(int));
        (*files)[i].close();
    }    

    return 0;
}



