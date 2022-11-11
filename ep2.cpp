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
		return left->compare(*right);
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
		template<typename key_t, typename value_t>
		EdgeStatus check_edgenode(const OrdenedLinkedMap<key_t, value_t>* list) {
			if(list->first->prev != nullptr) // error in left edge node
				return EdgeStatus::edge_leftnode_error;
			if(list->last->next != nullptr) // error in right edge node
				return EdgeStatus::edge_rightnode_error;
			return EdgeStatus::edge_nullability; // sucess
		}
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
		template<typename key_t, typename value_t>
		EdgeStatus check_edges(const OrdenedLinkedMap<key_t, value_t>* list) {
			EdgeStatus status = check_edgelink(list);
			if(status != EdgeStatus::unknown_edgestatus)
				return status;
			return check_edgenode(list);
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
namespace word_counter {
	namespace LLDE = ordened_linked_map;
	namespace NodeIterator = LLDE::NodeIterator;
	template<typename word_t>
	struct WordCounter {
		LLDE::OrdenedLinkedMap<word_t, size_t*>* list;
		size_t src_id;
		size_t amount;
	};
	template<typename word_t>
	void initialize(WordCounter<word_t>* state, LLDE::cmp_fn<word_t> compare, size_t amount) {
		state->list = new LLDE::OrdenedLinkedMap<word_t, size_t*>;
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
		LLDE::Node<word_t, size_t*>* element = LLDE::find_or_create(state->list, word, &found);
		if(!found)
			element->value = new size_t[state->amount];
		element->value[state->src_id]++;
		return found;
	}
	template<typename word_t>
	void insert_or_dealloc(WordCounter<word_t>* state, word_t* word) {
		if(insert_word(state, word))
			delete word;
	}
	namespace test {
		void simple_test(){
			std::wstring* words[26];
			WordCounter<std::wstring> counter;
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
				std::wcout << "word: "  << *words[i] << std::endl;
			/*for(size_t i = 0; i < 12; i++)
				insert_word(&counter, sources + i);
			next_source(&counter);
			for(size_t i = 12; i < 26; i++)
				insert_word(&counter, sources + i);*/
			
		}
		/*void debug_wstring(WordCounter* counter){
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
		}*/
	}
}
void tests(){
	const char* filenames[4];
	std::wcout << L"";
	filenames[0] = "test1.txt";
	filenames[1] = "test2.txt";
	filenames[2] = "test3.txt";
	filenames[3] = "test4.txt";
	LineReaderFile::test::test(filenames, 3);
	ordened_linked_map::test::test_string_comparation();
	ordened_linked_map::test::test_edge_insertion(true);
	ordened_linked_map::test::test_check_edge();
	ordened_linked_map::test::test_psearch_int();
	ordened_linked_map::test::test_foc_int();
	ordened_linked_map::test::test_psearch_wstring();
	ordened_linked_map::test::test_foc_wstring();
	//word_counter::test::simple_test();
}
int main() {
	//std::locale::global (std::locale (""));
	setlocale(LC_ALL, "");
	tests();
	return 0;
}
