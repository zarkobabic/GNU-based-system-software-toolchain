#include "assembler.hpp"
#include <map>
#include <fstream>
#include <cmath>

extern map<string, SymbolTableEntry*> *symbolTable;
extern map<int, Section*> *allSections;
extern int location_counter;
extern Section* activeSection;
extern string outputName;

void print_hex_representation(const std::vector<char>& v) {
  for (char c : v) {
    printf("%02x ", static_cast<unsigned int>(c & 0xff));
  }
  std::puts("");
}

void print_binary_representation(const std::vector<char>& v) {
  for (char c : v) {
    for (int i = 7; i >= 0; --i) {
      std::cout << ((c >> i) & 1);
    }
  }
  std::cout << '\n';
}


/* Directives */

void assemblyLabel(string* label){

   map<string, SymbolTableEntry*>::iterator foundIter = symbolTable->find(*label);
   if (foundIter != symbolTable->end())
   {
     //simbol je pronadjen u tabeli simbola


     //ako je vec definisan
      if(foundIter->second->defined){

        cerr << "Greska: Simbol je vec definisan, dvostruko definisanje!"<<endl;
        exit(-1);

      }else{

        if(foundIter->second->whatSection == 0){ //bio je neki stavljen kao globalan ili je bio referisan unapred a lokalni, sad se dodefinise
          //azuriraj podatke


          foundIter->second->defined = true;
          foundIter->second->value = location_counter;
          foundIter->second->whatSection = activeSection->idSection;

          //PROLAZIMO KROZ SVE RELOKACIONE ZAPISE KOJI SU O OVOM SIMBOLU NAPRAVLJENI RELOKACIJOM UNAPRED
          //IMAJU INDEKS TOG SIMBOLA I TIP IM JE 1-NEZAVRSENE
          map<int, Section*>::iterator itSection;
          for (itSection = allSections->begin(); itSection != allSections->end(); itSection++){
              for(int k = 0; k < itSection->second->relocationTableForThisSection->size(); k++){
                if(((*(itSection->second->relocationTableForThisSection))[k]->symbolId == foundIter->second->index) && ((*(itSection->second->relocationTableForThisSection))[k]->type == 1)){
                  (*(itSection->second->relocationTableForThisSection))[k]->symbolId = foundIter->second->whatSection;
                  (*(itSection->second->relocationTableForThisSection))[k]->addend += foundIter->second->value;
                  (*(itSection->second->relocationTableForThisSection))[k]->type = 0;
                }
              }
          }
          
          


          //backpatching kad se dodefinise ako je vec koriscen u kodu
          for(int i = 0; i < foundIter->second->flink->size(); i++){
            int idSectionToPatch = (*(foundIter->second->flink))[i]->patchSectionId;
            int addressToPatch = (*(foundIter->second->flink))[i]->patchLocationCounter;

            //Ovako dohvatamo tu sekciju u kojoj bi trebalo da se uradi backpatching iz hash mape
            map<int, Section*>::iterator foundedSection = allSections->find(idSectionToPatch);
                if (foundedSection != allSections->end()){
                  
                  
                  //Ako je lokalni uradi backpatching

                  int valueToPatch = foundIter->second->value;
                  
                  //if(foundIter->second->type == 0){ kad bismo hteli da backpatchujemo samo lokalne
                    for(int j = 0; j < 2; j++){
                      (*(foundedSection->second->value))[addressToPatch + j] = (valueToPatch & 0xff);
                      valueToPatch = valueToPatch >> 8;
                    }
                  //}

                }
          }
        }
      }
   }
   else{
     //sigurno nema relokacija
    //simbol nije pronadjen u tabeli simbola

    //sigurno je lokalan simbol koji nije do sada koriscen
    SymbolTableEntry *helper = new SymbolTableEntry(location_counter, 0, activeSection->idSection, true);
    symbolTable->insert({*label, helper});

   }
  // printSymbolTable();
}

void assemblyGlobalDirective(LineArguments* arguments){

  for(int i = 0; i < arguments->argName->size();i++){


    map<string, SymbolTableEntry*>::iterator foundIter = symbolTable->find(*((*(arguments->argName))[i]));
   if (foundIter != symbolTable->end())
   {
     //ako simbol vec postoji u tabeli simbola definisati ga kao globalni
    foundIter->second->type = 1;
   }
   else{
     //ako simbol ne postoji u tabeli simbola onda ga dodati kao undefined
    
    SymbolTableEntry *helper = new SymbolTableEntry(0, 1, 0, false);
    symbolTable->insert({*((*(arguments->argName))[i]), helper});
   }

  }


  // printSymbolTable();

}


void assemblyExternDirective(LineArguments* arguments){
  //posebno oznaciti simbole koji se uvoze i izvoze

  for(int i = 0; i < arguments->argName->size();i++){

    map<string, SymbolTableEntry*>::iterator foundIter = symbolTable->find(*((*(arguments->argName))[i]));
   if (foundIter != symbolTable->end())
   {
     //ako simbol vec postoji u tabeli simbola definisati ga kao globalni uvezeni sa EXTERN
    foundIter->second->type = 2;
   }
   else{
     //ako simbol ne postoji u tabeli simbola onda ga dodati kao undefined globalni uvezeni
    
    SymbolTableEntry *helper = new SymbolTableEntry(0, 2, 0, false);
    symbolTable->insert({*((*(arguments->argName))[i]), helper});
   }

  }
  // printSymbolTable();
}

void assemblySectionDirective(string* sectionName){
  

  //ako sekcija vec postoji samo nastaviti sa upisivanjem njenog sadrzaja, promeniti aktivnu sekciju
  
  //azurirati location counter u trenutnoj sekciji sem na pocetku kad je nullptr
  if(activeSection != nullptr){
    activeSection->location_counter = location_counter;
    activeSection->size = location_counter;
  }
  

   //iz tabele simbola dobijemo redni br te sekcije
  map<string, SymbolTableEntry*>::iterator sectionInSymTable = symbolTable->find(*sectionName);
   if (sectionInSymTable != symbolTable->end())
   {
     //sekcija vec postoji, samo se treba prebaciti na nju
     
    map<int, Section*>::iterator foundedSection = allSections->find(sectionInSymTable->second->index);
                if (foundedSection != allSections->end()){
                  
                  activeSection = foundedSection->second;
                  location_counter = foundedSection->second->location_counter;

                }

   }
   else{
    //Ako sekcija vec ne postoji napraviti je, dodati u tabelu simbola i tabelu sekcija
    
    SymbolTableEntry *helper = new SymbolTableEntry(0, 3, -1, true);
    symbolTable->insert({*sectionName, helper});

    Section *createdSection = new Section(*sectionName, helper->index);
    activeSection = createdSection;
    location_counter = 0;
    allSections->insert({helper->index, createdSection});
   }
   
  //  printSymbolTable();
  //  printAllSections();

}

void assemblyWordDirective(LineArguments* arguments){
  
  for(int i = 0; i < arguments->argName->size();i++){
    string argument_name = (*((*(arguments->argName))[i]));
    int argument_type = (*(arguments->argType))[i];

    if(argument_type== 1){ //kad je literal

    
      int base = 10;
      if (argument_name.substr(0, 2) == "0x") {
          base = 16;
          argument_name = argument_name.substr(2);
      } else if (argument_name[0] == '0') {
          base = 8;
      }
      unsigned long num = stoul(argument_name, nullptr, base);
      
      activeSection->value->push_back((char)(num & 0x00ff));
      activeSection->value->push_back((char)((num & 0xff00)>>8));
    }
    else{ //kad je simbol
      // print_hex_representation(*activeSection->value);
      map<string, SymbolTableEntry*>::iterator foundIter = symbolTable->find(argument_name);
      
      if (foundIter != symbolTable->end()){
        //ako je pronadjen u tabeli simbola treba proveriti da li je global/extern(tj da li je definisan) 
        if(foundIter->second->defined == 0){
          
          
          if(foundIter->second->type == 0){ //lokalni ref unapred, nekad je vec referisan unapred pa ga ima u tabeli simbola
            
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

            //NEZAVRSENA RELOKACIJA ZA LOKALNI PODATAK APSOLUTNO ADRESIRANJE - nezavrsena jer je referisan unapred pa u addend treba dodati krajnju vrednost simbola na kraju
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 1, 0, 2);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          else if(foundIter->second->type == 1 || foundIter->second->type == 2) { //globalni ili extern
            //ako nije definisan upisati 0 i ostaviti relokacoini zapis
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, 0,0);
            activeSection->relocationTableForThisSection->push_back(reHelp);
          }
          
        }
        else{ //definisani lokalni ili globalni

          if(foundIter->second->type == 0){ //lokalni definisan
            int valueContainer = foundIter->second->value;
            activeSection->value->push_back((char)(valueContainer & 0x00ff));
            activeSection->value->push_back((char)((valueContainer & 0xff00)>>8));
          
            //ZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE (kao addend stavljamo njegovu vrednost)
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->whatSection, activeSection->idSection,location_counter, 0, foundIter->second->value, 2);
            activeSection->relocationTableForThisSection->push_back(reHelp);      
          }
          else if(foundIter->second->type == 1 || foundIter->second->type == 2){ // globalni ili extern definisan
           
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, 0, 0);
            activeSection->relocationTableForThisSection->push_back(reHelp);   
          }
        
        }

      }
      else{ //lokalni definisan unapred prvi put

        //opet moguce da je unapred referenciran samo prvi put pa ga ovaj put nema ni u tabeli simbola
        SymbolTableEntry *helper = new SymbolTableEntry(0, 0, 0, false);//lokalni samo definisan unapred
        
        activeSection->value->push_back((char)(0));
        activeSection->value->push_back((char)(0));


        //dodavanje reference unapred ukoliko je bude bilo
          ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
          newNode->patchSectionId = activeSection->idSection;
          newNode->patchLocationCounter = location_counter;
          helper->flink->push_back(newNode);

          //ZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE
          RelocationTableEntry* reHelp = new RelocationTableEntry(helper->index, activeSection->idSection,location_counter, 1, 0, 2);
          activeSection->relocationTableForThisSection->push_back(reHelp);

          symbolTable->insert({argument_name, helper});
      }
    }
    location_counter +=2;
  }
  
  // printAllSections();
  // printSymbolTable();
}

void assemblySkipDirective(string* litteralToSkip){

    int base = 10;
    if (litteralToSkip->substr(0, 2) == "0x") {
        base = 16;
        *litteralToSkip = litteralToSkip->substr(2);
    } else if ((*litteralToSkip)[0] == '0') {
        base = 8;
    }
    unsigned long num = std::stoul(*litteralToSkip, nullptr, base);
    for(int i = 0; i < num; i++){
       activeSection->value->push_back((char)(0));
    }
    location_counter += num;
}

void assemblyEndDirective(){
  if(activeSection != nullptr){
    activeSection->location_counter = location_counter;
    activeSection->size = location_counter;
  }

  ofstream outfile;
    outfile.open(outputName, std::ios::binary);

    if (outfile.is_open()){

      map<int, Section*>::iterator itSection;

      size_t size = allSections->size();
      outfile.write(reinterpret_cast<char*>(&(size)), sizeof(int)); //salje broj sekcija


      for (itSection = allSections->begin(); itSection != allSections->end(); itSection++){
        outfile.write(reinterpret_cast<char*>(&itSection->second->idSection), sizeof(int));
        outfile.write(reinterpret_cast<char*>(&itSection->second->location_counter), sizeof(int));

        size = itSection->second->name->size();
        outfile.write(reinterpret_cast<char*>(&size), sizeof(size_t));
        outfile.write(itSection->second->name->data(), size);

        size = itSection->second->value->size();
        outfile.write(reinterpret_cast<char*>(&size), sizeof(size_t)); //velicina bytes
        outfile.write(itSection->second->value->data(), size); //saljem vector<char>

      }


      //salje tabelu simbola
      map<string, SymbolTableEntry*>::iterator oneSymbol;
      size = symbolTable->size();
      outfile.write(reinterpret_cast<char*>(&(size)), sizeof(int)); //salje broj simbola u tabeli simbola
      

      for (oneSymbol = symbolTable->begin(); oneSymbol != symbolTable->end(); oneSymbol++){

        //saljemo ime
        size = oneSymbol->first.size(); 
        outfile.write(reinterpret_cast<char*>(&size), sizeof(size_t));
        outfile.write(oneSymbol->first.data(), size);

        outfile.write(reinterpret_cast<char*>(&oneSymbol->second->index), sizeof(int));
        outfile.write(reinterpret_cast<char*>(&oneSymbol->second->whatSection), sizeof(int));
        outfile.write(reinterpret_cast<char*>(&oneSymbol->second->value), sizeof(short));
        outfile.write(reinterpret_cast<char*>(&oneSymbol->second->type), sizeof(int));
        //defined ne treba linkeru
      }



      //Slanje relokacija za svaku sekciju
      map<int, Section*>::iterator itSec;

      size = allSections->size();
      outfile.write(reinterpret_cast<char*>(&(size)), sizeof(int)); //salje broj sekcija


      for (itSec = allSections->begin(); itSec != allSections->end(); itSec++){
        
        size_t numRelocations = itSec->second->relocationTableForThisSection->size();
        outfile.write(reinterpret_cast<char*>(&(numRelocations)), sizeof(int)); //salje broj relokacija u toj sekciji
        
        vector<RelocationTableEntry*>* relocations = itSec->second->relocationTableForThisSection;
        for(int j = 0; j < numRelocations; j++){



          //Dohvatamo ime simbola
          
          map<string, SymbolTableEntry*>::iterator oneSymbol;
          for (oneSymbol = symbolTable->begin(); oneSymbol != symbolTable->end(); oneSymbol++){
            if(oneSymbol->second->index == (*relocations)[j]->symbolId){
              
              size_t nameLength = oneSymbol->first.size();
              outfile.write(reinterpret_cast<char*>(&nameLength), sizeof(size_t));
              outfile.write(oneSymbol->first.data(), nameLength);

            }
          }
          
          outfile.write(reinterpret_cast<char*>(&(*relocations)[j]->symbolId), sizeof(int));
          outfile.write(reinterpret_cast<char*>(&(*relocations)[j]->patchSectionId), sizeof(int));
          outfile.write(reinterpret_cast<char*>(&(*relocations)[j]->patchLocationCounter), sizeof(int));
          // outfile.write(reinterpret_cast<char*>(&(*relocations)[j]->type), sizeof(int));
          outfile.write(reinterpret_cast<char*>(&(*relocations)[j]->addend), sizeof(short));
          outfile.write(reinterpret_cast<char*>(&(*relocations)[j]->allTypes), sizeof(int));

        }
        //globalna apsolutna 0, globalna relativna 1, lokalna aps 2, lokalna relativna 3
      }
    
      outfile.close();
    }
    else{
    }
  // print_hex_representation(*activeSection->value);
  // printSymbolTable();
  // printAllSections();

}



/* Instructions */


/* FLOW CONTROL */
void assemblyHALT(){
  activeSection->value->push_back((char)(0));
  location_counter += 1;
}

void assemblyINT(string* registerNum){
  //registerNum je oblika R6 treba nam samo drugi el stringa, tj broj
  int register_num = stoi((*registerNum).substr(1));
  activeSection->value->push_back((char)(0x10));
  activeSection->value->push_back((char)(0x0f | (register_num<<4))); //dobijamo 4f za r=4 
  location_counter += 2;
}

void assemblyIRET(){
  activeSection->value->push_back((char)(0x20));
  location_counter += 1;
}


void translateAddressJumpToMachineCode(LineArguments* arguments, char InstrDescr){
  
  int adrType = (*arguments).adressingType;
  string argValue = *((*(arguments->argName))[0]);
  

  switch(adrType){
    case 20: { //neposredno literal
      
      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(0xf0));

      activeSection->value->push_back((char)(0x00));

      int base = 10;
      if (argValue.substr(0, 2) == "0x") {
          base = 16;
          argValue = argValue.substr(2);
      } else if (argValue[0] == '0') {
          base = 8;
      }
      unsigned long num = stoul(argValue, nullptr, base);

      
      //ovde je literal -> little endian
      activeSection->value->push_back((char)(num & 0xff));
      activeSection->value->push_back((char)((num & 0xff00)>>8));
      location_counter += 5;
      break;
    }




    case 21: { //neposredno simbol

      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(0xf0));

      activeSection->value->push_back((char)(0x00));
      location_counter += 3;

      //vrednost simbola
      map<string, SymbolTableEntry*>::iterator foundIter = symbolTable->find(argValue);
      if (foundIter != symbolTable->end()){
        //ako je pronadjen u tabeli simbola treba proveriti da li je global/extern(tj da li je definisan) 
        if(foundIter->second->defined == 0){
          
          
          if(foundIter->second->type == 0){ //lokalni ref unapred, nekad je vec referisan unapred pa ga ima u tabeli simbola
            
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

            //NEZAVRSENA RELOKACIJA ZA LOKALNI PODATAK APSOLUTNO ADRESIRANJE - nezavrsena jer je referisan unapred pa u addend treba dodati krajnju vrednost simbola na kraju
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 1, 0, 2);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          else if(foundIter->second->type == 1 || foundIter->second->type == 2) { //globalni ili extern
            //ako nije definisan upisati 0 i ostaviti relokacoini zapis
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, 0, 0);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          
        }
        else{ //definisani lokalni ili globalni

          if(foundIter->second->type == 0){ //lokalni definisan
            int valueContainer = foundIter->second->value;
            activeSection->value->push_back((char)(valueContainer & 0xff));
            activeSection->value->push_back((char)((valueContainer & 0xff00)>>8));
          
            //ZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE (kao addend stavljamo njegovu vrednost)
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->whatSection, activeSection->idSection,location_counter, 0, foundIter->second->value, 2);
            activeSection->relocationTableForThisSection->push_back(reHelp);      
          }
          else if(foundIter->second->type == 1){ // globalni definisan
           
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, 0, 0);
            activeSection->relocationTableForThisSection->push_back(reHelp);   
          }
        
        }

      }
      else{ //lokalni definisan unapred prvi put

        //opet moguce da je unapred referenciran samo prvi put pa ga ovaj put nema ni u tabeli simbola
        SymbolTableEntry *helper = new SymbolTableEntry(0, 0, 0, false);//lokalni samo definisan unapred
        
        activeSection->value->push_back((char)(0));
        activeSection->value->push_back((char)(0));


        //dodavanje reference unapred ukoliko je bude bilo
          ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
          newNode->patchSectionId = activeSection->idSection;
          newNode->patchLocationCounter = location_counter;
          helper->flink->push_back(newNode);

          //ZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE
          RelocationTableEntry* reHelp = new RelocationTableEntry(helper->index, activeSection->idSection,location_counter, 1, 0, 2);
          activeSection->relocationTableForThisSection->push_back(reHelp);

          symbolTable->insert({argValue, helper});
      }

      location_counter += 2;
      break;
    }

    case 22: { //PC relativno

      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(0xf7));

      activeSection->value->push_back((char)(0x05));
      location_counter += 3;

      //vrednost simbola
      map<string, SymbolTableEntry*>::iterator foundIter = symbolTable->find(argValue);
      if (foundIter != symbolTable->end()){
        //ako je pronadjen u tabeli simbola treba proveriti da li je global/extern(tj da li je definisan) 
        if(foundIter->second->defined == 0){
          
          
          if(foundIter->second->type == 0){ //lokalni ref unapred, nekad je vec referisan unapred pa ga ima u tabeli simbola
            
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

            //NEZAVRSENA RELOKACIJA ZA LOKALNI PODATAK APSOLUTNO ADRESIRANJE - nezavrsena jer je referisan unapred pa u addend treba dodati krajnju vrednost simbola na kraju
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 1, -2, 3);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          else if(foundIter->second->type == 1 || foundIter->second->type == 2) { //globalni ili extern
            //ako nije definisan upisati 0 i ostaviti relokacoini zapis
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, -2, 1);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          
        }
        else{ //definisani lokalni ili globalni

          if(foundIter->second->type == 0){ //lokalni definisan
            int valueContainer = foundIter->second->value;
            activeSection->value->push_back((char)(valueContainer & 0xff));
            activeSection->value->push_back((char)((valueContainer & 0xff00)>>8));
          
            //ZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE (kao addend stavljamo njegovu vrednost)
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->whatSection, activeSection->idSection,location_counter, 0, foundIter->second->value - 2, 3);
            activeSection->relocationTableForThisSection->push_back(reHelp);      
          }
          else if(foundIter->second->type == 1){ // globalni definisan
           
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, -2, 1);
            activeSection->relocationTableForThisSection->push_back(reHelp);   
          }
        
        }

      }
      else{ //lokalni definisan unapred prvi put

        //opet moguce da je unapred referenciran samo prvi put pa ga ovaj put nema ni u tabeli simbola
        SymbolTableEntry *helper = new SymbolTableEntry(0, 0, 0, false);//lokalni samo definisan unapred
        
        activeSection->value->push_back((char)(0));
        activeSection->value->push_back((char)(0));


        //dodavanje reference unapred ukoliko je bude bilo
          ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
          newNode->patchSectionId = activeSection->idSection;
          newNode->patchLocationCounter = location_counter;
          helper->flink->push_back(newNode);

          //ZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE
          RelocationTableEntry* reHelp = new RelocationTableEntry(helper->index, activeSection->idSection,location_counter, 1, -2, 3);
          activeSection->relocationTableForThisSection->push_back(reHelp);

          symbolTable->insert({argValue, helper});
      }

      location_counter += 2;
      break;
    }



    case 23:{ //mem direktno literal -> *<literal>

      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(0xf0));

      activeSection->value->push_back((char)(0x04));

      int base = 10;
      if (argValue.substr(0, 2) == "0x") {
          base = 16;
          argValue = argValue.substr(2);
      } else if (argValue[0] == '0') {
          base = 8;
      }
      unsigned long num = stoul(argValue, nullptr, base);

      
      //ovde je literal -> little endian
      activeSection->value->push_back((char)(num & 0xff));
      activeSection->value->push_back((char)((num & 0xff00)>>8));
      location_counter += 5;
      break;
    }


    case 24:{ //vrednost mem na adresi simbol -> *<symbol>
      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(0xf0));

      activeSection->value->push_back((char)(0x04));
      location_counter += 3;

      //vrednost simbola
      map<string, SymbolTableEntry*>::iterator foundIter = symbolTable->find(argValue);
      if (foundIter != symbolTable->end()){
        //ako je pronadjen u tabeli simbola treba proveriti da li je global/extern(tj da li je definisan) 
        if(foundIter->second->defined == 0){
          
          
          if(foundIter->second->type == 0){ //lokalni ref unapred, nekad je vec referisan unapred pa ga ima u tabeli simbola
            
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

            //NEZAVRSENA RELOKACIJA ZA LOKALNI PODATAK APSOLUTNO ADRESIRANJE - nezavrsena jer je referisan unapred pa u addend treba dodati krajnju vrednost simbola na kraju
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 1, 0, 2);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          else if(foundIter->second->type == 1 || foundIter->second->type == 2) { //globalni ili extern
            //ako nije definisan upisati 0 i ostaviti relokacoini zapis
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, 0,0);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          
        }
        else{ //definisani lokalni ili globalni

          if(foundIter->second->type == 0){ //lokalni definisan
            int valueContainer = foundIter->second->value;
            activeSection->value->push_back((char)(valueContainer & 0xff));
            activeSection->value->push_back((char)((valueContainer & 0xff00)>>8));
          
            //ZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE (kao addend stavljamo njegovu vrednost)
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->whatSection, activeSection->idSection,location_counter, 0, foundIter->second->value, 2);
            activeSection->relocationTableForThisSection->push_back(reHelp);      
          }
          else if(foundIter->second->type == 1){ // globalni definisan
           
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, 0, 0);
            activeSection->relocationTableForThisSection->push_back(reHelp);   
          }
        
        }

      }
      else{ //lokalni definisan unapred prvi put

        //opet moguce da je unapred referenciran samo prvi put pa ga ovaj put nema ni u tabeli simbola
        SymbolTableEntry *helper = new SymbolTableEntry(0, 0, 0, false);//lokalni samo definisan unapred
        
        activeSection->value->push_back((char)(0));
        activeSection->value->push_back((char)(0));


        //dodavanje reference unapred ukoliko je bude bilo
          ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
          newNode->patchSectionId = activeSection->idSection;
          newNode->patchLocationCounter = location_counter;
          helper->flink->push_back(newNode);

          //ZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE
          RelocationTableEntry* reHelp = new RelocationTableEntry(helper->index, activeSection->idSection,location_counter, 1, 0, 2);
          activeSection->relocationTableForThisSection->push_back(reHelp);

          symbolTable->insert({argValue, helper});
      }

      location_counter += 2;
      
      break;
    }

    case 25: { //registarsko direktno

      int register_num = stoi((argValue).substr(1));
      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(0xf<<4 | register_num)); //dobijamo f4 za r=4 
      activeSection->value->push_back((char)(0x01));
      location_counter += 3;
      break;
    }

    case 26: { //registarsko indirektno

      int register_num = stoi((argValue).substr(1));
      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(0xf<<4 | register_num)); //dobijamo f4 za r=4 
      activeSection->value->push_back((char)(0x02));
      
      location_counter += 3;
      break;
    }

    case 27: { //registarsko indirektno sa pomerajem u vidu literala
      string secondArgValue = *((*(arguments->argName))[1]); //samo za registarska indirektna
      int register_num = stoi((argValue).substr(1));
      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(0xf<<4 | register_num)); //dobijamo f4 za r=4  
      activeSection->value->push_back((char)(0x03));

      int base = 10;
      if (secondArgValue.substr(0, 2) == "0x") {
          base = 16;
          secondArgValue = secondArgValue.substr(2);
      } else if (secondArgValue[0] == '0') {
          base = 8;
      }
      unsigned long num = stoul(secondArgValue, nullptr, base);

      
      activeSection->value->push_back((char)(num & 0xff));
      activeSection->value->push_back((char)((num & 0xff00)>>8));

      location_counter += 5;
     
      break;
    }

    case 28: { //Registarsko indirektno sa pomerajem u vidu simbola

      string secondArgValue = *((*(arguments->argName))[1]); //samo za registarska indirektna
      int register_num = stoi((argValue).substr(1));
      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(0xf<<4 | register_num)); //dobijamo f4 za r=4 
      activeSection->value->push_back((char)(0x03));

      location_counter += 3;

      //vrednost simbola
      map<string, SymbolTableEntry*>::iterator foundIter = symbolTable->find(secondArgValue);
      if (foundIter != symbolTable->end()){
        //ako je pronadjen u tabeli simbola treba proveriti da li je global/extern(tj da li je definisan) 
        if(foundIter->second->defined == 0){
          
          
          if(foundIter->second->type == 0){ //lokalni ref unapred, nekad je vec referisan unapred pa ga ima u tabeli simbola
            
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

            //NEZAVRSENA RELOKACIJA ZA LOKALNI PODATAK APSOLUTNO ADRESIRANJE - nezavrsena jer je referisan unapred pa u addend treba dodati krajnju vrednost simbola na kraju
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 1, 0, 2);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          else if(foundIter->second->type == 1 || foundIter->second->type == 2) { //globalni ili extern
            //ako nije definisan upisati 0 i ostaviti relokacoini zapis
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, 0, 0);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          
        }
        else{ //definisani lokalni ili globalni

          if(foundIter->second->type == 0){ //lokalni definisan
            int valueContainer = foundIter->second->value;
            activeSection->value->push_back((char)(valueContainer & 0xff));
            activeSection->value->push_back((char)((valueContainer & 0xff00)>>8));
          
            //ZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE (kao addend stavljamo njegovu vrednost)
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->whatSection, activeSection->idSection,location_counter, 0, foundIter->second->value, 2);
            activeSection->relocationTableForThisSection->push_back(reHelp);      
          }
          else if(foundIter->second->type == 1){ // globalni definisan
           
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, 0, 0);
            activeSection->relocationTableForThisSection->push_back(reHelp);   
          }
        
        }

      }
      else{ //lokalni definisan unapred prvi put

        //opet moguce da je unapred referenciran samo prvi put pa ga ovaj put nema ni u tabeli simbola
        SymbolTableEntry *helper = new SymbolTableEntry(0, 0, 0, false);//lokalni samo definisan unapred
        
        activeSection->value->push_back((char)(0));
        activeSection->value->push_back((char)(0));


        //dodavanje reference unapred ukoliko je bude bilo
          ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
          newNode->patchSectionId = activeSection->idSection;
          newNode->patchLocationCounter = location_counter;
          helper->flink->push_back(newNode);

          //NEZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE
          RelocationTableEntry* reHelp = new RelocationTableEntry(helper->index, activeSection->idSection,location_counter, 1, 0, 2);
          activeSection->relocationTableForThisSection->push_back(reHelp);


          symbolTable->insert({secondArgValue, helper});
      }

      location_counter += 2;
      // print_hex_representation(*activeSection->value);
      break;
    }


  }
}


void assemblyCALL(LineArguments* arguments){
  translateAddressJumpToMachineCode(arguments, ((char)(0x30)));
}

void assemblyRET(){
  activeSection->value->push_back((char)(0x40));
  location_counter += 1;
}


/* JUMPS */
void assemblyJMP(LineArguments* arguments){
  translateAddressJumpToMachineCode(arguments, ((char)(0x50)));
}

void assemblyJEQ(LineArguments* arguments){
  translateAddressJumpToMachineCode(arguments, ((char)(0x51)));
}

void assemblyJNE(LineArguments* arguments){
  translateAddressJumpToMachineCode(arguments, ((char)(0x52)));
}

void assemblyJGT(LineArguments* arguments){
  translateAddressJumpToMachineCode(arguments, ((char)(0x53)));
}


/* VALUES MANIPULATION*/
void assemblyPUSH(string* registerNum){
  int register_num = stoi((*registerNum).substr(1));
  activeSection->value->push_back((char)(0xB0));
  activeSection->value->push_back((char)(0x6 | (register_num<<4))); //dobijamo 46 za r=4 
  activeSection->value->push_back((char)(0x12));
  location_counter += 3;
}

void assemblyPOP(string* registerNum){
  int register_num = stoi((*registerNum).substr(1));
  activeSection->value->push_back((char)(0xA0));
  activeSection->value->push_back((char)(0x6 | (register_num<<4))); //dobijamo 46 za r=4 
  activeSection->value->push_back((char)(0x42));
  location_counter += 3;
}

void assemblyXCHG(string* registerDstNum, string* registerSrcNum){
  
  int register_dst = stoi((*registerDstNum).substr(1));
  int register_src = stoi((*registerSrcNum).substr(1));
  activeSection->value->push_back((char)(0x60));
  activeSection->value->push_back((char)((register_src & 0xf) | (register_dst<<4))); //dobijamo 4f za r=4 
  location_counter += 2;
}


/* ARITMETIC */
void assemblyADD(string* registerDstNum, string* registerSrcNum){
  int register_dst = stoi((*registerDstNum).substr(1));
  int register_src = stoi((*registerSrcNum).substr(1));
  activeSection->value->push_back((char)(0x70));
  activeSection->value->push_back((char)((register_src & 0xf) | (register_dst<<4))); //dobijamo 4f za r=4 
  location_counter += 2;
}

void assemblySUB(string* registerDstNum, string* registerSrcNum){
    
  int register_dst = stoi((*registerDstNum).substr(1));
  int register_src = stoi((*registerSrcNum).substr(1));
  activeSection->value->push_back((char)(0x71));
  activeSection->value->push_back((char)((register_src & 0xf) | (register_dst<<4))); //dobijamo 4f za r=4 
  location_counter += 2;
}

void assemblyMUL(string* registerDstNum, string* registerSrcNum){
  int register_dst = stoi((*registerDstNum).substr(1));
  int register_src = stoi((*registerSrcNum).substr(1));
  activeSection->value->push_back((char)(0x72));
  activeSection->value->push_back((char)((register_src & 0xf) | (register_dst<<4))); //dobijamo 4f za r=4 
  location_counter += 2;
}

void assemblyDIV(string* registerDstNum, string* registerSrcNum){
  int register_dst = stoi((*registerDstNum).substr(1));
  int register_src = stoi((*registerSrcNum).substr(1));
  activeSection->value->push_back((char)(0x73));
  activeSection->value->push_back((char)((register_src & 0xf) | (register_dst<<4))); //dobijamo 4f za r=4 
  location_counter += 2;
}



/* LOGICAL */
void assemblyCMP(string* registerDstNum, string* registerSrcNum){
  int register_dst = stoi((*registerDstNum).substr(1));
  int register_src = stoi((*registerSrcNum).substr(1));
  activeSection->value->push_back((char)(0x74));
  activeSection->value->push_back((char)((register_src & 0xf) | (register_dst<<4))); //dobijamo 4f za r=4 
  location_counter += 2;
}

void assemblyNOT(string* registerNum){
  int register_num = stoi((*registerNum).substr(1));
  activeSection->value->push_back((char)(0x80));
  activeSection->value->push_back((char)(0x0 | (register_num<<4))); //dobijamo 4f za r=4 
  location_counter += 2;
}

void assemblyAND(string* registerDstNum, string* registerSrcNum){
  int register_dst = stoi((*registerDstNum).substr(1));
  int register_src = stoi((*registerSrcNum).substr(1));
  activeSection->value->push_back((char)(0x81));
  activeSection->value->push_back((char)((register_src & 0xf) | (register_dst<<4))); //dobijamo 4f za r=4 
  location_counter += 2;
}

void assemblyOR(string* registerDstNum, string* registerSrcNum){
  int register_dst = stoi((*registerDstNum).substr(1));
  int register_src = stoi((*registerSrcNum).substr(1));
  activeSection->value->push_back((char)(0x82));
  activeSection->value->push_back((char)((register_src & 0xf) | (register_dst<<4))); //dobijamo 4f za r=4 
  location_counter += 2;
}

void assemblyXOR(string* registerDstNum, string* registerSrcNum){
  int register_dst = stoi((*registerDstNum).substr(1));
  int register_src = stoi((*registerSrcNum).substr(1));
  activeSection->value->push_back((char)(0x83));
  activeSection->value->push_back((char)((register_src & 0xf) | (register_dst<<4))); //dobijamo 4f za r=4 
  location_counter += 2;
}

void assemblyTEST(string* registerDstNum, string* registerSrcNum){
  int register_dst = stoi((*registerDstNum).substr(1));
  int register_src = stoi((*registerSrcNum).substr(1));
  activeSection->value->push_back((char)(0x84));
  activeSection->value->push_back((char)((register_src & 0xf) | (register_dst<<4))); //dobijamo 4f za r=4 
  location_counter += 2;
}


/* SHIFTING */
void assemblySHL(string* registerDstNum, string* registerSrcNum){
  int register_dst = stoi((*registerDstNum).substr(1));
  int register_src = stoi((*registerSrcNum).substr(1));
  activeSection->value->push_back((char)(0x90));
  activeSection->value->push_back((char)((register_src & 0xf) | (register_dst<<4))); //dobijamo 4f za r=4 
  location_counter += 2;
}

void assemblySHR(string* registerDstNum, string* registerSrcNum){
  int register_dst = stoi((*registerDstNum).substr(1));
  int register_src = stoi((*registerSrcNum).substr(1));
  activeSection->value->push_back((char)(0x91));
  activeSection->value->push_back((char)((register_src & 0xf) | (register_dst<<4))); //dobijamo 4f za r=4 
  location_counter += 2;
}

void translateAddressDataToMachineCode(string* registerToLoad, LineArguments* srcOperand, char InstrDescr){

  int adrType = srcOperand->adressingType;
  int register_dst = stoi((*registerToLoad).substr(1));
  string argValue = *((*(srcOperand->argName))[0]);
  
  switch (adrType)
  {
    case 10: { //neposredno literal
      
      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(register_dst<<4)); //dobijamo 40 za r=4 
      activeSection->value->push_back((char)(0x00));

      int base = 10;
      if (argValue.substr(0, 2) == "0x") {
          base = 16;
          argValue = argValue.substr(2);
      } else if (argValue[0] == '0') {
          base = 8;
      }
      unsigned long num = stoul(argValue, nullptr, base);
      
      activeSection->value->push_back((char)(num & 0xff));
      activeSection->value->push_back((char)((num & 0xff00)>>8));

      location_counter += 5;
      break;
    }

    case 11: { //neposredno simbol
      
      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(register_dst<<4)); //dobijamo 40 za r=4
      activeSection->value->push_back((char)(0x00));
      location_counter += 3;

      //vrednost simbola
      map<string, SymbolTableEntry*>::iterator foundIter = symbolTable->find(argValue);
      if (foundIter != symbolTable->end()){
        //ako je pronadjen u tabeli simbola treba proveriti da li je global/extern(tj da li je definisan) 
        if(foundIter->second->defined == 0){
          
          
          if(foundIter->second->type == 0){ //lokalni ref unapred, nekad je vec referisan unapred pa ga ima u tabeli simbola
            
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

            //NEZAVRSENA RELOKACIJA ZA LOKALNI PODATAK APSOLUTNO ADRESIRANJE - nezavrsena jer je referisan unapred pa u addend treba dodati krajnju vrednost simbola na kraju
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 1, 0, 2);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          else if(foundIter->second->type == 1 || foundIter->second->type == 2) { //globalni ili extern
            //ako nije definisan upisati 0 i ostaviti relokacoini zapis
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, 0, 0);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          
        }
        else{ //definisani lokalni ili globalni

          if(foundIter->second->type == 0){ //lokalni definisan
            int valueContainer = foundIter->second->value;
            activeSection->value->push_back((char)(valueContainer & 0xff));
            activeSection->value->push_back((char)((valueContainer & 0xff00)>>8));
          
            //ZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE (kao addend stavljamo njegovu vrednost)
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->whatSection, activeSection->idSection,location_counter, 0, foundIter->second->value, 2);
            activeSection->relocationTableForThisSection->push_back(reHelp);      
          }
          else if(foundIter->second->type == 1){ // globalni definisan
           
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, 0, 0);
            activeSection->relocationTableForThisSection->push_back(reHelp);   
          }
        
        }

      }
      else{ //lokalni definisan unapred prvi put

        //opet moguce da je unapred referenciran samo prvi put pa ga ovaj put nema ni u tabeli simbola
        SymbolTableEntry *helper = new SymbolTableEntry(0, 0, 0, false);//lokalni samo definisan unapred
        
        activeSection->value->push_back((char)(0));
        activeSection->value->push_back((char)(0));


        //dodavanje reference unapred ukoliko je bude bilo
          ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
          newNode->patchSectionId = activeSection->idSection;
          newNode->patchLocationCounter = location_counter;
          helper->flink->push_back(newNode);

          //ZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE
          RelocationTableEntry* reHelp = new RelocationTableEntry(helper->index, activeSection->idSection,location_counter, 1, 0, 2);
          activeSection->relocationTableForThisSection->push_back(reHelp);

          symbolTable->insert({argValue, helper});
      }

      location_counter += 2;
      break;
    }
  
    case 12: { //mem[literal] vrednost iz memorije na adresi literal -> <literal>

      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(register_dst<<4)); //dobijamo 40 za r=4  
      activeSection->value->push_back((char)(0x04));

      int base = 10;
      if (argValue.substr(0, 2) == "0x") {
          base = 16;
          argValue = argValue.substr(2);
      } else if (argValue[0] == '0') {
          base = 8;
      }
      unsigned long num = stoul(argValue, nullptr, base);

      
      //ovde je literal -> little endian
      activeSection->value->push_back((char)(num & 0xff));
      activeSection->value->push_back((char)((num & 0xff00)>>8));
      location_counter += 5;
      break;
    }

    case 13: { //mem[simbol] vrednost iz memorije na adresi simbol -> <symbol>
      
      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(register_dst<<4)); //dobijamo 40 za r=4  
      activeSection->value->push_back((char)(0x04));
      location_counter += 3;

      //vrednost simbola
      map<string, SymbolTableEntry*>::iterator foundIter = symbolTable->find(argValue);
      if (foundIter != symbolTable->end()){
        //ako je pronadjen u tabeli simbola treba proveriti da li je global/extern(tj da li je definisan) 
        if(foundIter->second->defined == 0){
          
          
          if(foundIter->second->type == 0){ //lokalni ref unapred, nekad je vec referisan unapred pa ga ima u tabeli simbola
            
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

            //NEZAVRSENA RELOKACIJA ZA LOKALNI PODATAK APSOLUTNO ADRESIRANJE - nezavrsena jer je referisan unapred pa u addend treba dodati krajnju vrednost simbola na kraju
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 1, 0, 2);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          else if(foundIter->second->type == 1 || foundIter->second->type == 2) { //globalni ili extern
            //ako nije definisan upisati 0 i ostaviti relokacoini zapis
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, 0, 0);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          
        }
        else{ //definisani lokalni ili globalni

          if(foundIter->second->type == 0){ //lokalni definisan
            int valueContainer = foundIter->second->value;
            activeSection->value->push_back((char)(valueContainer & 0xff));
            activeSection->value->push_back((char)((valueContainer & 0xff00)>>8));
          
            //ZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE (kao addend stavljamo njegovu vrednost)
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->whatSection, activeSection->idSection,location_counter, 0, foundIter->second->value, 2);
            activeSection->relocationTableForThisSection->push_back(reHelp);      
          }
          else if(foundIter->second->type == 1){ // globalni definisan
           
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, 0, 0);
            activeSection->relocationTableForThisSection->push_back(reHelp);   
          }
        
        }

      }
      else{ //lokalni definisan unapred prvi put

        //opet moguce da je unapred referenciran samo prvi put pa ga ovaj put nema ni u tabeli simbola
        SymbolTableEntry *helper = new SymbolTableEntry(0, 0, 0, false);//lokalni samo definisan unapred
        
        activeSection->value->push_back((char)(0));
        activeSection->value->push_back((char)(0));


        //dodavanje reference unapred ukoliko je bude bilo
          ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
          newNode->patchSectionId = activeSection->idSection;
          newNode->patchLocationCounter = location_counter;
          helper->flink->push_back(newNode);

          //ZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE
          RelocationTableEntry* reHelp = new RelocationTableEntry(helper->index, activeSection->idSection,location_counter, 1, 0, 2);
          activeSection->relocationTableForThisSection->push_back(reHelp);

          symbolTable->insert({argValue, helper});
      }

      location_counter += 2;
      
      break;
    }

    case 14: { //PC relativno

      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(0x7 | (register_dst<<4))); //dobijamo 46 za r=4 
      activeSection->value->push_back((char)(0x03));
      location_counter += 3;

      //vrednost simbola
      map<string, SymbolTableEntry*>::iterator foundIter = symbolTable->find(argValue);
      if (foundIter != symbolTable->end()){
        //ako je pronadjen u tabeli simbola treba proveriti da li je global/extern(tj da li je definisan) 
        if(foundIter->second->defined == 0){
          
          
          if(foundIter->second->type == 0){ //lokalni ref unapred, nekad je vec referisan unapred pa ga ima u tabeli simbola
            
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

            //NEZAVRSENA RELOKACIJA ZA LOKALNI PODATAK APSOLUTNO ADRESIRANJE - nezavrsena jer je referisan unapred pa u addend treba dodati krajnju vrednost simbola na kraju
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 1, -2, 3);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          else if(foundIter->second->type == 1 || foundIter->second->type == 2) { //globalni ili extern
            //ako nije definisan upisati 0 i ostaviti relokacoini zapis
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);
           
           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, -2, 1);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          
        }
        else{ //definisani lokalni ili globalni

          if(foundIter->second->type == 0){ //lokalni definisan
            int valueContainer = foundIter->second->value;
            activeSection->value->push_back((char)(valueContainer & 0xff));
            activeSection->value->push_back((char)((valueContainer & 0xff00)>>8));
          
            //ZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE (kao addend stavljamo njegovu vrednost)
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->whatSection, activeSection->idSection,location_counter, 0, foundIter->second->value - 2, 3);
            activeSection->relocationTableForThisSection->push_back(reHelp);      
          }
          else if(foundIter->second->type == 1){ // globalni definisan
           
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, -2, 1);
            activeSection->relocationTableForThisSection->push_back(reHelp);   
          }
        
        }

      }
      else{ //lokalni definisan unapred prvi put

        //opet moguce da je unapred referenciran samo prvi put pa ga ovaj put nema ni u tabeli simbola
        SymbolTableEntry *helper = new SymbolTableEntry(0, 0, 0, false);//lokalni samo definisan unapred
        
        activeSection->value->push_back((char)(0));
        activeSection->value->push_back((char)(0));


        //dodavanje reference unapred ukoliko je bude bilo
          ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
          newNode->patchSectionId = activeSection->idSection;
          newNode->patchLocationCounter = location_counter;
          helper->flink->push_back(newNode);

          //ZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE
          RelocationTableEntry* reHelp = new RelocationTableEntry(helper->index, activeSection->idSection,location_counter, 1, -2, 3);
          activeSection->relocationTableForThisSection->push_back(reHelp);

          symbolTable->insert({argValue, helper});
      }

      location_counter += 2;
      break;
    }

    case 15: { //registarsko direktno
      int register_num = stoi((argValue).substr(1));
      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(register_num | (register_dst<<4))); 
      activeSection->value->push_back((char)(0x01));
      location_counter += 3;
      break;
    }

    case 16: { //registarsko indirektno
      int register_num = stoi((argValue).substr(1));
      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(register_num | (register_dst<<4))); 
      activeSection->value->push_back((char)(0x02));
      location_counter += 3;
      break;
    }

    case 17: { //registarsko indirektno sa pomerajem u vidu literala
      string secondArgValue = *((*(srcOperand->argName))[1]); //samo za registarska indirektna
      int register_num = stoi((argValue).substr(1));
      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(register_num | (register_dst<<4))); 
      activeSection->value->push_back((char)(0x03));

      int base = 10;
      if (secondArgValue.substr(0, 2) == "0x") {
          base = 16;
          secondArgValue = secondArgValue.substr(2);
      } else if (secondArgValue[0] == '0') {
          base = 8;
      }
      unsigned long num = stoul(secondArgValue, nullptr, base);

      
      activeSection->value->push_back((char)(num & 0xff));
      activeSection->value->push_back((char)((num & 0xff00)>>8));

      location_counter += 5;
     
      break;
    }

    case 18: { //Registarsko indirektno sa pomerajem u vidu simbola

      string secondArgValue = *((*(srcOperand->argName))[1]); //samo za registarska indirektna
      int register_num = stoi((argValue).substr(1));
      activeSection->value->push_back(InstrDescr);
      activeSection->value->push_back((char)(register_num | (register_dst<<4))); 
      activeSection->value->push_back((char)(0x03));

      location_counter += 3;

      //vrednost simbola
      map<string, SymbolTableEntry*>::iterator foundIter = symbolTable->find(secondArgValue);
      if (foundIter != symbolTable->end()){
        //ako je pronadjen u tabeli simbola treba proveriti da li je global/extern(tj da li je definisan) 
        if(foundIter->second->defined == 0){
          
          
          if(foundIter->second->type == 0){ //lokalni ref unapred, nekad je vec referisan unapred pa ga ima u tabeli simbola
            
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

            //NEZAVRSENA RELOKACIJA ZA LOKALNI PODATAK APSOLUTNO ADRESIRANJE - nezavrsena jer je referisan unapred pa u addend treba dodati krajnju vrednost simbola na kraju
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 1, 0, 2);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          else if(foundIter->second->type == 1 || foundIter->second->type == 2) { //globalni ili extern
            //ako nije definisan upisati 0 i ostaviti relokacoini zapis
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

            //dodavanje reference unapred ukoliko je bude bilo
            ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
            newNode->patchSectionId = activeSection->idSection;
            newNode->patchLocationCounter = location_counter;
            foundIter->second->flink->push_back(newNode);

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, 0, 0);
            activeSection->relocationTableForThisSection->push_back(reHelp);

          }
          
        }
        else{ //definisani lokalni ili globalni

          if(foundIter->second->type == 0){ //lokalni definisan
            int valueContainer = foundIter->second->value;
            activeSection->value->push_back((char)(valueContainer & 0xff));
            activeSection->value->push_back((char)((valueContainer & 0xff00)>>8));
          
            //ZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE (kao addend stavljamo njegovu vrednost)
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->whatSection, activeSection->idSection,location_counter, 0, foundIter->second->value, 2);
            activeSection->relocationTableForThisSection->push_back(reHelp);      
          }
          else if(foundIter->second->type == 1){ // globalni definisan
           
            activeSection->value->push_back((char)(0));
            activeSection->value->push_back((char)(0));

           //ZAVRSENA RELOKACIJA ZA GLOBALNI PODATAK-APSOLUTNO ADRESIRANJE
            RelocationTableEntry* reHelp = new RelocationTableEntry(foundIter->second->index, activeSection->idSection,location_counter, 0, 0, 0);
            activeSection->relocationTableForThisSection->push_back(reHelp);   
          }
        
        }

      }
      else{ //lokalni definisan unapred prvi put

        //opet moguce da je unapred referenciran samo prvi put pa ga ovaj put nema ni u tabeli simbola
        SymbolTableEntry *helper = new SymbolTableEntry(0, 0, 0, false);//lokalni samo definisan unapred
        
        activeSection->value->push_back((char)(0));
        activeSection->value->push_back((char)(0));


        //dodavanje reference unapred ukoliko je bude bilo
          ForwardReferenceTableNode* newNode = new ForwardReferenceTableNode();
          newNode->patchSectionId = activeSection->idSection;
          newNode->patchLocationCounter = location_counter;
          helper->flink->push_back(newNode);

          //NEZAVRSENA RELOKACIJA ZA LOKALNI PODATAK-APSOLUTNO ADRESIRANJE
          RelocationTableEntry* reHelp = new RelocationTableEntry(helper->index, activeSection->idSection,location_counter, 1, 0, 2);
          activeSection->relocationTableForThisSection->push_back(reHelp);


          symbolTable->insert({secondArgValue, helper});
      }

      location_counter += 2;
      // print_hex_representation(*activeSection->value);
      break;
    }


  }

}


void assemblyLDR(string* registerToLoad, LineArguments* srcOperand){
  translateAddressDataToMachineCode(registerToLoad, srcOperand, ((char)(0xA0)));
}

void assemblySTR(string* registerToLoad, LineArguments* srcOperand){
  translateAddressDataToMachineCode(registerToLoad, srcOperand, ((char)(0xB0)));
}

void printSymbolTable(){

  //ISCITAVANJE TABELE SIMBOLA (HASH MAPE)
  std::cout<< "labela" << " " <<"indeks" <<" " << "sekcija" << " " << "vrednost" << " "<< "tip" << " " << "definisan";
  for(auto it = symbolTable->cbegin(); it != symbolTable->cend(); ++it){
      std::cout <<"\n"<< it->first << " "; 
      it->second->print();
  }
  //---------------------------------------------

}

void printAllSections(){
  std::cout<< "idSekcije" << " " <<"ime" <<" " << "locationCounter";
  for(auto it = allSections->cbegin(); it != allSections->cend(); ++it){
      std::cout <<"\n"<< it->first << " "; 
      it->second->print();
  }
}
