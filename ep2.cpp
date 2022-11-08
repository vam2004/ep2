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
	void insert_beetween(OrdenedLinkedMap* list, Node* element, Node* left, Node* right) {
		assert(left != nullptr && left->next == left); // as said in warning (1)
		assert(right != nullptr && right->prev == left); // as said in warning (2)
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
	// the cmp_fn implementation for type "int"*((int*) left);
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
		std::string sright = *((std::string*) right);
		return sleft.compare(sright);
	}
	// the cmp_fn implementation for type "std::wstring"
	int compare_wstring(const void* left, const void* right) {
		std::wstring sleft = *((std::wstring*) left);
		std::wstring sright = *((std::wstring*) right);
		return sleft.compare(sright);
	}
	Node* create_node(void* key, void* value) {
		Node* tmp = new Node;
		tmp->key = key;
		tmp->value = value;
		tmp->next = nullptr;
		tmp->prev = nullptr;
		#ifdef LINKMARK
		tmp->linkmark = CREATION_MARK;
		#endif
		return tmp;
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
			if(list->first->prev != nullptr)
				return EdgeStatus::edge_leftnode_error;
			if(list->last->next != nullptr)
				return EdgeStatus::edge_rightnode_error;
			return EdgeStatus::edge_nullability;
		}
		EdgeStatus check_edgelink(const OrdenedLinkedMap* list) {
			if(list->first == nullptr) {
				if(list->last == nullptr)
					return EdgeStatus::edge_nullability;
				return EdgeStatus::edge_rightlink_error;
			}
			if(list->last == nullptr)
				return EdgeStatus::edge_leftlink_error;
			return EdgeStatus::unknown_edgestatus;
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
	// checks for circular reference from end to begining. If exist returns the first node of circular reference. Otherwise, returns nullptr
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
		const char* get_cmp_symbol(int result);
		void show_comparation_wstring(const std::wstring* left, const std::wstring* right);
		void printnode(const Node* src);
		void printmap_left_int(const OrdenedLinkedMap* list);
		void printmap_right_int(const OrdenedLinkedMap* list);
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
				show_comparation_wstring(text[i], text[i+1]);
			}
			std::wcout << std::endl;
		}
		const char* get_cmp_symbol(int result) {
			if (result == -1)
				return "<";
			if (result == 0)
				return "=";
			return ">";
		}
		void show_comparation_wstring(const std::wstring* left, const std::wstring* right){
			std::wcout << *left << L" ";
			std::wcout << get_cmp_symbol(compare_wstring(left, right));
			std::wcout << L" " << *right << std::endl;
		}
		void printnode(const Node* src){
			std::wcout << "key=" << *((int*) src->key) << " ";
			std::wcout << "adress=" << src << " ";
			#ifdef LINKMARK
			std::wcout << "linkmark= " << " ";
			#endif
			std::wcout << "value=" << *((int*) src->value) << std::endl;
		}
		void printmap_int(NodeIterator::NodeIterator* state) {
			while(NodeIterator::isalive(state))
				printnode(NodeIterator::next(state));
		}
		void printmap_left_int(const OrdenedLinkedMap* list) {
			for(Node* now = list->first; now != nullptr; now = now->next) {
				printnode(now);
			}
		}
		void printmap_right_int(const OrdenedLinkedMap* list) {
			for(Node* now = list->last; now != nullptr; now = now->prev) {
				printnode(now);
			}
		}
	}
}
int main() {
	const char* filenames[4];
	//std::locale::global (std::locale (""));
	setlocale(LC_ALL, "");
	std::wcout << L"";
	filenames[0] = "test1.txt";
	filenames[1] = "test2.txt";
	filenames[2] = "test3.txt";
	filenames[3] = "test4.txt";
	LineReaderFile::test::test(filenames, 3);
	OrdenedLinkedMap::test::test_string_comparation();
	OrdenedLinkedMap::test::test_edge_insertion(true);
	OrdenedLinkedMap::test::test_check_edge();
	return 0;
}
