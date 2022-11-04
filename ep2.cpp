#include <iostream>
#include <fstream>
#include <string>
// encapuslate lineReader into a namespace
namespace LineReader {
	/*
	* Represent the state of iterator	
	*/
	struct LineReader {
		const char** filenames; // the names of files to be readed
		std::ifstream* source;  // the state of actual file begin readed
		size_t fileid;  // the index of actual file into filenames
		size_t amount; // the number of files to be readed
		bool alive; // indicates if the iteration doesn't ended
	};
	// prototypes
	void finalizeFileState(LineReader* state);
	bool nextFileState(LineReader* state);
	void finalizeFileState(LineReader* state);
	void nextFile(LineReader* state);
	bool checkNextFile(const LineReader* state);
	void createState(LineReader* state, const char** filenames, const size_t amount);
	bool nextLine(LineReader* state, std::string& buffer);
	/*
	Change the actual file begin readed in lineReader
	[Warnings]:
		(1) The "fileid", "amount" and "filenames" fields needs to be properly initialize before calling.
		(2) The stream's failbit shall be tested after calling, maybe using checkNextFile(). 
		
	For futher information, see:
		- https://cplusplus.com/reference/fstream/ifstream/ifstream/
		- https://cplusplus.com/reference/ios/ios_base/iostate/
		- https://cplusplus.com/reference/ios/ios_base/openmode/
	*/
	bool nextFileState(LineReader* state) {
		const char* filename; 
		if(state->fileid >= state->amount)
			return false; // stop interation
		filename = state->filenames[state->fileid]; // take a reference to name of the next file
		state->source = new std::ifstream(filename); // construct a new file (default open mode: std::ios::in)
		return true; // continue interation
	}
	/*
	Finalizes the previuos file source
	[warnings]:
	(2) The "source" field shall be always a type-valid heap-allocated adress or nullptr. Otherwise, it will cause heap corruption.
	*/
	void finalizeFileState(LineReader* state) {
		if(state->source != nullptr) { // non-poisoned state
			state->source->close(); // close the underlaying stream
			delete state->source;
			state->source = nullptr;
		}
	}
	/*
	Finalizes the previuos file source, and create a new file source.
	[Warnings]:
		- Inherits warnings (1) and (2) from nextFileState()
		- Inherits warning (1) from finalizeFileState()
		(1) The state shall be initialized before calling
	For futher information, see:
	- https://cplusplus.com/reference/fstream/ifstream/close/
	*/
	void nextFile(LineReader* state) {
		state->fileid++; // inclement the index of file
		finalizeFileState(state); // finalizes the previuos state
		state->alive = nextFileState(state); // create a new state
	}
	/*
	Check state of underlaying stream, and returns:
	- false, if the actual source file was the failbit and badbit set.
	- true, otherwise.
	[Warnings]:
		(1) It should called in the very after nextFile() to avoid race conditions, aka TOCTOU. 

	For futher information, see:
	- https://en.wikipedia.org/wiki/Time-of-check_to_time-of-use
	*/
	bool checkNextFile(const LineReader* state) {
		if(state->source->bad()) // check if bad bit is set
			return false; // the stream was poisoned
		if(state->source->fail()) // check if failbit is set
			return false; // the stream was poisoned
		return true; // sucess
	}
	/*
	Initialize the state of struct lineReader.
	[warnings]:
		- Inherits warning (2) from nextFileState().
	*/
	void createState(LineReader* state, const char** filenames, const size_t amount) {
		state->fileid = 0; // its is "filenames" associted index, therefore starts with 0
		state->amount = amount; // the size of "filenames"
		state->filenames = filenames; 
		state->source = nullptr; // init as nullptr (null-initialization warranty)
		state->alive = nextFileState(state); // initialize actual file
	}
	/*
	read a whole line into "buffer" from the actual stream in "state". Returns a boolean value representing if the stream wasn't ended or poisoned.
	[warnings]:
		(1) The "state" shall be alive (the iteration wasn't ended)
		(2) If the stream was poisoned or ended, then acessing "buffer" is a undefined behaviour
	*/
	bool nextLine(LineReader* state, std::string& buffer) {
		std::ifstream* source = state->source; // cast source field into (std::ifstream*)
		return std::getline(*source, buffer).good(); // call std::getline and returns a representing if the stream wasn't ended or poisoned (failbit, badbit and eofbit aren't setted)
	}
	// use a namespace to encapsulate the testing suit
	namespace test {
		void printFilename(const LineReader* state);
		void test(const char** filenames, const size_t amount){
			LineReader state; // the state machine
			std::string line; // the buffer
			createState(&state, filenames, amount); // initialize the state machine
			while(state.alive) { // validate if iterator ended
				printFilename(&state); // print the name of actual file
				if(checkNextFile(&state)) { // validate the state
					while(nextLine(&state, line)) { // read a line from actual file
						std::cout << line << std::endl; // print the line
					}
				} else { // posoined state
					std::cout << "Invalid Input" << std::endl; // send a error message
					finalizeFileState(&state); // finalize the posoined state
					break; // stop iteration
				}
				nextFile(&state); // goto next file
			}
		}
		/*
		Prints the name of actual file.
		[warnings]:
			(1) The "state" shall be alive (the iteration wasn't ended)
		*/
		void printFilename(const LineReader* state) {
			std::cout << "========" << std::flush;
			std::cout << state->filenames[state->fileid] << std::flush; 
			std::cout << "========" << std::endl;
		}
	}
}
namespace OrdenedLinkedMap {
	/*
	expeted a function that returns:
		-1, if the left element is lesser than right element
		0, if the left element is equal to right element
		1, if the left element is greater than right
	*/
	using cmp_fn = int (*)(void*, void*);
	/*
	The node in the linked ordened map
	*/
	struct Node {
		void* key; // the value used for referencing 
		void* value; // the value begin referencied
		Node* prev; // previuos node in the ordened linked map
		Node* next; // next node in the ordened linked map
	};
	/*
	The entry point of linked ordened map
	*/
	struct OrdenedLinkedMap {
		Node* first; // the first node in the ordened map
		Node* last; // the last node in the ordened map
		size_t size;
		cmp_fn compare; // trampoline: the function used for comparing
	};
	/*
	The result of partialSearch().
	*/
	struct SearchInterval {
		Node* lt; // the node that is lesser than the target (nullptr if not exists)
		Node* eq; // the node that is equal to the target (nullptr if not exists)
		Node* gt; // the node that is great than the target (nullptr if not exists)
	};
	/*
	Insert a element in the left side of another.
	[warnings]:
		(1) Position shall not be null
	*/
	void insert_left(OrdenedLinkedMap* list, Node* element, Node* position) {
		Node* previuos = position->prev; // previuos node of "position"
		element->next = position; // previous <-> element <-> position
		element->prev = previuos;
		if(previuos != nullptr) {
			previuos->next = element;
		} else {
			list->first = element;
		}	
		position->prev = element;
	}
	/*
	Insert a element in the right side of another. 
	[warnings]:
		(1) Position shall not be null
	*/
	void insert_right(OrdenedLinkedMap* list, Node* element, Node* position) {
		Node* next = position->next; // next node of "position"
		element->next = next; // position <-> element <-> next
		element->prev = position;
		if(next != nullptr) {
			next->next = element;
		} else {
			list->last = element;
		}
		position->next = element;
	}
	void insert_first(OrdenedLinkedMap* list, Node* element) {
		Node* first = list->first;
		if(first == nullptr) {
			list->first = element;
			list->last = element;
		} else {
			insert_left(list, element, first);
		}
	}
	void insert_last(OrdenedLinkedMap* list, Node* element) {
		Node* last = list->last;
		if(last == nullptr) {
			list->last = element;
			list->first = element;
		} else {
			insert_right(list, element, last);
		}
	}
	SearchInterval partial_find(OrdenedLinkedMap* list, void* key) {
		/*
		The target is the node the first node that key field is equal to "key"
		A candidate is a node that could be the target. Exists up to one candidate, because the fields's value of all nodes after the first candidate are greater than key, and, therefore, all subsequent nodes aren't candidates.
		*/
		SearchInterval state = {nullptr, nullptr, nullptr};
		cmp_fn compare = list->compare; // store the function to compare in the stack
		Node* pre = nullptr; // the last node that was lesser than key (null-initialization)
		Node* now = list->first; // A candidate, if exists. Otherwise, it is nullptr (including the case of the list begin empty, that is, when list->first is nullptr). 
		int cmp = 0; // Comparation flag. Non-zero means that the candidate was found. 
		while(now != nullptr && cmp == 0) {
			/*
			compare(now, key) + 1 == 0 --> now < key
			compare(now, key) + 1 == 1 --> now == key
			compare(now, key) + 1 == 2 --> now > key
			*/
			cmp = compare(now, key) + 1; // update the comparation flag
			pre = now; // update the last node (now != nullptr)
			now = now->next; // now->next == nullptr if and only if the list reached the end
		} /*
		The field "next" last element of the ordened linked map shall be nullptr. So, if "now" is nullptr, then all element are lesser than "key". Otherwise, "now" is a element that is greater or equal to "key". 
		If exists at least one element in the list, then "prev" will pointer to last element that is lesser than key. But, if the list is empty, then "prev" remains as nullptr
		*/
		if(pre == nullptr) { // The list is empty
			return state; // {lt: nullptr, eq: nullptr, gt: nullptr}
		} 
		state.lt = pre; // The list is not empty
		if (now == nullptr) { // Reached the end
			return state; // {lt: prev, eq: nullptr, gt: nullptr}
		}
		if(cmp == 1) { // The target was found
			state.eq = now; // {lt: prev, eq: gt, gt: nullptr}
		} else { // The target wasn't found
			state.gt = now; // {lt: prev, eq: nullptr, gt: now}
		}
		return state;
	}
	/*
	return a boolean value representing if the ordened linked map was empty when the partial_find() was done. This could depends of how partial_find() was implemented, therefore should be always used for this situation.
	*/
	bool intv_empty(SearchInterval* source){
		return source->lt == nullptr;
	}
	/*
	Atomically returns a node that contains the element, if exists; or create and returns a new node, if not exists.
	*/
	Node* find_or_create(OrdenedLinkedMap* list, void* key) {
		return nullptr;
	}
	namespace test {
		void printmap_left_int(OrdenedLinkedMap* list);
		void printmap_right_int(OrdenedLinkedMap* list);
		int compare_int(void* left, void* right) {
			int ileft = *((int*) left);
			int iright = *((int*) left);
			if(ileft < iright)
				return -1;
			if(ileft == iright)
				return 0;
			return 1;
		}
		void test_insertion() {
			int left_keys[] = {10, 15, 16, 97, -113, 48};
			int left_values[] = {20, 82, -72, 37, 51, 45};
			int right_keys[] = {0, 80, 59, 68, 31, 56};
			int right_values[] = {0, 79, 53, 111, -7, 83};
			OrdenedLinkedMap list = {nullptr, nullptr, 0, compare_int};
			for(size_t i = 0; i < 6; i++) {
				Node* data = new Node;
				data->key = (void*) (left_keys + i);
				data->value = (void*) (left_values + i);
				data->next = nullptr;
				data->prev = nullptr;
				insert_first(&list, data);
			}
			for(size_t i = 0; i < 6; i++) {
				Node* data = new Node;
				data->key = (void*) (right_keys + i);
				data->value = (void*) (right_values + i);
				data->next = nullptr;
				data->prev = nullptr;
				insert_last(&list, data);
			}
			printmap_left_int(&list);
			std::cout << std::endl;
			printmap_right_int(&list);
		}
		void printnode(Node* src){
			std::cout << "key=" << *((int*) src->key) << " ";
			std::cout << "value=" << *((int*) src->value) << std::endl;
		}
		void printmap_left_int(OrdenedLinkedMap* list) {
			for(Node* now = list->first; now != nullptr; now = now->next) {
				printnode(now);
			}
		}
		void printmap_right_int(OrdenedLinkedMap* list) {
			for(Node* now = list->last; now != nullptr; now = now->prev) {
				printnode(now);
			}
		}
	}
}
int main() {
	const char* filenames[3];
	filenames[0] = "test1.txt";
	filenames[1] = "test2.txt";
	filenames[3] = "test3.txt";
	LineReader::test::test(filenames, 2);
	OrdenedLinkedMap::test::test_insertion();
	return 0;
}
