#include <iostream>
#include <stdio.h>
#include <string>
// encapuslate lineReader into a namespace
namespace LineReaderFile {
	struct LineReader {
		const char** filenames; // the names of files to be readed
		FILE* source;  // the state of actual file begin readed
		size_t fileid;  // the index of actual file into filenames
		size_t amount; // the number of files to be readed
		bool alive;
	};
	// prototypes
	const char* get_filename(const LineReader* state);
	bool open_file(LineReader* state);
	void close_file(LineReader* state);
	void next_file(LineReader* state);
	bool isgood(const LineReader* state);
	bool isalive(const LineReader* state);
	bool check_state(const LineReader* state);
	void create(LineReader* state, const char** filenames, const size_t amount);
	/*
	Returns the pointer to name of actual file.
	[Warnings]:
		(1) It shall be called only once the iterator is alive. Otherwise it could be undefined behavior.
	*/
	const char* get_filename(const LineReader* state) {
		return state->filenames[state->fileid];
	}
	/*
	Tries the actual state's fully initialization. Returns a boolean value indicating that iteration still alive
	[Warnings]:
		(1) Inherits the warning (1) from get_filename().
		(2) The "fileid", "amount" and "filenames" fields needs to be properly initialize before calling.
	*/
	
	bool open_file(LineReader* state) {
		if(state->fileid >= state->amount)
			return false; // stop interation
		state->source = fopen(get_filename(state), "r"); // open the file in read mode
		return true; // constinue iteration
	}
	/*
	Finalizes the previuos state, closing the underlaying file (if wasn't closed).
	[Warnings]:
		(1) The "source" field should be always either a valid file or NULL.
	*/
	void close_file(LineReader* state) {
		if(state->source != NULL) { // non-poisoned state
			fclose(state->source); // close the underlaying file
			state->source = NULL; // set to null to avoid reuse
		}
	}
	/*
	Finalizes the previuos state and advances to next state. 
	[Warnings]:
		(1) Inherits warnings (1) and (2) from open_file()
		(2) Inherits warning (1) from close_file()
	*/
	void next_file(LineReader* state) {
		close_file(state); // close the previuos file (only after first iteration)
		state->fileid++;
		state->alive = open_file(state); // update "alive" field, which indicate if iteration is alive
	}
	/*
	check if iteration is alive and the actual state was sucessful initialized
	*/
	bool isgood(const LineReader* state) {
		return state->alive && check_state(state); // returns a boolean value indicating if the iteration is alive and the actual state was sucessful initialized.
	}
	/*
	check if iteration is alive
	*/
	bool isalive(const LineReader* state) {
		return state->alive;
	}
	/*
	Check state of underlaying file, and returns:
	- false, if the actual source file pointer is NULL
	- true, otherwise.
	*/
	bool check_state(const LineReader* state) {
		return state->source != NULL; 
	}
	/*
	Properly initialize the state of iterator.
	[warnings]:
		(1) It will override all fields of state. So make sure for properly finalizing it, if the struct LineReader was reused. 
	*/
	void create(LineReader* state, const char** filenames, const size_t amount) {
		state->fileid = 0; // its is "filenames" associted index, therefore starts with 0
		state->amount = amount; // the size of "filenames"
		state->filenames = filenames; 
		state->source = NULL; // init as NULL (warning (1) from close_file())
		state->alive = open_file(state); // update "alive" field, which indicate if iteration is alive
	}
	void read_word(LineReader* state, std::string& buffer) {
		
	}
	// use a namespace to encapsulate the testing suit
	namespace test {
		void printFilename(const LineReader* state);
		void test(const char** names, const size_t amount){
			LineReader state; // the state machine
			std::string line; // the buffer
			for(create(&state, names, amount); isgood(&state); next_file(&state)) {
				printFilename(&state);
				char tmp;
				while((tmp = fgetc(state.source)) != EOF) 
					std::cout << tmp;
			} // the iteration is not alive or is poisoned
			std::cout << std::endl << "@=========@ end @=========@" << std::endl;
			close_file(&state); // close the underlaying file, if it is opened.
			if(!check_state(&state))  // iteration is poisoned and is alive
				std::cout << "Invalid File: " << get_filename(&state) << std::endl << std::endl;
		}
		/*
		Prints the name of actual file.
		[warnings]:
			(1) The "state" shall be alive (the iteration wasn't ended)
		*/
		void printFilename(const LineReader* state) {
			std::cout << "========" << std::flush;
			std::cout << get_filename(state) << std::flush; 
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
	using cmp_fn = int (*)(const void*, const void*);
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
	void clear_search_interval(SearchInterval* into) {
		into->lt = nullptr;
		into->eq = nullptr;
		into->gt = nullptr;
	}
	void partial_find(SearchInterval* state, OrdenedLinkedMap* list, void* key) {
		/*
		The target is the node the first node that key field is equal to "key"
		A candidate is a node that could be the target. Exists up to one candidate, because the fields's value of all nodes after the first candidate are greater than key, and, therefore, all subsequent nodes aren't candidates.
		*/
		clear_search_interval(state);
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
			return; // {lt: nullptr, eq: nullptr, gt: nullptr}
		} 
		state->lt = pre; // The list is not empty
		if (now == nullptr) // Reached the end
			return; // {lt: prev, eq: nullptr, gt: nullptr}
		if(cmp == 1) { // The target was found
			state->eq = now; // {lt: prev, eq: gt, gt: nullptr}
		} else { // The target wasn't found
			state->gt = now; // {lt: prev, eq: nullptr, gt: now}
		}
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
	// the cmp_fn implementation for type "int"
	int compare_int(const void* left, const void* right) {
		int ileft = *((int*) left);
		int iright = *((int*) left);
		if(ileft < iright)
			return -1;
		if(ileft == iright)
			return 0;
		return 1;
	}
	// the cmp_fn implementation for type "std::string"
	int compare_string(const void* left, const void* right) {
		std::string sleft = *((std::string*) left);
		std::string sright = *((std::string*) left);
		return sleft.compare(sright);
	}
	// the cmp_fn implementation for type "std::wstring"
	int compare_wstring(const void* left, const void* right) {
		std::wstring sleft = *((std::wstring*) left);
		std::wstring sright = *((std::wstring*) left);
		return sleft.compare(sright);
	}
	Node* create_node(void* key, void* value) {
		Node* tmp = new Node;
		tmp->key = key;
		tmp->value = value;
		tmp->next = nullptr;
		tmp->prev = nullptr;
		return tmp;
	}
	namespace test {
		void printmap_left_int(OrdenedLinkedMap* list);
		void printmap_right_int(OrdenedLinkedMap* list);
		void test_insertion() {
			int left_keys[] = {10, 15, 16, 97, -113, 48};
			int left_values[] = {20, 82, -72, 37, 51, 45};
			int right_keys[] = {0, 80, 59, 68, 31, 56};
			int right_values[] = {0, 79, 53, 111, -7, 83};
			OrdenedLinkedMap list = {nullptr, nullptr, 0, compare_int};
			for(size_t i = 0; i < 6; i++) {
				Node* data = create_node(left_keys + i, left_values + i);
				insert_first(&list, data);
			}
			for(size_t i = 0; i < 6; i++) {
				Node* data = create_node(right_keys + i, right_values + i);
				insert_last(&list, data);
			}
			printmap_left_int(&list);
			std::cout << std::endl;
			printmap_right_int(&list);
		}
		void test_string_comparation(){
			const wchar_t* src[] = {L"é", L"és", L"bem", L"bom", L"já", L"com", L"de", L"e", L"e"};
			std::wstring* text[9];
			for(size_t i = 0; i < 9; i++)
				text[i] = new std::wstring(src[i]);
			std::cout << std::flush;
			for(size_t i = 0; i < 8; i++) {
				//int cmp = compare_string(text[i], text[i+1]);
				std::wcout << *text[i] << L" ";
				/*if (cmp == -1)
					std::wcout << L"<";
				else if (cmp == 0)
					std::wcout << L"=";
				else
					std::wcout << L">";*/
				//std::wcout << L" " << text[i+1] << std::endl;
			}
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
	const char* filenames[4];
	filenames[0] = "test1.txt";
	filenames[1] = "test2.txt";
	filenames[2] = "test3.txt";
	filenames[3] = "test4.txt";
	std::wcout << "¿és mi hermano?" << std::endl;
	//LineReaderFile::test::test(filenames, 3);
	//OrdenedLinkedMap::test::test_string_comparation();
	//OrdenedLinkedMap::test::test_insertion();
	
	return 0;
}
