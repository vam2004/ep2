//#define __STDC_WANT_LIB_EXT1_ 1
#include <iostream>
#include <locale>
#include <stdio.h>
#include <fstream>
#include <assert.h>
#include <string>
#include <wchar.h>
#include <wctype.h>
#include <codecvt>
//debugging flags
#ifdef __DEBUG_INSERTIONS__
	#define __SHOW_COMPARATIONS__
	#define __SHOW_INSERTIONS__
	#define __SHOW_IF_FOUND_INSERTION__
	#define __SHOW_SEARCHING__
#endif

#ifdef __VERBOSE_TESTS__
	#define __SANITY_TESTS__
#endif
// encapuslate lineReader into a namespace
namespace line_reader {
	template<typename source_t>
	struct LineReader {
		const char** filenames; // the names of files to be readed
		source_t source;  // the state of actual file begin readed
		size_t fileid;  // the index of actual file into filenames
		size_t amount; // the number of files to be readed
		bool alive;
	};
	// prototypes
	template<typename source_t>
		const char* get_filename(const LineReader<source_t>* state);
	template<typename source_t>
		bool open_file(LineReader<source_t>* state);
	template<typename source_t>
		void close_file(LineReader<source_t>* state);
	template<typename source_t>
		void next_file(LineReader<source_t>* state);
	template<typename source_t>
		bool isgood(const LineReader<source_t>* state);
	template<typename source_t>
		bool isalive(const LineReader<source_t>* state);
	template<typename source_t>
		void create(LineReader<source_t>* state, const char** filenames, const size_t amount);
	// traits
	template<typename source_t> source_t create_source(const char* filename);
	template<typename source_t> source_t null_source();
	// implementation for FILE*
	template<>
	FILE* create_source<FILE*>(const char* filename) {
		return fopen(filename, "r");
	}
	void delete_source(FILE* source) {
		fclose(source);
	}
	template<>
	FILE* null_source<FILE*>() {
		return NULL;
	}
	
	bool check_source(const LineReader<FILE*>* state) {
		return state->source != NULL; 
	}
	// implementation for std::wifstream*
	template<>
	std::wifstream* create_source<std::wifstream*>(const char* filename) {
		return new std::wifstream(filename);;
	}
	template<>
	std::wifstream* null_source<std::wifstream*>() {
		return nullptr;
	}
	// implementation for std::ifstream*
	template<>
	std::ifstream* create_source<std::ifstream*>(const char* filename) {
		return new std::ifstream(filename);;
	}
	template<>
	std::ifstream* null_source<std::ifstream*>() {
		return nullptr;
	}
	// shared implementation for std::basic_ifstream<charT>*
	template<typename charT>
	void delete_source(std::basic_ifstream<charT>* source) {
		source->close();
		delete source;
	}
	template<typename charT>
	bool check_source(const LineReader<std::basic_ifstream<charT>*>* state) {
		if(state->source == nullptr)
			return false;
		if(state->source->bad()) // check if bad bit is set
			return false; // the stream was poisoned
		if(state->source->fail()) // check if failbit is set
			return false; // the stream was poisoned
		return true; // sucess
	}
	// end implementation
	/*
	Returns the pointer to name of actual file.
	[Warnings]:
		(1) It shall be called only once the iterator is alive. Otherwise it could be undefined behavior.
	*/
	template<typename source_t>
	const char* get_filename(const LineReader<source_t>* state) {
		return state->filenames[state->fileid];
	}
	
	/*
	Tries the actual state's fully initialization. Returns a boolean value indicating that iteration still alive
	[Warnings]:
		(1) Inherits the warning (1) from get_filename().
		(2) The "fileid", "amount" and "filenames" fields needs to be properly initialize before calling.
	*/
	template<typename source_t>
	bool open_file(LineReader<source_t>* state) {
		if(state->fileid >= state->amount)
			return false; // stop interation
		state->source = create_source<source_t>(get_filename(state)); // open the file in read mode
		return true; // constinue iteration
	}
	/*
	Finalizes the previuos state, closing the underlaying file (if wasn't closed).
	[Warnings]:
		(1) The "source" field should be always either a valid file or null_source.
	*/
	template<typename source_t>
	void close_file(LineReader<source_t>* state) {
		if(state->source != null_source<source_t>()) { // non-poisoned state
			delete_source(state->source); // close the underlaying file
			state->source = null_source<source_t>(); // set to null to avoid reuse
		}
	}
	/*
	Finalizes the previuos state and advances to next state. 
	[Warnings]:
		(1) Inherits warnings (1) and (2) from open_file()
		(2) Inherits warning (1) from close_file()
	*/
	template<typename source_t>
	void next_file(LineReader<source_t>* state) {
		close_file(state); // close the previuos file (only after first iteration)
		state->fileid++;
		state->alive = open_file(state); // update "alive" field, which indicate if iteration is alive
	}
	/*
	check if iteration is alive
	*/
	template<typename source_t>
	bool isalive(const LineReader<source_t>* state) {
		return state->alive;
	}
	/*
	check if iteration is alive and the actual state was sucessful initialized
	*/
	template<typename source_t>
	bool isgood(const LineReader<source_t>* state) {
		return isalive(state) && check_source(state); // returns a boolean value indicating if the iteration is alive and the actual state was sucessful initialized.
	}
	/*
	Properly initialize the state of iterator.
	[warnings]:
		(1) It will override all fields of state. So make sure for properly finalizing it, if the struct LineReader was reused. 
	*/
	template<typename source_t>
	void create(LineReader<source_t>* state, const char** filenames, const size_t amount) {
		state->fileid = 0; // its is "filenames" associted index, therefore starts with 0
		state->amount = amount; // the size of "filenames"
		state->filenames = filenames; 
		state->source = null_source<source_t>(); // init as null_source (warning (1) from close_file())
		state->alive = open_file(state); // update "alive" field, which indicate if iteration is alive
	}
	// use a namespace to encapsulate the testing suit
	namespace test {
		template<typename source_t> 
			void printFilename(const LineReader<source_t>* state);
		
		void print_file(FILE* source){
			while(true) {
				wchar_t tmp = getwc(source);
				if(feof(source))
					break;
				std::wcout << tmp;
			}
		}
		void print_file(std::wifstream* source){
			wchar_t buffer;
			while(source->get(buffer)) 
				std::wcout << buffer;
		}
		void print_file(std::ifstream* source){
			char buffer;
			while(source->get(buffer)) 
				std::cout << buffer;
		}
		template<typename source_t>
		void test(const char** names, const size_t amount){
			LineReader<source_t> state; // the state machine
			for(create(&state, names, amount); isgood(&state); next_file(&state)) {
				printFilename(&state);
				print_file(state.source);
			} // the iteration is not alive or is poisoned
			std::wcout << std::endl << "@=========@ end @=========@" << std::endl;
			close_file(&state); // close the underlaying file, if it is opened.
			if(isalive(&state))  // iteration is poisoned and is alive
				std::wcout << "Invalid File: " << get_filename(&state) << std::endl;
			std::wcout << std::flush << std::endl;
		}
		/*
		Prints the name of actual file.
		[warnings]:
			(1) The "state" shall be alive (the iteration wasn't ended)
		*/
		template<typename source_t>
		void printFilename(const LineReader<source_t>* state) {
			std::wcout << L"========" << std::flush;
			std::wcout << get_filename(state) << std::flush; 
			std::wcout << L"========" << std::endl;
		}
	}
}

namespace ordened_linked_map {
	
	/*
	
	expeted a function that returns:
		negative number, if the left element is lesser than right element
		zero, if the left element is equal to right element
		positive number, if the left element is greater than right
	Likely to be implemented: 
		- https://cplusplus.com/reference/string/string/compare/
		- 
	*/
	template<typename key_t>
	using cmp_fn = int (*)(const key_t*, const key_t*);
	const char* get_cmp_symbol(int result) {
		if (result == -1)
			return "<";
		if (result == 0)
			return "=";
		return ">";
	}
	/*
	The node in the linked ordened map
	*/
	//#define LINKMARK
	#ifdef LINKMARK
		#define DECRESCENT_LINKMARK 1
		#define CRESCENT_LINKMARK 2 
		#define CREATION_MARK 4
	#endif
	template<typename key_t, typename value_t>
	struct Node {
		key_t* key; // the value used for referencing 
		value_t* value; // the value begin referencied
		Node<key_t, value_t>* prev; // previuos (left) node in the ordened linked map
		Node<key_t, value_t>* next; // next (right) node in the ordened linked map
		#ifdef LINKMARK
		int linkmark; // used for validating
		#endif
	};
	/*
	The entry point of linked ordened map
	*/
	template<typename key_t, typename value_t>
	struct OrdenedLinkedMap {
		Node<key_t, value_t>* first; // the first node (left egde) of the ordened linked map
		Node<key_t, value_t>* last; // the last node (right edge) of the ordened linked map
		cmp_fn<key_t> compare; // trampoline: the function used for comparing
	};
	template<typename key_t, typename value_t>
	void initalize_empty(OrdenedLinkedMap<key_t, value_t>* list, cmp_fn<key_t> compare) {
		list->first = nullptr;
		list->last = nullptr;
		list->compare = compare;
	}
	namespace NodeIterator {
		template<typename key_t, typename value_t>
		struct NodeIterator {
			Node<key_t, value_t>* now;
			bool reverse;
		};
		// main prototypes
		template<typename key_t, typename value_t>
		void rewind(NodeIterator<key_t, value_t>* state, const OrdenedLinkedMap<key_t, value_t>* list);
		template<typename key_t, typename value_t>
		bool isalive(const NodeIterator<key_t, value_t>* state);
		template<typename key_t, typename value_t>
		bool isreverse(const NodeIterator<key_t, value_t>* state);
		template<typename key_t, typename value_t>
		void seekend(NodeIterator<key_t, value_t>* state, const OrdenedLinkedMap<key_t, value_t>* list);
		template<typename key_t, typename value_t>
		Node<key_t, value_t>* next(NodeIterator<key_t, value_t>* state);
		template<typename key_t, typename value_t>
		Node<key_t, value_t>* back(NodeIterator<key_t, value_t>* state);
		
		template<typename key_t, typename value_t>
		NodeIterator<key_t, value_t>* create(NodeIterator<key_t, value_t>* state, Node<key_t, value_t>* now) {
			state->reverse = false;
			state->now = now;
			return state;
		}
		template<typename key_t, typename value_t>
		NodeIterator<key_t, value_t>* createReverse(NodeIterator<key_t, value_t>* state, Node<key_t, value_t>* now) {
			state->reverse = true;
			state->now = now;
			return state;
		}
		template<typename key_t, typename value_t>
		bool isreverse(const NodeIterator<key_t, value_t>* state) {
			return state->reverse;
		}
		template<typename key_t, typename value_t>
		bool isalive(const NodeIterator<key_t, value_t>* state) {
			return state->now != nullptr;
		}
		template<typename key_t, typename value_t>
		void rewind(NodeIterator<key_t, value_t>* state, const OrdenedLinkedMap<key_t, value_t>* list) {
			state->now = state->reverse ? list->last : list->first;
		}
		template<typename key_t, typename value_t>
		void seekend(NodeIterator<key_t, value_t>* state, const OrdenedLinkedMap<key_t, value_t>* list) {
			state->now = state->reverse ? list->first : list->last;
		}
		template<typename key_t, typename value_t>
		Node<key_t, value_t>* next(NodeIterator<key_t, value_t>* state) {
			Node<key_t, value_t>* now = state->now;
			state->now = state->reverse ? now->prev : now->next;
			return now;
		}
		template<typename key_t, typename value_t>
		Node<key_t, value_t>* back(NodeIterator<key_t, value_t>* state) {
			Node<key_t, value_t>* now = state->now;
			state->now = state->reverse ? now->next : now->prev;
			return now;
		}
	}
	/*
	The result of partialSearch().
	*/
	template<typename key_t, typename value_t>
	struct SearchInterval {
		Node<key_t, value_t>* lt; // the node that is lesser than the target (nullptr if not exists)
		Node<key_t, value_t>* eq; // the node that is equal to the target (nullptr if not exists)
		Node<key_t, value_t>* gt; // the node that is great than the target (nullptr if not exists)
	};
	/*
	Insert a element at the left side of another. If the "position" is the left edge of linked ordened map then inserts and update that.
	[Warnings]:
		(1) The left edge of linked ordened map shall have its pointer to left node as nullptr.
		(2) The "position" shall not be nullptr. 
		(3) The "element" shall not be nullptr.
		(4) The fields pointer to left and right nodes of "element" should be nullptr. Because these pointers could be overwriten (pontential memory leak).
		(5) The function doesn't validate ordering
	*/
	template<typename key_t, typename value_t>
	void insertLeft(OrdenedLinkedMap<key_t, value_t>* list, Node<key_t, value_t>* element, Node<key_t, value_t>* position) {
		Node<key_t, value_t>* previuos = position->prev; // copy the reference to left node of "position"
		element->next = position; // (1) link "position" as the right node of "element"
		element->prev = previuos; // (2) link "previuos" as the left node of "element"
		position->prev = element; // (3) link "element" as the left node of "element"
		if(previuos != nullptr) { // the "position" is not the left edge of linked ordened map
			previuos->next = element;
			/* Crescent links:
			* 	"previuos" points to "element" as right node
			* 	"element" points to "position" as right node [because of (1)]
			* Decrescent links:
			*	"position" points to "element" as left node [because of (3)]
			*	"element" points to "previuos" as left node [because of (2)]
			* Diagram: 
			* 	previuos <-> element <-> position
			*/
		} else { // the "position" is the left edge of linked ordened map
			list->first = element;
			/* Crescent links:
			* 	"element" is the left edge node of ordened linked map
			* 	"element" points to "position" as right node [because of (1)]
			* Decrescent links:
			*	"position" points to "element" as left node [because of (3)]
			*	"element" is the left edge node of ordened linked map
			* Diagram: 
			* 	(left edge) -> element <-> position
			*/
		}	
	}
	/*
	Insert a element at the right side of another.  If the "position" is the right edge of map then inserts and update that.
	[Warnings]:
		(1) The right edge of linked ordened map shall have its pointer to right node as nullptr.
		(2) The "position" shall not be nullptr. 
		(3) The "element" shall not be nullptr.
		(4) The fields pointer to left and right nodes of "element" should be nullptr. Because these pointers will be overwriten (pontential memory leak).
		(5) The function doesn't validate ordering
	*/
	template<typename key_t, typename value_t>
	void insertRight(OrdenedLinkedMap<key_t, value_t>* list, Node<key_t, value_t>* element, Node<key_t, value_t>* position) {
		Node<key_t, value_t>* next = position->next; // copy the reference to left node of "position"
		element->next = next; // (1) link "next" as the right node of "element"
		element->prev = position; // (2) link "position" as the left node of "element"
		position->next = element; // (3) link "element" as the right node of "position"
		if(next != nullptr) {// the "position" is not the right edge of linked ordened map
			next->prev = element;
			/* Crescent links:
			*	"position" points to "element" as right node [because of (3)]
			* 	"element" points to "next" as right node [because of (1)]
			* Decrescent links:
			*	"next" points to "element" as left node 
			*	"element" points to "previuos" as left node [because of (2)]
			* Diagram: 
			* 	position <-> element <-> next
			*/
		} else {
			list->last = element;
			/* Crescent links:
			*	"position" points to "element" as right node [because of (3)]
			* 	"element" is the right edge node of ordened linked map
			* Decrescent links:
			*	"element" is the right edge node of ordened linked map
			*	"element" points to "previuos" as left node [because of (2)]
			* Diagram: 
			* 	position <-> element <- (right edge)
			*/
		}
		
	}
	/*
	Inserts a node at begining of linked map. That is, inserts and update the first 
	[Warnings]:
		(1) If the linked ordened map is empty, then the left edge shall be nullptr
		(2) Inherits warnings (1), (3), (4) and (5) from insertLeft()
	*/
	template<typename key_t, typename value_t>
	void appendLeft(OrdenedLinkedMap<key_t, value_t>* list, Node<key_t, value_t>* element) {
		Node<key_t, value_t>* first = list->first;
		if(first == nullptr) { // the linked ordened map is empty
			list->first = element; // if the linked ordened map is not empty, the left edge shalln't be nullptr
			list->last = element; // if the linked ordened map is not empty, the right edge shalln't be nullptr
		} else { // the linked ordened map is not empty
			assert(first->prev == nullptr); // as said in the warning (1)
			insertLeft(list, element, first); // insert at left side of left edge 
		}
	}
	/*
	Inserts a node at begining of linked map. That is, inserts and update the first 
	[Warnings]:
		(1) If the linked ordened map is empty, then the right edge shall be nullptr
		(2) Inherits warnings (1), (3), (4) and (5) from insertRight()
	*/
	template<typename key_t, typename value_t>
	void appendRight(OrdenedLinkedMap<key_t, value_t>* list, Node<key_t, value_t>* element) {
		Node<key_t, value_t>* last = list->last;
		if(last == nullptr) {
			list->first = element; // if the linked ordened map is not empty, the left edge shalln't be nullptr
			list->last = element; // if the linked ordened map is not empty, the right edge shalln't be nullptr
		} else {
			assert(last->next == nullptr); // as said in the warning (1)
			insertRight(list, element, last); // insert at right side of right edge
		}
	}
	/* insert a node between two nodes. If exactly one of these two nodes is nullptr, then insert into corresponding edge of the map.
	[Warnings]:
		(1) The "left" node shall be at left side of "right" node. 
		(2) The "right" node shall be at right side of "right" node. 
		(3) Inherits warnings (1) and (2) from appendLeft()
		(3) Inherits warnings (1) and (2) from appendRight()
		(4) Both "left" and "right"
	*/
	template<typename key_t, typename value_t>
	void insertBetween(OrdenedLinkedMap<key_t, value_t>* list, Node<key_t, value_t>* element, Node<key_t, value_t>* left, Node<key_t, value_t>* right) {
		//assert(left != nullptr && left->next == right); // as said in warning (1)
		//assert(right != nullptr && right->prev == left); // as said in warning (2)
		if(left == nullptr) { // right is the left edge of ordened linked map
			appendLeft(list, element); // insert at begining
		} else { // right is not the left edge of ordened linked map
			insertRight(list, element, left); // insert at right side of "left" (and therefore at left side of "right")
		}
	}
	template<typename key_t, typename value_t>
	void clear_search_interval(SearchInterval<key_t, value_t>* into) {
		into->lt = nullptr;
		into->eq = nullptr;
		into->gt = nullptr;
	}
	template<typename key_t, typename value_t> 
	void debug_node(Node<key_t, value_t>* source, std::wostream& output){
		if(source == nullptr)
			output << "nullptr";
		else 
			output << *source->key;
	}
	
	template<typename key_t, typename value_t>
	SearchInterval<key_t, value_t>* partial_find(SearchInterval<key_t, value_t>* state, OrdenedLinkedMap<key_t, value_t>* list, key_t* key) {
		/*
		The target is the node the first node that key field is equal to "key"
		A candidate is a node that could be the target. Exists up to one candidate, because the key fields's value of all nodes after the first candidate are greater than key, and, therefore, all subsequent nodes aren't candidates.
		*/
		clear_search_interval(state);
		cmp_fn<key_t> compare = list->compare; // store the function to compare in the stack
		Node<key_t, value_t>* pre = nullptr; // If exists at least one element in the list, then it will be pointer to last element that is lesser than key. Otherwise, it will remain as nullptr
		Node<key_t, value_t>* now = list->first; // A candidate, if exists. Otherwise, it is nullptr (including the case of the list begin empty, that is, when list->first is nullptr). 
		int cmp = 0; // Comparation flag. Non-zero means that the candidate was found. 
		while(now != nullptr) {
			/*
			compare(now->key, key) + 1 <= 0 --> now->key < key
			compare(now->key, key) + 1 == 1 --> now->key == key
			compare(now->key, key) + 1 >= 2 --> now->key > key
			*/
			cmp = compare(now->key, key) + 1; // update the comparation flag
			if(cmp > 0) // now->key >= key
				break;
			// now->key < key
			pre = now; // update the last node
			now = now->next; // advance to next node
		}
		
		#ifdef __SHOW_SEARCHING__
		std::wcout << "compare: " << cmp << std::endl;
		std::wcout << "lt: ";
		debug_node(pre, std::wcout);
		std::wcout << std::endl;
		#endif
		
		state->lt = pre; // The variable "pre" contains the last node that is lesser than the target, or nullptr if not exists.
		// "Now" will be nullptr if, and only if, "cmp" is not equal to zero.
		if(cmp <= 0) // there isn't a node is greater or equal to target.
			return state;
		// "now" is the candidate
		if(cmp == 1) { // target was found
			state->eq = now; // the candidate is the target
			state->gt = now->next; // the greater neightbor is the first node that key is greater than target
		} else { // target was not found
			state->gt = now; // the candidate is the first node that key is greater than target
		}
		#ifdef __SHOW_SEARCHING__
		std::wcout << "eq: ";
		debug_node(state->eq, std::wcout);
		std::wcout << std::endl;
		
		std::wcout << "gt: ";
		debug_node(state->gt, std::wcout);
		std::wcout << std::endl;
		#endif
		return state;
	}
	/*
	return a boolean value representing if the ordened linked map was empty when the partial_find() was done. This could depends of how partial_find() was implemented, therefore should be always used for this situation.
	*/
	template<typename key_t, typename value_t>
	bool intv_empty(SearchInterval<key_t, value_t>* source){
		return source->eq == nullptr && source->lt == nullptr && source->gt == nullptr;
	}
	template<typename key_t, typename value_t>
	bool intv_found(SearchInterval<key_t, value_t>* source) {
		return source->eq != nullptr;
	}
	// the cmp_fn implementation for type "int"*((int*) left);
	int compare_int(const int* left, const int* right) {
		if(left < right) 
			return -1;
		if(left == right)
			return 0;
		return 1;
	}
	// the cmp_fn implementation for type "std::string"
	int compare_string(const std::string* left, const std::string* right) {
		return left->compare(*right);
	}
	// the cmp_fn implementation for type "std::wstring"
	int compare_wstring(const std::wstring* left, const std::wstring* right) {
		//std::wcout << "cmp_wstring: " << *left << std::endl;
		//std::wcout << "cmp_wstring: " << *right << std::endl;
		int cmp_result = left->compare(*right);
		#ifdef __SHOW_COMPARATIONS__
		std::wcout << "\t[" << cmp_result << "] "<< *left << L" ";
		std::wcout << get_cmp_symbol(cmp_result);
		std::wcout << L" " << *right << std::endl;
		#endif
		return cmp_result;
	}
	// initialize the node with defaults values 
	template<typename key_t, typename value_t>
	void init_node(Node<key_t, value_t>* into, key_t* key, value_t* value) {
		into->key = key; // assign the key
		into->value = value; // assign the value
		into->next = nullptr; // null-initialization
		into->prev = nullptr; // null-initialization
		#ifdef LINKMARK
		into->linkmark = CREATION_MARK;
		#endif
	}
	// create a node in the heap and then initialize with defaults value
	template<typename key_t, typename value_t>
	Node<key_t, value_t>* create_node(key_t* key, value_t* value) {
		Node<key_t, value_t>* tmp = new Node<key_t, value_t>; // create a node in heap 
		init_node(tmp, key, value); // initialize with defaults value
		return tmp; // returns the node
	}
	/*
	Atomically returns a node that contains the key, if exists; or create and returns a new node, if not exists.
	[Warnings]:
		(1) If not exists a node that contains the key, then will be created a new node that has the field value as nullptr. Make sure to properly initializate or handle the nullptr in value
	*/
	template<typename key_t, typename value_t>
	Node<key_t, value_t>* find_or_create(OrdenedLinkedMap<key_t, value_t>* list, key_t* key, bool* found) {
		SearchInterval<key_t, value_t> state; // create a temporary state machine
		*found = intv_found(partial_find(&state, list, key));
		if(*found) { // the key was found
			return state.eq; // return the node that contains the key
		}
		Node<key_t, value_t>* element = create_node<key_t, value_t>(key, nullptr);	// as said in warning (1)
		if(intv_empty(&state)) { // the list is empty
			appendLeft(list, element); // insert at begining
		} else {
			insertBetween(list, element, state.lt, state.gt); // insert between the node last node that is lesser than target; and the first node that is greater than target
		}
		return element; // return the element which is properly poisitioned, but partially initialized
	}
	namespace check_edge {
		enum EdgeStatus {
			edge_nullability, // both edge link and node nullability was warranted
			edge_leftlink_error, // the left link shall be null when right link is null
			edge_rightlink_error,// the right link shall be null when left link is null
			edge_leftnode_error, // the left node of left edge node isn't null
			edge_rightnode_error, // the right node of right edge node isn't null
			unknown_edgestatus // only the edge link was warranted
		};
		/*
		Check the double edge node nullability:
		- The left edge shall points to nullptr as left node (edge node nullability)
		- The right edge shall points to nullptr as right node (edge node nullability)
		[warnings]: 
		(1) It is intented to be called after check_edgelink(). Otherwise, you shall warranty that two egdes links aren't nullptr
		*/
		template<typename key_t, typename value_t>
		EdgeStatus check_edgenode(const OrdenedLinkedMap<key_t, value_t>* list) {
			if(list->first->prev != nullptr) // error in left edge node
				return EdgeStatus::edge_leftnode_error;
			if(list->last->next != nullptr) // error in right edge node
				return EdgeStatus::edge_rightnode_error;
			return EdgeStatus::edge_nullability; // sucess
		}/*
		Check the double edge link nullability:
		- The left edge points to nullptr as right node if, and only if, the right edge points to nullptr (Double edge link nullability)
		And returns a error constant if fails. Otherwise, returns EdgeStatus::edge_nullability if both edge link and node nullability are warranted, or EdgeStatus::unknown_edgestatus if the node nullability is unknown.
		*/
		template<typename key_t, typename value_t>
		EdgeStatus check_edgelink(const OrdenedLinkedMap<key_t, value_t>* list) {
			if(list->first == nullptr) {
				if(list->last == nullptr) // both edges are nullptr (sucess)
					return EdgeStatus::edge_nullability;
				return EdgeStatus::edge_rightlink_error; // the right edge shall be nullptr (error) 
			}  // the left edge is not nullptr
			if(list->last == nullptr) // the right edge is nullptr (error)
				return EdgeStatus::edge_leftlink_error; // the left edge shall be nullptr
			return EdgeStatus::unknown_edgestatus; // the edge links are sane. But the edge nodes status still not covered
		}
		/*
		Atomically checks the edges links and edges nodes. Returns only EdgeStatus::edge_nullability on sucess. Otherwise returns the respective error code.
		*/
		template<typename key_t, typename value_t>
		EdgeStatus check_edges(const OrdenedLinkedMap<key_t, value_t>* list) {
			EdgeStatus status = check_edgelink(list); // check edges links
			if(status != EdgeStatus::unknown_edgestatus) // the list is empty (which has edge node warranty) or has link error
				return status; // in both cases the test already ended
			return check_edgenode(list); // finally checks the nodes (they are warrented to be not null)
		}
	}
	
	namespace hard_linkcheck {
		using NodeIterator::isalive;
		template<typename key_t, typename value_t>
		size_t get_omega(const Node<key_t, value_t>* element, const Node<key_t, value_t>** nodes, size_t amount) {
			size_t pos = 0;
			while(pos < amount && element != nodes[pos]) 
				pos++;
			return pos;
		}
		template<typename key_t, typename value_t>
		Node<key_t, value_t>* check_unordered (NodeIterator::NodeIterator<key_t, value_t>* state, const Node<key_t, value_t>** nodes, size_t amount) {
			while(isalive(state)) {
				Node<key_t, value_t>* now = state->now;
				#ifdef LINKMARK
				if(!(now->linkmark & CREATION_MARK))
					return now;
				#endif
				if(amount <= get_omega(now, nodes, amount)) 
					return now;
				NodeIterator::next(state);
			}
			return nullptr;
		}
		template<typename key_t, typename value_t>
		Node<key_t, value_t>* check_positioned(NodeIterator::NodeIterator<key_t, value_t>* state, const Node<key_t, value_t>** nodes, size_t amount) {
			for(size_t i = 0; i < amount && isalive(state); i++) {
				if(nodes[i] != state->now)
					return state->now;
				NodeIterator::next(state);
			}
			return nullptr;
		}
		template<typename key_t, typename value_t>
		Node<key_t, value_t>* check_ordered(NodeIterator::NodeIterator<key_t, value_t>* state, cmp_fn<key_t> compare) {
			Node<key_t, value_t>* prev = nullptr;
			int status = 0;
			int exact = state->reverse ? 1 : -1;
			while(isalive(state)) {
				if(prev != nullptr)
					status = compare(prev, state->now);
				if(status != 0 && status != exact)
					return state->now;
				prev = NodeIterator::next(state);
			}
			return nullptr;
		}
	}
	namespace soft_linkcheck {
		#ifdef LINKMARK
		bool has_linkmark(const Node* now, bool reverse) {
			if(reverse) {
				return now->linkmark & DECRESCENT_LINKMARK;
			} else {
				return now->linkmark & CRESCENT_LINKMARK;
			}
		}
		Node* check_circularref(NodeIterator::NodeIterator* state) {
			while(NodeIterator::isalive(state)) {
				if(has_linkmark(state->now, state->reverse))
					return state->now;
				NodeIterator::next(state);
			}
			return nullptr;
		}
		bool check_reachable(const OrdenedLinkedMap* list) {
			Node* first = list->first;
			Node* last = list->last;
			if(first != nullptr && last != nullptr) {
				if(first->linkmark & DECRESCENT_LINKMARK)
					return true;
				if(last->linkmark & CRESCENT_LINKMARK)
					return true;
			}
			return false;
		}
		const int CLEAR_LINKMARK = (DECRESCENT_LINKMARK | CRESCENT_LINKMARK);
		void clear_mark(NodeIterator::NodeIterator* state, int mask) {
			while(NodeIterator::isalive()) {
				Node* now = NodeIterator::next(state);
				now->linkmark = now->linkmark ^ (now->linkmark & mask);
			}
		}
		#endif
		template<typename key_t, typename value_t>
		bool check_crosslink_of(const Node<key_t, value_t>* element) {
				Node<key_t, value_t>* prev = element->prev;
				Node<key_t, value_t>* next = element->next;
					if(next != nullptr && next->prev != element)
					return false;
				if(prev != nullptr && prev->next != element)
					return false;
				return true;
			}
		template<typename key_t, typename value_t>
		Node<key_t, value_t>* check_crosslink(NodeIterator::NodeIterator<key_t, value_t>* state) {
			while(NodeIterator::isalive(state)) {
				if(!check_crosslink_of(state->now))
					return state->now;
				NodeIterator::next(state);
			}
			return nullptr;
		}
	}
	
	namespace test {
		using namespace check_edge;
		// prototypes
		void test_edge_insertion(bool debug);
		void test_string_comparation();
		void show_cmp_wstring(const std::wstring* left, const std::wstring* right);
		template<typename value_t>
		void printnode_int(const Node<int, value_t>* src);
		template<typename value_t> 
		void printmap_int(NodeIterator::NodeIterator<int, value_t>* state);
		void test_check_edge(){
			Node<int, int>* nodes [3];
			int keys[] = {11, 12, 14};
			int values[] = {-1, 7, 21};
			for(size_t i = 0; i < 3; i++)
				nodes[i] = create_node(keys + i, values + i);
			OrdenedLinkedMap<int, int> list = {nullptr, nullptr, compare_int};
			
			assert(check_edges(&list) == edge_nullability);
			
			list.first = nodes[2];
			assert(check_edges(&list) == edge_leftlink_error);
			list.first = nullptr;
			
			list.last = nodes[2];
			assert(check_edges(&list) == edge_rightlink_error);
			list.last = nullptr;
			
			appendRight(&list, nodes[0]);
			appendRight(&list, nodes[1]);
			assert(check_edges(&list) == edge_nullability);
			
			list.first->prev = nodes[2];
			assert(check_edges(&list) == edge_leftnode_error);
			list.first->prev = nullptr;
			
			list.last->next = nodes[2];
			assert(check_edges(&list) == edge_rightnode_error);
			list.last->next = nullptr;
			
			assert(check_edges(&list) == edge_nullability);
			for(size_t i = 0; i < 3; i++)
				delete nodes[i];
		}
		
		void test_edge_insertion(bool debug) {
			const size_t left_nodes = 6;
			const size_t right_nodes = 6;
			const size_t nodes_amount = left_nodes + right_nodes;
			int left_keys[] = {10, 15, 16, 97, -113, 48};
			int left_values[] = {20, 82, -72, 37, 51, 45};
			int right_keys[] = {0, 80, 59, 68, 31, 56};
			int right_values[] = {0, 79, 53, 111, -7, 83};
			const Node<int, int>* nodes[12];
			OrdenedLinkedMap<int, int> list = {nullptr, nullptr, compare_int};
			NodeIterator::NodeIterator<int, int> mapper;
			assert(check_edges(&list) == edge_nullability);
			for(size_t i = 0; i < left_nodes; i++) {
				Node<int, int>* data = create_node(left_keys + i, left_values + i);
				nodes[(left_nodes - 1) - i] = data;
				appendLeft(&list, data);
				assert(check_edges(&list) == edge_nullability);
			}
			for(size_t i = 0; i < right_nodes; i++) {
				Node<int, int>* data = create_node(right_keys + i, right_values + i);
				nodes[left_nodes + i] = data;
				appendRight(&list, data);
				assert(check_edges(&list) == edge_nullability);
			}
			if(debug) {
				printmap_int(create(&mapper, list.first));
				std::wcout << std::endl;
				printmap_int(createReverse(&mapper, list.last));
			}
			assert(hard_linkcheck::check_positioned(create(&mapper, list.first), nodes, nodes_amount) == nullptr);
			create(&mapper, list.first);
			assert(soft_linkcheck::check_crosslink(&mapper) == nullptr);
			//assert(check_leftlink(&list, nodes, nodes_amount) == nullptr);
			//assert(check_rightlink(&list, nodes, nodes_amount) == nullptr);
			for(size_t i = 0; i < 12; i++)
				delete nodes[i];
		}
		
		void test_string_comparation(){
			const wchar_t* src[] = {L"é", L"és", L"bem", L"bom", L"já", L"com", L"de", L"e", L"e"};
			std::wstring* text[9];
			for(size_t i = 0; i < 9; i++)
				text[i] = new std::wstring(src[i]);
			std::wcout << std::flush;
			for(size_t i = 0; i < 8; i++) {
				show_cmp_wstring(text[i], text[i+1]);
			}
			std::wcout << std::endl;
			for(size_t i = 0; i < 9; i++)
				delete text[i];
		}
		template<typename key_t, typename value_t>
		void clear_nodes(size_t amount, Node<key_t, value_t>** nodes) {
			for(size_t i = 0; i < amount; i++)
				delete nodes[i];
			delete[] nodes;
		}
		template<typename key_t, typename value_t>
		Node<key_t, value_t>** test_partial_search(key_t* keys[3], OrdenedLinkedMap<key_t, value_t>* list){
			Node<key_t, value_t>** nodes = new Node<key_t, value_t> *[3];
			for(size_t i = 0; i < 3; i++)
				nodes[i] = create_node<key_t, value_t>(keys[i], nullptr);
			SearchInterval<key_t, value_t> state;
			assert(intv_empty(partial_find(&state, list, keys[0])));
			
			appendLeft(list, nodes[2]);
			
			partial_find<key_t, value_t>(&state, list, keys[0]);
			assert(state.lt == nullptr);
			assert(state.eq == nullptr);
			assert(state.gt == nodes[2]);
			assert(!intv_empty(&state));
			
			partial_find<key_t, value_t>(&state, list, keys[2]);
			assert(state.lt == nullptr);
			assert(state.eq == nodes[2]);
			assert(state.gt == nullptr);
			assert(!intv_empty(&state));
			
			appendLeft(list, nodes[0]);
			
			partial_find(&state, list, keys[0]);
			assert(state.lt == nullptr);
			assert(state.eq == nodes[0]);
			assert(state.gt == nodes[2]);
			assert(!intv_empty(&state));
			
			partial_find(&state, list, keys[1]);
			assert(state.lt == nodes[0]);
			assert(state.eq == nullptr);
			assert(state.gt == nodes[2]);
			assert(!intv_empty(&state));
			
			partial_find(&state, list, keys[2]);
			assert(state.lt == nodes[0]);
			assert(state.eq == nodes[2]);
			assert(state.gt == nullptr);
			assert(!intv_empty(&state));
			
			insertBetween(list, nodes[1], nodes[0], nodes[2]);
			
			partial_find(&state, list, keys[0]);
			assert(state.lt == nullptr);
			assert(state.eq == nodes[0]);
			assert(state.gt == nodes[1]);
			assert(!intv_empty(&state));
			
			partial_find(&state, list, keys[1]);
			assert(state.lt == nodes[0]);
			assert(state.eq == nodes[1]);
			assert(state.gt == nodes[2]);
			assert(!intv_empty(&state));
			
			partial_find(&state, list, keys[2]);
			assert(state.lt == nodes[1]);
			assert(state.eq == nodes[2]);
			assert(state.gt == nullptr);
			assert(!intv_empty(&state));
			return nodes;
		}
		void test_psearch_int() {
			int keys[] = {-1, 27, 44};
			int* pkeys[3];
			for(size_t i = 0; i < 3; i++)
				pkeys[i] = keys + i;
			OrdenedLinkedMap<int, void*> list = {nullptr, nullptr, compare_int};
			clear_nodes(3, test_partial_search(pkeys, &list));
		}
		void test_psearch_wstring(){
			const wchar_t* keys[] = {
				L"abc", L"abd", L"xyz",
			};
			std::wstring* pkeys[3];
			for(size_t i = 0; i < 3; i++)
				pkeys[i] = new std::wstring(keys[i]);
			OrdenedLinkedMap<std::wstring, void*> list = {nullptr, nullptr, compare_wstring};
			clear_nodes(3, test_partial_search(pkeys, &list));
			for(size_t i = 0; i < 3; i++)
				delete pkeys[i];
		}
		template<typename key_t, typename value_t> 
		Node<key_t, value_t>** test_find_or_create(key_t* source[4], OrdenedLinkedMap<key_t, value_t>* list){
			Node<key_t, value_t>** nodes = new Node<key_t, value_t> *[4];
			bool found;
			nodes[0] = find_or_create<key_t, value_t>(list, source[0], &found);
			assert(!found);
			assert(nodes[0]->value == nullptr);
			
			assert(find_or_create(list, source[0], &found) == nodes[0]);
			assert(found);
			
			nodes[3] = find_or_create(list, source[3], &found);
			assert(!found);
			
			nodes[2] = find_or_create(list, source[2], &found);
			assert(!found);
			
			assert(find_or_create(list, source[3], &found) == nodes[3]);
			assert(found);
			assert(find_or_create(list, source[2], &found) == nodes[2]);
			assert(found);
			
			nodes[1] = find_or_create(list, source[1], &found);
			assert(!found);
			
			for(size_t i = 0; i < 4; i++) {
				assert(find_or_create(list, source[i], &found) == nodes[i]);
				assert(found);
				assert(nodes[i] != nullptr);
				assert(nodes[i]->value == nullptr);
			}
			return nodes;
		}
		void test_foc_int(){
			int keys[4] = {-1, 27, 44, 70};
			int* pkeys[4];
			for(size_t i = 0; i < 4; i++)
				pkeys[i] = keys + i;
			OrdenedLinkedMap<int, void*> list = {nullptr, nullptr, compare_int};
			clear_nodes(4, test_find_or_create(pkeys, &list));
		}
		void test_foc_wstring(){
			const wchar_t* keys[] = {
				L"Hello", L"my", L"darling", L"friend",
			};
			std::wstring* pkeys[4];
			for(size_t i = 0; i < 4; i++)
				pkeys[i] = new std::wstring(keys[i]);
			OrdenedLinkedMap<std::wstring, void*> list = {nullptr, nullptr, compare_wstring};
			clear_nodes(4, test_find_or_create(pkeys, &list));
			for(size_t i = 0; i < 4; i++)
				delete pkeys[i];
		}
		void show_cmp_wstring(const std::wstring* left, const std::wstring* right){
			std::wcout << *left << L" ";
			std::wcout << get_cmp_symbol(compare_wstring(left, right));
			std::wcout << L" " << *right << std::endl;
		}
		template<typename value_t> 
		void printnode_int(const Node<int, value_t>* src){
			std::wcout << "key=" << *src->key << " ";
			std::wcout << "adress=" << (void*) src << " ";
			#ifdef LINKMARK
			std::wcout << "linkmark= " << src->linkmark;
			#endif
			std::wcout << "value=" << *src->value << std::endl;
		}
		template<typename value_t> 
		void printmap_int(NodeIterator::NodeIterator<int, value_t>* state) {
			while(NodeIterator::isalive(state))
				printnode_int(NodeIterator::next(state));
		}
	}
}
namespace stateview {
	template<typename word_t> void system_pause(word_t message);
	void system_pause();
	void use_memory(size_t size);
};
/*namespace getword {
	void getwchar_f(FILE* source, wchar_t* into) {
		*into = getwc(source);
	}
	void getwchar_f(std::wistream* source, wchar_t* into) {
		*source >> *into;
	}
	template<typename source_t>
	int get_rawword(source_t source, wchar_t* buffer, size_t amount) {
		size_t end = amount - 1;
		size_t pos = 0;
		while(pos < end) {
			wchar_t tmp;
			getwchar_f(source, &tmp);
			if(tmp == WEOF) 
				return 1;			
			if(iswspace(tmp))
				break;
			buffer[pos++] = tmp;
		}
		buffer[pos] = '\0';
		return 0;
	}
}*/
namespace word_parse {
	// Look-ahead tokenizer
	template<typename charT>
		using buffer_t = std::basic_string<charT>;
	template <typename charT>
	struct wparse {
		buffer_t<charT>* buffer; // the quere used for storing the current head
		size_t pos; // current position in the buffer
	};
	/*
	Intializate the look-ahead tokenizer, and provide a initial size for its quere
	*/
	template<typename charT>
	void initialize(wparse<charT>* state, size_t initial_size) {
		state->buffer = new buffer_t<charT>; // create the buffer in the heap
		if(initial_size) // preallocate the buffer
			state->buffer->reserve(initial_size); // reserving a greater space avoids reallocating
		state->pos = 0; // no word was processed
	}
	// Intializate the look-ahead tokenizer and prealloc the default size
	template<typename charT>
	void initialize(wparse<charT>* state) {
		initialize(state, 0);
	}
	/*
	Copy the text into quere. If amount is zero, it does nothing.
	[Warnings]:
		(1) The function couldn't check if the source text ended with '\0' before maximum size, so you shall ensure that "amount" is lesser or equal to both maximum size of source text and the index of first '\0' if exists. 
	*/
	template<typename charT>
	void feed(wparse<charT>* state, const charT* buffer, size_t amount) {
		if(amount)
			state->buffer->append(buffer, amount);
	}
	// Copy the text into quere.
	template<typename charT>
	void feed(wparse<charT>* state, const buffer_t<charT> buffer) {
		*(state->buffer) += buffer;
		//std::wcout << "Buffer: " << *(state->buffer) << std::endl;
	}
	// check if a wide character match the spec
	bool is_gen_alphanum(wchar_t src) {
		bool flag = !iswspace(src) && iswalnum(src);
		return flag;
	}
	// check if a character match the spec
	bool is_gen_alphanum(char src) {
		bool flag = !isspace(src) && isalnum(src);
		return flag;
	}
	/* Calculates the position of the first character that matchs the spec, if all characters in buffer doesn't match the spec, return the "end". So all charater before that position are ignored in the quere
	*/
	template<typename charT>
	size_t exclude_inter(buffer_t<charT> buffer, size_t pos, size_t end) {
		while(pos < end && !is_gen_alphanum(buffer[pos]))
			pos++;
		return pos;
	}
	/* Calculates the position of the first character that doesn't match the spec, if all characters in buffer matchs the spec, return the "end". So all charater before that position will be part of token
	*/
	template<typename charT>
	size_t include_inter(buffer_t<charT> buffer, size_t pos, size_t end) {
		while(pos < end && is_gen_alphanum(buffer[pos]))
			pos++;
		return pos;
	}
	// same as exclude_inter(std::wstring, size_t, size_t), but the "end" is properly configured
	template<typename charT>
	size_t exclude_inter(buffer_t<charT> buffer, size_t pos) {
		return exclude_inter(buffer, pos, buffer.size());
	}
	// same as include_inter(std::wstring, size_t, size_t), but the "end" is properly configured
	template<typename charT>
	size_t include_inter(buffer_t<charT> buffer, size_t pos) {
		return include_inter(buffer, pos, buffer.size());
	}
	// remove all character from the quere that was already processed
	template<typename charT>
	void clear_buffer(wparse<charT>* state) {
		size_t ipos = state->pos; // the first character that wasn't processed
		if(ipos) { // indepotent: calling twice doesn't anything
			state->buffer->erase(0, ipos); // clear all character until before the first character that wasn't processed, and moves all remaining character to begining
			state->pos = 0; // update the position
		}
	}
	/*
	Clears all characters that was processed or doesn't match the spec. And set properly the "pos" to match the first valid character. Effectivilly removing garbage from the quere
	*/
	template<typename charT>
	void ignore(wparse<charT>* state) {
		buffer_t<charT> buffer = *(state->buffer); // take a reference to the buffer
		size_t bpos = state->pos; // get actaul pos
		size_t ignored = exclude_inter(buffer, bpos); // search first valid character if exists
		state->pos = ignored; // advance the cursor to first valid character or to end if not exists
		clear_buffer(state); // remove all characters before the first valid character
	}
	// checks if all character aren't processed
	template<typename charT>
	bool is_not_empty(const wparse<charT>* state) {
		size_t buffer_size = state->buffer->size();
		return state->pos < buffer_size;
	}
	//Copies a substring and returns a new wstring allocated in heap.
	template<typename charT>
	buffer_t<charT>* copy_from(const buffer_t<charT> source, size_t ipos, size_t epos) {
		// naive implementation
		/*if(epos > source.size())
			return nullptr;
		const size_t size = ipos < epos ? epos - ipos : 0;
		const wchar_t* sbuffer = source.c_str(); // inline the reference
		std::wstring* buffer = new std::wstring; // allocate the wstring heap
		buffer->reserve(size); // prelloc stace
		for(size_t i = ipos; i < epos; i++)
			buffer->push_back(sbuffer[i]);
		return buffer;*/
		// abstract implementation
		buffer_t<charT>* buffer = new buffer_t<charT>;
		buffer_t<charT> temp = source.substr(ipos, epos);
		buffer->swap(temp);
		return buffer;
	}
	
	enum status_t {
		read_sucess,
		empty_word,
		polling
	};
	/*
	Take a non-empty word from tokenizer if avaliable. Set "status" with error code, unless that it is nullptr, and returns nullptr otherwise.
	[Warnings]: 
		(1) All character that doesn't match the spec shall be ignored before calling it
	*/
	status_t analize_status(size_t word_end, size_t ipos, size_t bsize, status_t* status) {
		status_t _err = status_t::read_sucess;
		if (word_end == ipos) 
			_err = status_t::empty_word;
		else if(word_end >= bsize)
			_err = status_t::polling;
		if(status != nullptr)
			*status = _err;
		return _err;
	}
	template<typename charT>
	buffer_t<charT>* read_word(wparse<charT>* state, status_t* status) {
		const size_t ipos = state->pos;
		const size_t bsize = state->buffer->size();
		buffer_t<charT> buffer = *(state->buffer);
		const size_t word_end = include_inter(buffer, ipos);
		status_t _err = analize_status(word_end, ipos, bsize, status);
		if(_err != status_t::read_sucess)
			return nullptr;
		state->pos = word_end;
		buffer_t<charT>* cbuffer = copy_from(*state->buffer, ipos, word_end);
		return cbuffer;
	}
	template<typename charT>
	void destroy_state(wparse<charT>* state) {
		delete state->buffer;
	}
	namespace test {
		void simple_test() {
			const wchar_t* message = L"Hello my old friend! It's almost 18:00 o'clock\n";
			const wchar_t* expected[] = {
				L"Hello", L"my", L"old", L"friend", L"It", L"s", L"almost", L"18", L"00",
				L"o", L"clock"
			};
			size_t position = 0;
			wparse<wchar_t> state;
			initialize(&state, 256);
			feed(&state, message, wcslen(message));
			#ifdef __VERBOSE_TESTS__
				std::wcout << message;
			#endif
			while(is_not_empty(&state)) {
				std::wstring* word = read_word(&state, nullptr);
				#ifdef __VERBOSE_TESTS__
					std::wcout << ": " << *word << std::endl;
				#endif
				assert(*word == std::wstring(expected[position]));
				assert(position < 11);
				position++;
				delete word;
				ignore(&state);
			}
			destroy_state(&state);
		}
		
		template<typename charT>
		void echo_test(charT end_in, std::basic_ostream<charT>& into, std::basic_istream<charT>& _from) {
			wparse<charT> state;
			initialize(&state, 256);
			buffer_t<charT> buffer;
			while(true) {
				into << ">> "; // print the prompt
				if(!std::getline(_from, buffer, end_in)) // read a line into buffer
					break; // stop if fail bit or bad bit is setted
				buffer.push_back(' '); // end the last word
				feed(&state, buffer.c_str(), buffer.size()); // feed the state machine
				while(is_not_empty(&state)) {
					ignore(&state); // ignore bad character from state machine
					buffer_t<charT>* word = read_word(&state, nullptr); // take one word from state machine
					if(word == nullptr)
						break;
					into << *word << std::endl; // print the word
					delete word; // free allocated space to word
				}
				buffer.clear(); // clear the buffer
			}
			destroy_state(&state);
		}
		template<typename charT>
			void echo_test();
			
		template<>
		void echo_test<wchar_t>() {
			echo_test(L'\n', std::wcout, std::wcin);
		}
		template<>
		void echo_test<char>() {
			echo_test('\n', std::cout, std::cin);
		}
	}
}
namespace word_counter {
	namespace LLDE = ordened_linked_map; 
	namespace NodeIterator = LLDE::NodeIterator;
	template<typename word_t>
		using cmap_t = LLDE::OrdenedLinkedMap<word_t, size_t>;
	template<typename word_t>
		using cnode_t = LLDE::Node<std::wstring, size_t>;
	using std::basic_string;
	using std::basic_ostream;
	template<typename charT>
		using bstring = basic_string<charT>;
	template<typename charT>
		using bostream = basic_ostream<charT>;
	template<typename word_t>
	struct WordCounter {
		LLDE::OrdenedLinkedMap<word_t, size_t>* list; // the double linked map
		size_t src_id; // the actual index of counter
		size_t amount; // the amount of files which will be processed
	};
	template<typename word_t>
	void initialize(WordCounter<word_t>* state, LLDE::cmp_fn<word_t> compare, size_t amount) {
		state->list = new LLDE::OrdenedLinkedMap<word_t, size_t>;
		LLDE::initalize_empty(state->list, compare);
		state->amount = amount;
		state->src_id = 0;
	}
	void initialize(WordCounter<std::wstring>* state, size_t amount) {
		initialize(state, LLDE::compare_wstring, amount);
	}
	void initialize(WordCounter<std::string>* state, size_t amount) {
		initialize(state, LLDE::compare_string, amount);
	}
	template<typename word_t>
	void next_source(WordCounter<word_t>* state) {
		state->src_id++;
	}
	template<typename word_t>
	bool isalive(WordCounter<word_t>* state) {
		return state->src_id < state->amount;
	}
	template<typename word_t>
	bool insert_word(WordCounter<word_t>* state, word_t* word) {
		bool found;
		LLDE::Node<word_t, size_t>* element = LLDE::find_or_create(state->list, word, &found);
		#ifdef __SHOW_IF_FOUND_INSERTION__
		std::wcout << (found ? "" : "not ") << "found: " << *word << std::endl;
		#endif
		size_t* counter_map = element->value;
		if(!found) {
			counter_map = new size_t[state->amount];
			for(size_t i = 0; i < state->amount; i++)
				counter_map[i] = 0;
			element->value = counter_map;
		}
		counter_map[state->src_id]++;
		return found;
	}
	template<typename word_t>
	void insert_or_dealloc(WordCounter<word_t>* state, word_t* word) {
		#ifdef __SHOW_INSERTIONS__
		std::wcout << "@inserting: " << *word << std::endl;
		#endif
		if(insert_word(state, word)) {
			#ifdef __SHOW_INSERTIONS__
			std::wcout << "#already in counter: " << *word << std::endl;
			#endif
			delete word;
		}
			
	}
	template<typename charT> 
	void print_cnode(cnode_t<bstring<charT>>* node, size_t amount, bostream<charT>& into) {
		into << *node->key;
		size_t* counter_map = node->value;
		for(size_t i = 0; i < amount; i++)
			into << " "  << counter_map[i];
		into << std::endl;
	}
	template<typename charT>
	void print_counter(WordCounter<bstring<charT>>* counter, bostream<charT>& into) {
		NodeIterator::NodeIterator<bstring<charT>, size_t> state;
		NodeIterator::create(&state, counter->list->first);
		while(NodeIterator::isalive(&state)) {
			LLDE::Node<bstring<charT>, size_t>* now = NodeIterator::next(&state);
			print_cnode(now, counter->src_id, into);
		}
	}
	template<typename word_t, bool keep_key = false>
	void destroy_counter(WordCounter<word_t>* counter) {
		NodeIterator::NodeIterator<word_t, size_t> state;
		NodeIterator::create(&state, counter->list->first);
		while(NodeIterator::isalive(&state)) {
			LLDE::Node<word_t, size_t>* now = NodeIterator::next(&state);
			if(!keep_key)
				delete now->key;
			delete[] now->value;
			delete now;
		}
		delete counter->list;
	}
	namespace test {
		void simple_test(){
			std::wstring* pkeys[27];
			WordCounter<std::wstring> counter;
			const wchar_t* keys[] = {
				L"hello", L"my", L"darling", L"friend.", // 4
				L"would", L"you", L"like", L"to", L"hang", // 9
				L"out", L"a", L"bit", L"more?", //13
				L"would", L"you", L"enjoy", L"to", L"be", L"out", //19
				L"a", L"bit", L"more?", L"even", L"if", L"take", L"one", // 26
				L"hour?"
			};
			for(size_t i = 0; i < 27; i++)
				pkeys[i] = new std::wstring(keys[i]);
			const bool insertions_result[] = {
				false, false, false, false,
				false, false, false, false, false,
				false, false, false, false, 
				true, true, false, true, false, true, 
				true, true, true, false, false, false, false, 
				false
			};
			const wchar_t* result_ordering[] = {
				L"a", L"be", L"bit", L"darling", 
				L"enjoy", L"even", L"friend.",  L"hang", 
				L"hello", L"hour?", L"if", L"like", 
				L"more?",L"my", L"one", L"out", 
				L"take", L"to", L"would", L"you"
			};
			const size_t result_counter[20][2] = {
				{1, 1}, {0, 1}, {1, 1}, {1, 0},
				{0, 1}, {0, 1}, {1, 0}, {1, 0},
				{1, 0}, {0, 1}, {0, 1}, {1, 0},
				{0, 2}, {1, 0}, {0, 1}, {1, 1},
				{0, 1}, {1, 1}, {1, 1}, {1, 1},
			};
			#ifdef __VERBOSE_TESTS__
			std::wcout << "========== text ==========" << std::endl;
			for(size_t i = 0; i < 12; i++)
				std::wcout << *pkeys[i] << " ";
			std::wcout << std::endl << "========== text ==========" << std::endl;
			for(size_t i = 12; i < 27; i++)
				std::wcout << *pkeys[i] << " ";
			std::wcout << std::endl << "========== text ==========" << std::endl;
			#endif
			
			initialize(&counter, 2);
			for(size_t i = 0; i < 12; i++)
				assert(insert_word(&counter, pkeys[i]) == insertions_result[i]);
			next_source(&counter);
			for(size_t i = 12; i < 27; i++)
				assert(insert_word(&counter, pkeys[i]) == insertions_result[i]);
			next_source(&counter);
			
			size_t position = 0;
			NodeIterator::NodeIterator<std::wstring, size_t> it_state;
			NodeIterator::create(&it_state, counter.list->first);
			while(NodeIterator::isalive(&it_state)) {
				LLDE::Node<std::wstring, size_t>* now = NodeIterator::next(&it_state);
				#ifdef __VERBOSE_TESTS__
				print_cnode(now, counter.src_id, std::wcout);
				#endif
				assert(*now->key == std::wstring(result_ordering[position]));
				for(size_t i = 0, *arr = now->value; i < 2; i++)
					assert(arr[i] == result_counter[position][i]);
				position++;
			}
			
			destroy_counter<std::wstring, true>(&counter);
			
			for(size_t i = 0; i < 27; i++)
				delete pkeys[i];
		}
		void test_destructor(){
			std::wstring* pkeys[27];
			WordCounter<std::wstring> counter;
			const wchar_t* keys[] = {
				L"Hello", L"my", L"darling", L"friend.", // 4
				L"would", L"you", L"like", L"to", L"hang", // 9
				L"out", L"a", L"bit", L"more?", //13
				L"would", L"you", L"enjoy", L"to", L"be", L"out", //19
				L"a", L"bit", L"more?", L"even", L"if", L"take", L"one", // 26
				L"hour?"
			};
			
			for(size_t i = 0; i < 27; i++)
				pkeys[i] = new std::wstring(keys[i]);
			
			initialize(&counter, 2);
			for(size_t i = 0; i < 12; i++)
				insert_or_dealloc(&counter, pkeys[i]);
			next_source(&counter);
			for(size_t i = 12; i < 27; i++)
				insert_or_dealloc(&counter, pkeys[i]);
			
			destroy_counter(&counter);
		}
	}
}
namespace stateview {
	template<typename word_t>
	void system_pause(word_t message)
	{
		std::wstring dummy;
		std::wcout << message; // print pause message
		std::wcin.ignore(1, '\n'); // ignore previuos new-line in stream 
		std::getline(std::wcin, dummy); // await to user press enter and discard input
	}
	void system_pause() {
		system_pause("Press enter to continue...");
	}
	// increase used memory
	void use_memory(size_t size) {
		if(size > 0) {
			uint8_t* tmp = new uint8_t[size];
			for(size_t i = 0; i < size; i++)
				tmp[i] = i;
			system_pause();
			delete[] tmp;
		}
	}
}
void sanity_tests(){
	ordened_linked_map::test::test_string_comparation();
	ordened_linked_map::test::test_edge_insertion(true);
	ordened_linked_map::test::test_check_edge();
	ordened_linked_map::test::test_psearch_int();
	ordened_linked_map::test::test_psearch_wstring();
	ordened_linked_map::test::test_foc_int();
	ordened_linked_map::test::test_foc_wstring();
	word_counter::test::simple_test();
	word_counter::test::test_destructor();
	word_parse::test::simple_test();
}
void present_testname(const char* testname) {
	std::wcout << "--------------------------------------";
	std::wcout << " TEST ";
	std::wcout << "--------------------------------------" << std::endl;
	std::wcout << "         " << testname << std::endl;
	std::wcout << "--------------------------------------";
	std::wcout << " TEST ";
	std::wcout << "--------------------------------------" << std::endl;
}
void interactive_tests() {
	const char* filenames[3] = { 
		"test1.txt", "test2.txt", "invalid.txt"
	};
	present_testname("[LINE_READER] @test<FILE*>");
	line_reader::test::test<FILE*>(filenames, 3);
	present_testname("[LINE_READER] @test<wifstream*>");
	line_reader::test::test<std::wifstream*>(filenames, 3);
	present_testname("[WORD_PARSE] @echo<wchar_t>");
	word_parse::test::echo_test<wchar_t>();
}
/*
	Converts a C wide null-terminated string to lower case.
	[Warnings]:
		(1) The source shall be null-terminated
		(2) The source may be modified
*/
inline void towlowerstr(wchar_t* source) {
	for(size_t i = 0, size = wcslen(source); i < size; i++)
		source[i] = towlower(source[i]);
}
inline void towlowerstr(std::wstring& source) {
	for(size_t i = 0, size = source.size(); i < size; i++)
		source[i] = towlower(source[i]);
}

namespace project {
	namespace LR = line_reader;
	namespace WC = word_counter;
	namespace WP = word_parse;
	using IO = std::ios_base;
	using counter_t = WC::WordCounter<std::wstring>;
	using parse_t = WP::wparse<wchar_t>;
	// prototypes
	// read a raw word into buffer, returns true on sucess or at EOF. Otherwise returns false. Set ended with a bolean value indicating if reached EOF.
	template<typename source_t>
		bool read_rawword(source_t source, std::wstring* buffer, bool* ended);
	template<typename source_t>
	void process_file(counter_t* counter, source_t source);
	template<typename source_t>
		void entry_point(const char** names, const size_t amount);
	// entry point
	template<typename source_t>
	void entry_point(const char** names, size_t amount){
		LR::LineReader<source_t> state; // the state machine
		counter_t counter;
		WC::initialize(&counter, amount);
		for(LR::create(&state, names, amount); LR::isgood(&state); LR::next_file(&state)) {
			process_file(&counter, state.source);
			WC::next_source(&counter);
		}
			
		LR::close_file(&state); // close the underlaying file, if it is opened.
		if(LR::isalive(&state)) { // iteration is poisoned and is alive
			std::wcout << L"Entrada inválida!" << std::endl;
		} else { // sucess
			#ifndef __PIPE_TO_STDOUT__
				IO::openmode mode = IO::out | IO::trunc;
				std::wfstream output = std::wfstream("resultado.out", mode);
				WC::print_counter(&counter, output);
			#else
				WC::print_counter(&counter, std::wcout);
			#endif
		}
		WC::destroy_counter(&counter);
	}
	template<>
	bool read_rawword<std::wifstream*>(std::wifstream* source, std::wstring* buffer, bool* ended) {
		*source >> *buffer;
		*ended = source->eof();
		return source->good() || *ended;
	}
	
	template<typename source_t>
	void process_file(counter_t* counter, source_t source) {
		std::wstring buffer;
		parse_t parse;
		WP::initialize(&parse);
		bool ended = false;
		while(read_rawword(source, &buffer, &ended)) {
			buffer.push_back(' '); // sentinel
			WP::feed(&parse, buffer); // insert a raw word (with bad characters)
			while(WP::is_not_empty(&parse)) {
				WP::ignore(&parse); // ignore bad character from state machine
				std::wstring* word = WP::read_word(&parse, nullptr); // take one word (without bad characters)
				if (word == nullptr)
					break;
				towlowerstr(*word); // coverts to lowercase
				//std::wcout << *word << std::endl; // print the word
				WC::insert_or_dealloc(counter, word);
			}
			if(ended)
				break;
		}
		WP::destroy_state(&parse);
	}
	namespace tests {
		void invalid_input(){
			const char* filenames[4] = { 
				"test1.txt", "test2.txt", "test3.txt", "invalid.txt"
			};
			entry_point<std::wifstream*>(filenames, 4);
		}
		void simplest() {
			const char* filenames[1] = {"hello_world.txt"};
			entry_point<std::wifstream*>(filenames, 1);
		}
	}
}
// delegate the call to the main handler
void proxy_call(const int argc, const char** argv) {
	if (argc < 3)
		return;
	int raw_amount = atoi(argv[1]);
	if(raw_amount < 0)
		return;
	size_t amount = (size_t) raw_amount;
	size_t provided = (size_t) (argc - 2);
	amount = amount < provided ? amount : provided;
	const char** filenames = argv + 2;
	/*std::wcout << "amount: " << amount << std::endl;
	for(size_t i = 0; i < amount; i++)
		std::wcout << filenames[i] << std::endl;
	for(size_t i = 0; i < 27; i++)
		std::wcout << L'-';
	std::wcout << std::endl;*/
	project::entry_point<std::wifstream*>(filenames, amount);
}
int main(int argc, char** argv) {
	std::ios_base::sync_with_stdio(false);
	std::locale::global (std::locale (""));
	std::wcin.imbue( std::locale("") );
	std::wcout.imbue( std::locale("") );
	//setlocale(LC_ALL, "");
	std::wcout << L"";
	#ifdef __SANITY_TESTS__
		sanity_tests();
	#endif
	proxy_call(argc, (const char**)argv);
	#ifdef __INTERACTIVE_TESTS__
		interactive_tests();
	#endif
	//project::tests::simplest();
	
	return 0;
}
/*
The paradigm used was processual programming with a bit of generic programming. You probabilly will need a bit of time to scroll up to all boilerplate. There's five state machines and greater part of code is reserved for testing. If the probabilly of single failure per line is 0.1%, then it may have at least one failure. At least 40% of lines are pretty document, and is expected to have more than 10% of that are outdated. There's at least five state machine:
 - LineReader: abstracts the command line argument as interator .
 - OrdenedLinkedMap: a ordened double linked list with key-value fields.
 - WordCounter: counts word per file using the OrdenedLinkedMap
 - WordParse: a buffered tokenizer
*/