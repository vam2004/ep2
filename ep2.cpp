#include <iostream>
#include <locale>
#include <clocale>
#include <stdio.h>
#include <assert.h>
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
				wchar_t tmp;
				while((tmp = getwc(state.source)) != WEOF) 
					std::wcout << tmp;
			} // the iteration is not alive or is poisoned
			std::wcout << std::endl << "@=========@ end @=========@" << std::endl;
			close_file(&state); // close the underlaying file, if it is opened.
			if(!check_state(&state))  // iteration is poisoned and is alive
				std::wcout << "Invalid File: " << get_filename(&state) << std::endl << std::endl;
		}
		/*
		Prints the name of actual file.
		[warnings]:
			(1) The "state" shall be alive (the iteration wasn't ended)
		*/
		void printFilename(const LineReader* state) {
			std::wcout << L"========" << std::flush;
			std::wcout << get_filename(state) << std::flush; 
			std::wcout << L"========" << std::endl;
		}
	}
}
namespace ordened_linked_map {
	/*
	expeted a function that returns:
		-1, if the left element is lesser than right element
		0, if the left element is equal to right element
		1, if the left element is greater than right
	*/
	using cmp_fn = int (*)(const void*, const void*);
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
	struct Node {
		void* key; // the value used for referencing 
		void* value; // the value begin referencied
		Node* prev; // previuos (left) node in the ordened linked map
		Node* next; // next (right) node in the ordened linked map
		#ifdef LINKMARK
		int linkmark; // used for validating
		#endif
	};
	/*
	The entry point of linked ordened map
	*/
	struct OrdenedLinkedMap {
		Node* first; // the first node (left egde) of the ordened linked map
		Node* last; // the last node (right edge) of the ordened linked map
		cmp_fn compare; // trampoline: the function used for comparing
	};
	void initalize_empty(OrdenedLinkedMap* list, cmp_fn compare) {
		list->first = nullptr;
		list->last = nullptr;
		list->compare = compare;
	}
	namespace NodeIterator {
		struct NodeIterator {
			Node* now;
			bool reverse;
		};
		// main prototypes
		void rewind(NodeIterator* state, const OrdenedLinkedMap* list);
		bool isalive(const NodeIterator* state);
		bool isreverse(const NodeIterator* state);
		void seekend(NodeIterator* state, const OrdenedLinkedMap* list);
		Node* next(NodeIterator* state);
		Node* back(NodeIterator* state);
		
		NodeIterator* create(NodeIterator* state, Node* now) {
			state->reverse = false;
			state->now = now;
			return state;
		}
		NodeIterator* createReverse(NodeIterator* state, Node* now) {
			state->reverse = true;
			state->now = now;
			return state;
		}
		bool isreverse(const NodeIterator* state) {
			return state->reverse;
		}
		bool isalive(const NodeIterator* state) {
			return state->now != nullptr;
		}
		void rewind(NodeIterator* state, const OrdenedLinkedMap* list) {
			state->now = state->reverse ? list->last : list->first;
		}
		void seekend(NodeIterator* state, const OrdenedLinkedMap* list) {
			state->now = state->reverse ? list->first : list->last;
		}
		Node* next(NodeIterator* state) {
			Node* now = state->now;
			state->now = state->reverse ? now->prev : now->next;
			return now;
		}
		Node* back(NodeIterator* state) {
			Node* now = state->now;
			state->now = state->reverse ? now->next : now->prev;
			return now;
		}
	}
	/*
	The result of partialSearch().
	*/
	struct SearchInterval {
		Node* lt; // the node that is lesser than the target (nullptr if not exists)
		Node* eq; // the node that is equal to the target (nullptr if not exists)
		Node* gt; // the node that is great than the target (nullptr if not exists)
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
	void insertLeft(OrdenedLinkedMap* list, Node* element, Node* position) {
		Node* previuos = position->prev; // copy the reference to left node of "position"
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
	void insertRight(OrdenedLinkedMap* list, Node* element, Node* position) {
		Node* next = position->next; // copy the reference to left node of "position"
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
	void appendLeft(OrdenedLinkedMap* list, Node* element) {
		Node* first = list->first;
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
	void appendRight(OrdenedLinkedMap* list, Node* element) {
		Node* last = list->last;
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
	void insertBetween(OrdenedLinkedMap* list, Node* element, Node* left, Node* right) {
		//assert(left != nullptr && left->next == right); // as said in warning (1)
		//assert(right != nullptr && right->prev == left); // as said in warning (2)
		if(left == nullptr) { // right is the left edge of ordened linked map
			appendLeft(list, element); // insert at begining
		} else { // right is not the left edge of ordened linked map
			insertRight(list, element, left); // insert at right side of "left" (and therefore at left side of "right")
		}
	}
	void clear_search_interval(SearchInterval* into) {
		into->lt = nullptr;
		into->eq = nullptr;
		into->gt = nullptr;
	}
	SearchInterval* partial_find(SearchInterval* state, OrdenedLinkedMap* list, void* key) {
		/*
		The target is the node the first node that key field is equal to "key"
		A candidate is a node that could be the target. Exists up to one candidate, because the key fields's value of all nodes after the first candidate are greater than key, and, therefore, all subsequent nodes aren't candidates.
		*/
		clear_search_interval(state);
		cmp_fn compare = list->compare; // store the function to compare in the stack
		Node* pre = nullptr; // If exists at least one element in the list, then it will be pointer to last element that is lesser than key. Otherwise, it will remain as nullptr
		Node* now = list->first; // A candidate, if exists. Otherwise, it is nullptr (including the case of the list begin empty, that is, when list->first is nullptr). 
		int cmp = 0; // Comparation flag. Non-zero means that the candidate was found. 
		while(now != nullptr) {
			/*
			compare(now->key, key) + 1 == 0 --> now->key < key
			compare(now->key, key) + 1 == 1 --> now->key == key
			compare(now->key, key) + 1 == 2 --> now->key > key
			*/
			cmp = compare(now->key, key) + 1; // update the comparation flag
			if(cmp != 0) // now->key < key
				break;
			// now->key < key
			pre = now; // update the last node
			now = now->next; // advance to next node
		}
		state->lt = pre; // The variable "pre" contains the last node that is lesser than the target, or nullptr if not exists.
		// "Now" will be nullptr if, and only if, "cmp" is not equal to zero.
		if(cmp == 0) // there isn't a node is greater or equal to target.
			return state;
		// "now" is the candidate
		if(cmp == 2) { // target was not found
			state->gt = now; // the candidate is the first node that key is greater than target
		} else { // target was found
			state->eq = now; // the candidate is the target
			state->gt = now->next; // the greater neightbor is the first node that key is greater than target
		}
		return state;
	}
	/*
	return a boolean value representing if the ordened linked map was empty when the partial_find() was done. This could depends of how partial_find() was implemented, therefore should be always used for this situation.
	*/
	bool intv_empty(SearchInterval* source){
		return source->eq == nullptr && source->lt == nullptr && source->gt == nullptr;
	}
	bool intv_found(SearchInterval* source) {
		return source->eq != nullptr;
	}
	// the cmp_fn implementation for type "int"*((int*) left);
	int compare_int(const void* left, const void* right) {
		int ileft = *((int*) left);
		int iright = *((int*) right); 
		if(ileft < iright) 
			return -1;
		if(ileft == iright)
			return 0;
		return 1;
	}
	// the cmp_fn implementation for type "std::string"
	int compare_string(const void* left, const void* right) {
		std::string sleft = *((std::string*) left);
		std::string sright = *((std::string*) right);
		return sleft.compare(sright);
	}
	// the cmp_fn implementation for type "std::wstring"
	int compare_wstring(const void* left, const void* right) {
		std::wstring sleft = *((std::wstring*) left);
		std::wstring sright = *((std::wstring*) right);
		return sleft.compare(sright);
	}
	// initialize the node with defaults values 
	void init_node(Node* into, void* key, void* value) {
		into->key = key; // assign the key
		into->value = value; // assign the value
		into->next = nullptr; // null-initialization
		into->prev = nullptr; // null-initialization
		#ifdef LINKMARK
		into->linkmark = CREATION_MARK;
		#endif
	}
	// create a node in the heap and then initialize with defaults value
	Node* create_node(void* key, void* value) {
		Node* tmp = new Node; // create a node in heap 
		init_node(tmp, key, value); // initialize with defaults value
		return tmp; // returns the node
	}
	/*
	Atomically returns a node that contains the key, if exists; or create and returns a new node, if not exists.
	[Warnings]:
		(1) If not exists a node that contains the key, then will be created a new node that has the field value as nullptr. Make sure to properly initializate or handle the nullptr in value
	*/
	Node* find_or_create(OrdenedLinkedMap* list, void* key, bool* found) {
		SearchInterval state; // create a temporary state machine
		*found = intv_found(partial_find(&state, list, key));
		if(*found) { // the key was found
			return state.eq; // return the node that contains the key
		}
		Node* element = create_node(key, nullptr);	// as said in warning (1)
		if(intv_empty(&state)) { // the list is empty
			appendLeft(list, element); // insert at begining
		} else {
			insertBetween(list, element, state.lt, state.gt); // insert between the node last node that is lesser than target; and the first node that is greater than target
		}
		return element; // return the element which is properly poisitioned, but partially initialized
	}
	/* Check if both edge are valid:
		- The left edge shall points to nullptr as left node (edge node nullability)
		- The right edge shall points to nullptr as right node (edge node nullability)
		- The left edge points to nullptr as right node if, and only if, the right edge points to nullptr (Double edge link nullability)
		Returns non-zero on error. Otherwise, returns zero.
	*/
	namespace check_edge {
		enum EdgeStatus {
			edge_nullability,
			edge_leftlink_error,
			edge_rightlink_error,
			edge_leftnode_error,
			edge_rightnode_error,
			unknown_edgestatus 
		};
		EdgeStatus check_edgenode(const OrdenedLinkedMap* list) {
			if(list->first->prev != nullptr) // error in left edge node
				return EdgeStatus::edge_leftnode_error;
			if(list->last->next != nullptr) // error in right edge node
				return EdgeStatus::edge_rightnode_error;
			return EdgeStatus::edge_nullability; // sucess
		}
		EdgeStatus check_edgelink(const OrdenedLinkedMap* list) {
			if(list->first == nullptr) {
				if(list->last == nullptr) // both edges are nullptr (sucess)
					return EdgeStatus::edge_nullability;
				return EdgeStatus::edge_rightlink_error; // the right edge shall be nullptr (error) 
			}  // the left edge is not nullptr
			if(list->last == nullptr) // the right edge is nullptr (error)
				return EdgeStatus::edge_leftlink_error; // the left edge shall be nullptr
			return EdgeStatus::unknown_edgestatus; // the edge links are sane. But the edge nodes status still not covered
		}
		EdgeStatus check_edges(const OrdenedLinkedMap* list) {
			EdgeStatus status = check_edgelink(list);
			if(status != EdgeStatus::unknown_edgestatus)
				return status;
			return check_edgenode(list);
		}
	}
	
	namespace hard_linkcheck {
		using NodeIterator::isalive;
		size_t get_omega(const Node* element, const Node** nodes, size_t amount) {
			size_t pos = 0;
			while(pos < amount && element != nodes[pos]) 
				pos++;
			return pos;
		}
		Node* check_unordered (NodeIterator::NodeIterator* state, const Node** nodes, size_t amount) {
			while(isalive(state)) {
				Node* now = state->now;
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
		Node* check_positioned(NodeIterator::NodeIterator* state, const Node** nodes, size_t amount) {
			for(size_t i = 0; i < amount && isalive(state); i++) {
				if(nodes[i] != state->now)
					return state->now;
				NodeIterator::next(state);
			}
			return nullptr;
		}
		Node* check_ordered(NodeIterator::NodeIterator* state, cmp_fn compare) {
			Node* prev = nullptr;
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
		bool check_crosslink_of(const Node* element) {
				Node* prev = element->prev;
				Node* next = element->next;
					if(next != nullptr && next->prev != element)
					return false;
				if(prev != nullptr && prev->next != element)
					return false;
				return true;
			}
		Node* check_crosslink(NodeIterator::NodeIterator* state) {
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
		void printnode_int(const Node* src);
		void printmap_int(NodeIterator::NodeIterator* state);
		void test_check_edge(){
			Node* nodes [3];
			int keys[] = {11, 12, 14};
			int values[] = {-1, 7, 21};
			for(size_t i = 0; i < 3; i++)
				nodes[i] = create_node(keys + i, values + i);
			OrdenedLinkedMap list = {nullptr, nullptr, compare_int};
			
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
		}
		
		void test_edge_insertion(bool debug) {
			const size_t left_nodes = 6;
			const size_t right_nodes = 6;
			const size_t nodes_amount = left_nodes + right_nodes;
			int left_keys[] = {10, 15, 16, 97, -113, 48};
			int left_values[] = {20, 82, -72, 37, 51, 45};
			int right_keys[] = {0, 80, 59, 68, 31, 56};
			int right_values[] = {0, 79, 53, 111, -7, 83};
			const Node* nodes[12];
			OrdenedLinkedMap list = {nullptr, nullptr, compare_int};
			NodeIterator::NodeIterator mapper;
			assert(check_edges(&list) == edge_nullability);
			for(size_t i = 0; i < left_nodes; i++) {
				Node* data = create_node(left_keys + i, left_values + i);
				nodes[(left_nodes - 1) - i] = data;
				appendLeft(&list, data);
				assert(check_edges(&list) == edge_nullability);
			}
			for(size_t i = 0; i < right_nodes; i++) {
				Node* data = create_node(right_keys + i, right_values + i);
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
		}
		
		void test_partial_search(){
			Node* nodes [3];
			int keys[] = {-1, 27, 44};
			int values[] = {-1, 21, 7};
			for(size_t i = 0; i < 3; i++)
				nodes[i] = create_node(keys + i, values + i);
			OrdenedLinkedMap list = {nullptr, nullptr, compare_int};
			SearchInterval state;
			NodeIterator::NodeIterator mapper;
			assert(intv_empty(partial_find(&state, &list, keys)));
			
			appendLeft(&list, nodes[2]);
			
			partial_find(&state, &list, keys);
			assert(state.lt == nullptr);
			assert(state.eq == nullptr);
			assert(state.gt == nodes[2]);
			assert(!intv_empty(&state));
			
			partial_find(&state, &list, keys + 2);
			assert(state.lt == nullptr);
			assert(state.eq == nodes[2]);
			assert(state.gt == nullptr);
			assert(!intv_empty(&state));
			
			appendLeft(&list, nodes[0]);
			
			partial_find(&state, &list, keys);
			assert(state.lt == nullptr);
			assert(state.eq == nodes[0]);
			assert(state.gt == nodes[2]);
			assert(!intv_empty(&state));
			
			partial_find(&state, &list, keys + 1);
			assert(state.lt == nodes[0]);
			assert(state.eq == nullptr);
			assert(state.gt == nodes[2]);
			assert(!intv_empty(&state));
			
			partial_find(&state, &list, keys + 2);
			assert(state.lt == nodes[0]);
			assert(state.eq == nodes[2]);
			assert(state.gt == nullptr);
			assert(!intv_empty(&state));
			
			insertBetween(&list, nodes[1], nodes[0], nodes[2]);
			
			partial_find(&state, &list, keys);
			assert(state.lt == nullptr);
			assert(state.eq == nodes[0]);
			assert(state.gt == nodes[1]);
			assert(!intv_empty(&state));
			
			partial_find(&state, &list, keys + 1);
			assert(state.lt == nodes[0]);
			assert(state.eq == nodes[1]);
			assert(state.gt == nodes[2]);
			assert(!intv_empty(&state));
			
			partial_find(&state, &list, keys + 2);
			assert(state.lt == nodes[1]);
			assert(state.eq == nodes[2]);
			assert(state.gt == nullptr);
			assert(!intv_empty(&state));
		}
		
		
		void show_cmp_wstring(const std::wstring* left, const std::wstring* right){
			std::wcout << *left << L" ";
			std::wcout << get_cmp_symbol(compare_wstring(left, right));
			std::wcout << L" " << *right << std::endl;
		}
		void printnode_int(const Node* src){
			std::wcout << "key=" << *((int*) src->key) << " ";
			std::wcout << "adress=" << src << " ";
			#ifdef LINKMARK
			std::wcout << "linkmark= " << " ";
			#endif
			std::wcout << "value=" << *((int*) src->value) << std::endl;
		}
		
		void printmap_int(NodeIterator::NodeIterator* state) {
			while(NodeIterator::isalive(state))
				printnode_int(NodeIterator::next(state));
		}
	}
}
namespace word_counter {
	namespace LLDE = ordened_linked_map;
	namespace NodeIterator = LLDE::NodeIterator;
	struct WordCounter {
		LLDE::OrdenedLinkedMap* list;
		size_t src_id;
		size_t amount;
	};
	void initialize(WordCounter* state, LLDE::cmp_fn compare, size_t amount) {
		state->list = new LLDE::OrdenedLinkedMap;
		LLDE::initalize_empty(state->list, compare);
		state->amount = amount;
		state->src_id = 0;
	}
	void initialize_wstring(WordCounter* state, size_t amount) {
		initialize(state, LLDE::compare_wstring, amount);
	}
	void initialize_string(WordCounter* state, size_t amount) {
		initialize(state, LLDE::compare_string, amount);
	}
	void next_source(WordCounter* state) {
		state->src_id++;
	}
	bool isalive(WordCounter* state) {
		return state->src_id < state->amount;
	}
	bool insert_word(WordCounter* state, void* word) {
		bool found;
		LLDE::Node* element = LLDE::find_or_create(state->list, word, &found);
		if(!found)
			element->value = new size_t[state->amount];
		size_t* counter =  (size_t*) element->value;
		counter[state->src_id]++;
		return found;
	}
	bool insert_word(WordCounter* state, std::wstring* word) {
		return insert_word(state, (void*) word);
	}
	bool insert_word(WordCounter* state, std::string* word) {
		return insert_word(state, (void*) word);
	}
	void insert_or_dealloc(WordCounter* state, std::wstring* word) {
		if(insert_word(state, word))
			delete word;
	}
	void insert_or_dealloc(WordCounter* state, std::string* word) {
		if(insert_word(state, word))
			delete word;
	}
	namespace test {
		void simple_test(){
			std::wstring* words[26];
			WordCounter counter;
			const wchar_t* sources[] = {
				L"Hello", L"my", L"darling", L"friend.", // 4
				L"would", L"you", L"like", L"to", L"hang", // 9
				L"out", L"a", L"bit", L"more?", //13
				L"would", L"you", L"enjoy", L"to", L"be", L"out", //19
				L"a", L"bit", L"more?", L"even", L"if", L"take", L"one", // 26
				L"hour?"
			};
			for(size_t i = 0; i < 26; i++)
				words[i] = new std::wstring(sources[i]);
			for(size_t i = 0; i < 26; i++)
				std::wcout << *words[i] << std::endl;
			/*for(size_t i = 0; i < 12; i++)
				insert_word(&counter, sources + i);
			next_source(&counter);
			for(size_t i = 12; i < 26; i++)
				insert_word(&counter, sources + i);*/
			
		}
		void debug_wstring(WordCounter* counter){
			NodeIterator::NodeIterator state;
			NodeIterator::create(&state, counter->list->first);
			size_t amount = counter->amount;
			while(NodeIterator::isalive(&state)) {
				std::wcout << "key: " << *((std::wstring*) state.now->key) << " ";
				//std::wcout << "adress=" << state->now << " ";
				std::wcout << "value: ";
				size_t* value = (size_t*) state.now->value;
				for(size_t i = 0; i < amount; i++)
					std::wcout << " " << value[i];
				std::wcout << std::endl;
			}
		}
	}
}
void tests(){
	const char* filenames[4];
	std::wcout << L"";
	filenames[0] = "test1.txt";
	filenames[1] = "test2.txt";
	filenames[2] = "test3.txt";
	filenames[3] = "test4.txt";
	//LineReaderFile::test::test(filenames, 3);
	//OrdenedLinkedMap::test::test_string_comparation();
	//OrdenedLinkedMap::test::test_edge_insertion(true);
	//OrdenedLinkedMap::test::test_check_edge();
	//OrdenedLinkedMap::test::test_partial_search();
	word_counter::test::simple_test();
}
int main() {
	//std::locale::global (std::locale (""));
	setlocale(LC_ALL, "");
	tests();
	return 0;
}
