#include <iostream>
#include <math.h>
#include <fstream>
#include <string>
#include <vector>

using namespace std;


struct Data{
	char inst;
	long long addr;
};

struct Info{
	int lastUsed;
	vector<long long> line;
};

int directMapped(int size, int lineSize, vector<Data> code, int numLines);
int setAssociative(int size, int lineSize, vector<Data> code, int numLines, int associativity);
int fullyAssociative(int size, int lineSize, vector<Data> code, int numLines, string policy);
int setWithNoAllo(int size, int lineSize, vector<Data> code, int numLines, int associativity);
int setWithNextPre(int size, int lineSize, vector<Data> code, int numLines, int associativity);
int prefetch(int size, int lineSize, vector<Data> code, int numLines, int associativity);


int main(int argc, char *argv[]) {

  // Temporary variables
  char inst;
  long long addr;
  int numLines =0;
  int hits;
  vector<Data> code;
  Data temp;
  ifstream fin(argv[1]);
  ofstream fout(argv[2]);
  
  while(fin >> inst >> std::hex >> addr){
	 temp.inst=inst;
	 temp.addr = addr;
	 code.push_back(temp);
	 numLines++;
  }
  
  hits=directMapped(1024, 32, code, numLines);
  fout << hits << "," << numLines<< "; ";
  hits=directMapped(4096, 32, code, numLines);
  fout << hits << "," << numLines<< "; ";
  hits=directMapped(16384, 32, code, numLines);
  fout << hits << "," << numLines<< "; ";
  hits=directMapped(32768, 32, code, numLines);
  fout << hits << "," << numLines<< "; " << endl;
  
  hits = setAssociative(16384,32,code,numLines,2);
  fout << hits << "," << numLines << "; ";
  hits = setAssociative(16384,32,code,numLines,4);
  fout << hits << "," << numLines << "; ";
  hits = setAssociative(16384,32,code,numLines,8);
  fout << hits << "," << numLines << "; ";
  hits = setAssociative(16384,32,code,numLines,16);
  fout << hits << "," << numLines << "; " << endl;
  
  hits = fullyAssociative(16384,32,code,numLines,"LRU");
  fout << hits << "," << numLines << "; " << endl;
  
  hits = fullyAssociative(16384,32,code,numLines,"HOT-COLD");
  fout << hits << "," << numLines << "; " << endl;
  
  hits = setWithNoAllo(16384,32,code,numLines,2);
  fout << hits << "," << numLines << "; ";
  hits = setWithNoAllo(16384,32,code,numLines,4);
  fout << hits << "," << numLines << "; ";
  hits = setWithNoAllo(16384,32,code,numLines,8);
  fout << hits << "," << numLines << "; ";
  hits = setWithNoAllo(16384,32,code,numLines,16);
  fout << hits << "," << numLines << "; " << endl;
  
  hits = setWithNextPre(16384,32,code,numLines,2);
  fout << hits << "," << numLines << "; ";
  hits = setWithNextPre(16384,32,code,numLines,4);
  fout << hits << "," << numLines << "; ";
  hits = setWithNextPre(16384,32,code,numLines,8);
  fout << hits << "," << numLines << "; ";
  hits = setWithNextPre(16384,32,code,numLines,16);
  fout << hits << "," << numLines << "; " << endl;
  
  hits = prefetch(16384,32,code,numLines,2);
  fout << hits << "," << numLines << "; ";
  hits = prefetch(16384,32,code,numLines,4);
  fout << hits << "," << numLines << "; ";
  hits = prefetch(16384,32,code,numLines,8);
  fout << hits << "," << numLines << "; ";
  hits = prefetch(16384,32,code,numLines,16);
  fout << hits << "," << numLines << "; " << endl;
    
  
  
  fin.close();
  fout.close();
  return 0;
}

int directMapped(int size, int lineSize, vector<Data> code, int numLines){
	
   int slots = size / lineSize;
	int lineIndex, index;
	int hits = 0;
	
	vector<vector<long long> > cache (slots, vector<long long> (lineSize, -1)); 
		
	for(int x=0; x<numLines; x++){
	
		index = (code[x].addr / lineSize) % slots;
		lineIndex = code[x].addr % lineSize;
		
		if(cache[index][lineIndex] != code[x].addr){

			for(int i = 0; i < lineSize; i++){
				cache[index][i] = (code[x].addr / lineSize)*lineSize+i;
				//cout << cache[index][i] << " ";
			}
			//cout << endl;

		}else{
			hits++;
		}
	}
	return hits;
}

int setAssociative(int size, int lineSize, vector<Data> code, int numLines, int associativity){
	
   int slots = size / lineSize / associativity;
   int lineIndex, index;
	int hits = 0;
   double usedCounter=0;
   
   Info tmp;
	tmp.lastUsed = -1;
	for(int i=0; i<lineSize; i++){tmp.line.push_back(-1);}
	
	vector< vector< Info > > cache(associativity, vector < Info> (slots, tmp ));
	
	for(int x=0; x<numLines; x++){
		bool hit = false;
		
		index = (code[x].addr / lineSize) % slots;
		lineIndex = code[x].addr % lineSize;

		for(unsigned int i = 0; i < cache.size(); i++){
			
			if(cache[i][index].line[lineIndex] == code[x].addr){
				hits++;
				usedCounter++;
				cache[i][index].lastUsed = usedCounter;

				hit = true;
				break;	
			}
		}
			
		if(!hit){

			int min = 276447231;
			int replaceIndex = -1;
	
			for(unsigned int i = 0; i < cache.size(); i++){
				if(cache[i][index].lastUsed < min){
					min = cache[i][index].lastUsed;
					replaceIndex = i;
				}
			}	

			usedCounter++;		
			cache[replaceIndex][index].lastUsed = usedCounter;
			for(unsigned int i = 0; i < cache[replaceIndex][index].line.size(); i++){
				cache[replaceIndex][index].line[i] = (code[x].addr / lineSize) * lineSize + i;
			}
		}
	}
			
	return hits;
}

int fullyAssociative(int size, int lineSize, vector<Data> code, int numLines, string policy){
	int ret = 0;
	
	if(policy=="HOT-COLD") {
		int slots = size / lineSize;
		
		vector<int> hotCold (slots-1, 0);
		
		Info tmp;
		tmp.lastUsed = -1;
		for(int i=0; i<lineSize; i++){tmp.line.push_back(-1);}
		
		vector<Info> cache (slots, tmp);
		
		int hits=0;

		for(int x=0; x<numLines; x++){
			ret++;
			
			bool hit = false;
			int hitIndex = -1;
			int lineIndex = code[x].addr % lineSize;
			
			for(unsigned int i = 0; i < cache.size(); i++){
				 
				if(cache[i].line[lineIndex] == code[x].addr){
					hit = true;
					hitIndex = i;
					break;
				}
			}
			int hotColdIndex = -1;
			
			if(hit){
				
				hits++;
				
				hotColdIndex = hitIndex + slots -1;
				while(hotColdIndex != 0){
					if(hotColdIndex % 2 == 0){
						hotColdIndex = (hotColdIndex - 2) / 2;
						hotCold[hotColdIndex] = 0;
					
					}else{
						hotColdIndex = (hotColdIndex - 1) / 2;
						hotCold[hotColdIndex] = 1;
					}
				}
			}
			
			else{
				
				
				int replaceIndex = -1;
				hotColdIndex = 0;
				for(int i = 0; i < log2(slots); i++){
					if(hotCold[hotColdIndex] == 0){
						hotCold[hotColdIndex] = 1;
						hotColdIndex = hotColdIndex * 2 + 1;
					}else{
						hotCold[hotColdIndex] = 0;
						hotColdIndex = hotColdIndex * 2 + 2;
					}
				}	

				
				replaceIndex = hotColdIndex - (slots - 1);
				
				
				for(unsigned int i = 0; i < cache[replaceIndex].line.size(); i++){
					cache[replaceIndex].line[i] = ((code[x].addr / lineSize) * lineSize) + i;
				}
			}
		}
		ret = hits;
	}
	else if(policy == "LRU") {
		ret = setAssociative(size, lineSize, code, numLines, size/lineSize);
	}
	return ret;
}

int setWithNoAllo(int size, int lineSize, vector<Data> code, int numLines, int associativity){
	int slots = size / lineSize / associativity;
	int lineIndex, index;
  	int hits = 0;
	int usedCounter=0;
	
  	Info tmp;
  	tmp.lastUsed = -1;
  	for(int i=0; i<lineSize; i++){tmp.line.push_back(-1);}

  	vector< vector< Info > > cache (associativity, vector < Info> (slots,  tmp));

   for(int x=0; x<numLines; x++){
    	bool hit = false;

    	index = (code[x].addr / lineSize) % slots;
    	lineIndex = code[x].addr % lineSize;

    	for(unsigned int i = 0; i < cache.size(); i++){

        	if(cache[i][index].line[lineIndex] == code[x].addr){
           	hits++;
           	usedCounter++;
            
            cache[i][index].lastUsed = usedCounter;

            hit = true;
            break;
        	}
    	}

    	if(!hit && (code[x].inst == 'L')){

        	int min = 276447231;
        	int replaceIndex = -1;

         for(unsigned int i = 0; i < cache.size(); i++){
           	if(cache[i][index].lastUsed < min){
             	min = cache[i][index].lastUsed;
             	replaceIndex = i;
            }
         }
         
         usedCounter++;
         cache[replaceIndex][index].lastUsed = usedCounter;
         for(unsigned int i = 0; i < cache[replaceIndex][index].line.size(); i++){
           	cache[replaceIndex][index].line[i] = (code[x].addr / lineSize) * lineSize + i;
         }
    	}
  	}
   return hits;
}

int setWithNextPre(int size, int lineSize, vector<Data> code, int numLines, int associativity){
	int slots = size / lineSize / associativity;
	int lineIndex, index;
  	int hits = 0;
	int usedCounter=0;
	
  	Info tmp;
  	tmp.lastUsed = -1;
 	for(int i=0; i<lineSize; i++){tmp.line.push_back(-1);}

 	vector< vector< Info > > cache (associativity, vector < Info> (slots,  tmp));

  	

   for(int x=0; x<numLines; x++){
    	bool hit = false;

    	index = (code[x].addr / lineSize) % slots;
    	lineIndex = code[x].addr % lineSize;

    	for(unsigned int i = 0; i < cache.size(); i++){

        	if(cache[i][index].line[lineIndex] == code[x].addr){
           	hits++;
           	usedCounter++;
           	cache[i][index].lastUsed = usedCounter;

           	hit = true;
           	break;
       	}
    	}

    	if(!hit){

        	int min = 276447231;
         int replaceIndex = -1;

         for(unsigned int i = 0; i < cache.size(); i++){
           	if(cache[i][index].lastUsed < min){
             	min = cache[i][index].lastUsed;
             	replaceIndex = i;
           	}
         }
         
         usedCounter++;
         cache[replaceIndex][index].lastUsed = usedCounter;
         for(unsigned int i = 0; i < cache[replaceIndex][index].line.size(); i++){
           	cache[replaceIndex][index].line[i] = (code[x].addr / lineSize) * lineSize + i;
         }
    	}
		
		index = (index + 1) % slots;
		bool nextHit = false;
		
		for(unsigned int i = 0; i < cache.size(); i++){
      	if(cache[i][index].line[0] == (((code[x].addr / lineSize)+1)*lineSize)){
      		usedCounter++;
            cache[i][index].lastUsed = usedCounter;
				nextHit = true;
				break;
			}
		}
		if(!nextHit){
			int replaceIndex = -1;
			int min = 276447231;
			for(unsigned int i = 0; i < cache.size(); i++){
				if(cache[i][index].lastUsed < min){
					min = cache[i][index].lastUsed;
					replaceIndex = i;
				}
			}
                         
         usedCounter++;
	      cache[replaceIndex][index].lastUsed = usedCounter;
        	for(unsigned int i = 0; i < cache[replaceIndex][index].line.size(); i++){
           	cache[replaceIndex][index].line[i] = ((code[x].addr / lineSize)+1) * lineSize + i;
	      }
		}  
	}
  	return hits;
}

int prefetch(int size, int lineSize, vector<Data> code, int numLines, int associativity){

   int slots = size / lineSize / associativity;
	int lineIndex, index;
  	int hits = 0;
	int usedCounter=0;
	
  	Info tmp;
  	tmp.lastUsed = -1;
 	for(int i=0; i<lineSize; i++){tmp.line.push_back(-1);}

 	vector< vector< Info > > cache (associativity, vector < Info> (slots,  tmp));

   for(int x=0; x<numLines; x++){
    	bool hit = false;

    	index = (code[x].addr / lineSize) % slots;
    	lineIndex = code[x].addr % lineSize;

    	for(unsigned int i = 0; i < cache.size(); i++){

        	if(cache[i][index].line[lineIndex] == code[x].addr){
           	hits++;
           	usedCounter++;
           	cache[i][index].lastUsed = usedCounter;

           	hit = true;
           	break;
       	}
    	}

    	if(!hit){

        	int min = 276447231;
         int replaceIndex = -1;

         for(unsigned int i = 0; i < cache.size(); i++){
           	if(cache[i][index].lastUsed < min){
             	min = cache[i][index].lastUsed;
             	replaceIndex = i;
           	}
         }
         
         usedCounter++;
         cache[replaceIndex][index].lastUsed = usedCounter;
         for(unsigned int i = 0; i < cache[replaceIndex][index].line.size(); i++){
           	cache[replaceIndex][index].line[i] = (code[x].addr / lineSize) * lineSize + i;
         }
    	}

		if(!hit){
       	index = (index + 1) % slots;
       	bool nextHit = false;
       	for(unsigned int i = 0; i < cache.size(); i++){
	         if(cache[i][index].line[0] == (((code[x].addr / lineSize)+1)*lineSize)){
	         	usedCounter++;
              	cache[i][index].lastUsed = usedCounter;
              	nextHit = true;
              	break;
	         }
       	}
       	if(!nextHit){
	         int replaceIndex = -1;
	         int min = 276447231;
	         for(unsigned int i = 0; i < cache.size(); i++){
              	if(cache[i][index].lastUsed < min){
                	min = cache[i][index].lastUsed;
                	replaceIndex = i;
              	}
	         }
 
	         usedCounter++;                  
	         cache[replaceIndex][index].lastUsed = usedCounter;
	         for(unsigned int i = 0; i < cache[replaceIndex][index].line.size(); i++){
	           	cache[replaceIndex][index].line[i] = ((code[x].addr / lineSize)+1) * lineSize + i;
	         } 
       	}
		}
 	}  
  	return hits;
}