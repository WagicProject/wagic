#include "../include/debug.h"
#include "../include/Subtypes.h"
#include <JGE.h>
#include <algorithm>

Subtypes * Subtypes::subtypesList = NEW Subtypes();



Subtypes::Subtypes(){
 nb_items = 0;
 offset = 100;
}

int Subtypes::Add(string value){
	int result = find(value);
	if (result) return result;
#if defined (WIN32) || defined (LINUX)
	char buf[4096];
	sprintf(buf, "Adding new type: *%s*\n",value.c_str()); 
	OutputDebugString(buf);
#endif
	std::transform( value.begin(), value.end(), value.begin(), ::tolower );
	values[nb_items] = value;
	nb_items++;
	return nb_items + offset - 1;
}

int Subtypes::Add(const char * subtype){
	string value = subtype;
	return Add(value);

}

int Subtypes::find(string value){
	std::transform( value.begin(), value.end(), value.begin(), ::tolower );
	for (int i = 0; i < nb_items; i++){
		if(values[i].compare(value) == 0){
			return i + offset;
		}
	}
	return 0;
}

int Subtypes::find(const char * subtype){
	string value = subtype;
	return (find(value));
		
}