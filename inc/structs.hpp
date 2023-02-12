#ifndef _STRUCTS__
#define _STRUCTS__

#include <iostream>
#include <string>
using namespace std;
#include <vector>

extern void print_hex_representation(const std::vector<char>& v);

struct RelocationTableEntry{
  int symbolId; 
  int patchSectionId; 
  int patchLocationCounter; 
  int type; //zavrsena(0) ili nezavrsena(1)
  short addend; 
  int allTypes; //globalna apsolutna 0, globalna relativna 1, lokalna aps 2, lokalna relativna 3
  
  
  RelocationTableEntry(int symbId, int patchSec, int patchLC, int typee, short addendd, int allTypess){
    symbolId = symbId;
    patchSectionId = patchSec;
    patchLocationCounter = patchLC;
    type = typee;
    addend = addendd;
    allTypes = allTypess;
  }

};

struct LineArguments{
    vector<string*> *argName; //ime
    vector<int>  *argType; //0 simbol; 1 literal; 2 registar;
    int adressingType; //10-neposredno literal, 11- neposredno simbol, 12- mem[literal] - tj apsolutno literal, 13 - mem[simbol] apsolutno, 14- mem[PC + simbol], 15- registarsko direktno, 16- registarsko indirektno - mem[reg], 17- mem[reg+literal], 18- mem[reg + simbol]
                      //20-neposredno literal, 21- mem[simbol], 22- mem[PC + simbol], 23 - mem[literal], 24- mem[simbol], 25 - registarsko direktno, 26- mem[reg] - registarsko indirektno, 27- mem[reg+literal], 28- mem[reg+simbol]

    LineArguments(string* name, int type, int adrType){
      argName = new vector<string*>();
      argName->push_back(name);
      argType = new vector<int>();
      argType->push_back(type);
      adressingType = adrType;
    }
};

struct ForwardReferenceTableNode{
  int patchSectionId;
  int patchLocationCounter;
};


struct SymbolTableEntry{
    int index;
    static int indexID;
    int whatSection; //kojoj sekciji pripada -1 -simbol je sekcija 0-undefined 1-absolute 2+ user defined
    short value;
    int type; //0 local; 1 global; 2 noType(globalni uvezeni sa EXTERN); 3 section;
    bool defined; 
    vector<ForwardReferenceTableNode*>* flink; //pokazivac na FRT

    SymbolTableEntry(int locationCnt, int symType, int symSection, bool isDefined){
      index = ++indexID;
      whatSection = symSection;
      value = locationCnt;
      type = symType;
      defined = isDefined;
      flink = new vector<ForwardReferenceTableNode*>();
    }
    
    void print(){
      cout << index << " " << whatSection << " " << value << " " << type << " " << defined << " "<< endl;
    }

};

struct Section{
  string* name;
  int idSection; //redni broj sekcije - redni broj u tabeli simbola
  int location_counter;
  int size; //velicina sekcije u bajtovima
  int startingAddress;
  vector<char>* value; //sadrzaj sekcije
  vector<RelocationTableEntry*>* relocationTableForThisSection;

  Section(string secName, int idSec){
    name = new string(secName);
    idSection = idSec;
    size = 0;
    location_counter = 0;
    value = new vector<char>();
    relocationTableForThisSection = new vector<RelocationTableEntry*>();
  }

  void print(){
    cout<< *name << " " << location_counter << endl;
    cout<< "Vrednosti:"<<endl;
    print_hex_representation(*value);

    for(int k = 0; k < relocationTableForThisSection->size(); k++){
        cout<<endl<< "Id simbola: " << (*relocationTableForThisSection)[k]->symbolId<<endl;
        cout<<"Sekcija u kojoj se patchuje: "<<(*relocationTableForThisSection)[k]->patchSectionId<<endl;
        cout<<"Mesto na kome se patchuje: "<<(*relocationTableForThisSection)[k]->patchLocationCounter<<endl;
        cout<<"Addend: "<<(*relocationTableForThisSection)[k]->addend<<endl;
        cout<<"Tip: "<<(*relocationTableForThisSection)[k]->type<<endl;
        cout<<"Tip: "<<(*relocationTableForThisSection)[k]->allTypes<<endl;          
    }
  }

};



#endif

