#include <stdlib.h>
#include <string.h>
#include "Tree.h"

// debug messages
static const char ALLOCATE[] = " - Allocating]\n";
static const char COST_READ[] = "[Cost Increment (Disk Access): Reading ";
static const char COST_WRITE[] = "[Cost Increment (Disk Access): Writing ";
static const char DEALLOCATE[] = " - Deallocating]\n";
static const char TREE[] = "[Tree ";

template <class Whatever>
int Tree<Whatever>::debug_on = 0;

template <class Whatever> 
long Tree<Whatever>::cost = 0;

template <class Whatever>
long Tree<Whatever>::operation = 0;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define THRESHOLD 2
#define ROOT_PLACE 8

template <class Whatever>
ostream & operator << (ostream &, const TNode<Whatever> &);

template <class Whatever>
struct  TNode {
// friends:

// data fields:
	Whatever data;
	long height;
	long balance;
	offset left;
	offset right;
	offset this_position;	// current position

// function fields:
	TNode () : height (0), balance (0), left (0), right (0), 
		this_position (0) {}

	// to declare the working TNode in Tree's Remove
	TNode (Whatever & element) : data (element), height (0), balance (0),
		left (0), right (0), this_position (0) {}
	
	TNode (Whatever &, fstream *, long &);	// to add new node to disk
	TNode (const offset &, fstream *);	// to read node from disk
	
	unsigned long Insert (Whatever &, fstream *, long &, offset &);
	// optional recursive Lookup declaration would go here
	void Read (const offset &, fstream *);	// read node from disk
	unsigned long Remove (TNode<Whatever> &, fstream *, long &, offset &,
		long fromSHB = FALSE);
	void ReplaceAndRemoveMax (TNode<Whatever> &, fstream *, offset &);
	void SetHeightAndBalance (fstream *, offset &);
	void Write (fstream *) const;		// update node to disk

	ostream & Write_AllTNodes (ostream &, fstream *) const;
};

/* 
* Name: Tree Insert
* Purpose: Insert the root or enter the tree to find entry
* Description:
        If the first entry, just make a node to start the tree. Otherwise, use
        root to enter the tree and search for the correct place. TNode's insert
        will handle insertion.
* Parameters:
        Whatever & element - info to store as data
* Return: TRUE/1 for true, FALSE/0 for false
* Result : yay new node
*/
template <class Whatever>
unsigned long Tree<Whatever> :: Insert (Whatever & element) {
  long retval;

  fio->seekp(0, ios::end);
  offset ending = fio->tellp(); // get ending pointer

  // insert root
  if (ending == ROOT_PLACE) { 
    TNode<Whatever> temp(element, fio, occupancy); // write ctor
    
    
    root = temp.this_position;

    return 1;

  // launch search for every other entry
  } else {
    TNode<Whatever> readRootNode(root, fio);
    retval = readRootNode.Insert(element, fio, occupancy, root);
    return retval;
  }

}

template <class Whatever>
void TNode<Whatever> :: ReplaceAndRemoveMax (TNode<Whatever> & targetTNode, 
	fstream * fio, offset & PositionInParent) {
	/* YOUR CODE GOES HERE */
}

template <class Whatever>
unsigned long TNode<Whatever> :: Remove (TNode<Whatever> & elementTNode,
	fstream * fio, long & occupancy, offset & PositionInParent,
	long fromSHB) {
	/* YOUR CODE GOES HERE */
	return 0;
}
	
template <class Whatever>
unsigned long Tree<Whatever> :: Remove (Whatever & element) {
	/* YOUR CODE GOES HERE */
	return 1;
}

template <class Whatever>
void TNode<Whatever> :: SetHeightAndBalance (fstream * fio,
	offset & PositionInParent) {
	/* YOUR CODE GOES HERE */
}

template <class Whatever>
long Tree <Whatever> :: GetCost () {
	return cost;
}

template <class Whatever>
long Tree <Whatever> :: GetOperation () {
  return operation;
}

template <class Whatever>
void Tree <Whatever> :: IncrementCost () {
	cost++;
}

template <class Whatever>
void Tree <Whatever> :: IncrementOperation () {
	operation++;
}

template <class Whatever>
void Tree <Whatever> :: ResetRoot () {
        /* YOUR CODE GOES HERE */       
}
	

template <class Whatever>
unsigned long TNode<Whatever> :: Insert (Whatever & element, fstream * fio,
	long & occupancy, offset & PositionInParent) {

	return 0;
}

template <class Whatever>
unsigned long Tree<Whatever> :: Lookup (Whatever & element) const {
	/* YOUR CODE GOES HERE */
        return 0;
}

template <class Whatever>
void TNode<Whatever> :: Read (const offset & position, fstream * fio) {
  


	fio -> seekp (position);
  fio -> read ((char *) this, sizeof(TNode<Whatever>));

  if (Tree<Whatever> :: debug_on) {
    cerr << COST_READ << (const char *)data << "]\n";
  } 
}

template <class Whatever>
TNode<Whatever> :: TNode (const offset & position, fstream * fio) {
  Read (position, fio);
}

// WRITE CONSTRUCTOR OVER HERE !!!!!!!
template <class Whatever>
TNode<Whatever> :: TNode (Whatever & element, fstream * fio, long & occupancy): 
			data (element), height (0), balance (0), left (0), 
			right (0) {
  fio -> seekp(0, ios :: end);
  this_position = fio -> tellp();
	occupancy++;
  Write(fio);
}

template <class Whatever>
void TNode<Whatever> :: Write (fstream * fio) const {

  if (Tree<Whatever> :: debug_on) {
    cerr << COST_WRITE << (const char *)data << "]\n";
  }

  
  fio -> seekp(0, ios::end); // put pointer at end
  //offset end = fio -> tellp(); //seems to be useless code
  fio -> write((const char *) this, sizeof(TNode<Whatever>));

}


template <class Whatever>
Tree<Whatever> :: Tree (const char * datafile) :
	fio (new fstream (datafile, ios :: out | ios :: in)) {
  tree_count = 1;
  occupancy = 0;
  root = ROOT_PLACE;

  // debug check
  if (debug_on) {
    cerr << TREE << Tree<Whatever> :: tree_count << ALLOCATE;
  }

  fio -> seekg(0, ios::beg);
  offset beginning = fio->tellg(); // get beginning pointer

  fio->seekg(0, ios::end);
  offset ending = fio->tellg(); // get ending pointer

  if (beginning == ending) { // file is empty, make root
    fio -> seekp(0, ios :: beg);
    fio -> write((const char *) &root, sizeof(root));
    fio -> write ((const char *) &occupancy, sizeof(occupancy));
    
    root = fio->tellp();

  } else {
    fio -> seekg(0, ios :: beg);
    fio -> read((char *) &root, sizeof(root));
    fio -> read((char *) &occupancy, sizeof(occupancy));
  }
}

template <class Whatever>
Tree<Whatever> :: ~Tree (void)
/***************************************************************************
% Routine Name : Tree :: ~Tree  (public)
% File :         Tree.c
% 
% Description :  deallocates memory associated with the Tree.  It
%                will also delete all the memory of the elements within
%                the table.
***************************************************************************/

{

    // debug check
  if (debug_on) {
    cerr << TREE << Tree<Whatever> :: tree_count << DEALLOCATE;
  }

  //fio -> write((const char *) & occupancy, sizeof(occupancy));
  root = 0;
  occupancy = 0;
  delete fio;
}	/* end: ~Tree */

template <class Whatever>
void Tree<Whatever> :: Set_Debug_On(void) {
  debug_on = 1;
}


template <class Whatever>
void Tree<Whatever> :: Set_Debug_Off(void) {
  debug_on = 0;
}

template <class Whatever>
ostream & operator << (ostream & stream, const TNode<Whatever> & nnn) {
	stream << "at height:  :" << nnn.height << " with balance:  "
		<< nnn.balance << "  ";
	return stream << nnn.data << "\n";
}

template <class Whatever>
ostream & Tree<Whatever> :: Write (ostream & stream) const
/***************************************************************************
% Routine Name : Tree :: Write (public)
% File :         Tree.c
% 
% Description : This funtion will output the contents of the Tree table
%               to the stream specificed by the caller.  The stream could be
%               cerr, cout, or any other valid stream.
%
% Parameters descriptions :
% 
% name               description
% ------------------ ------------------------------------------------------
% stream             A reference to the output stream.
% <return>           A reference to the output stream.
***************************************************************************/

{
        long old_cost = cost;

	stream << "Tree " << tree_count << ":\n"
		<< "occupancy is " << occupancy << " elements.\n";

	fio->seekg (0, ios :: end);
	offset end = fio->tellg ();

	// check for new file
	if (root != end) {
		TNode<Whatever> readRootNode (root, fio);
		readRootNode.Write_AllTNodes (stream, fio);
	}

        // ignore cost when displaying nodes to users
        cost = old_cost;

	return stream;
}

template <class Whatever>
ostream & TNode<Whatever> ::
Write_AllTNodes (ostream & stream, fstream * fio) const {
	if (left) {
		TNode<Whatever> readLeftNode (left, fio);
		readLeftNode.Write_AllTNodes (stream, fio);
	}
	stream << *this;
	if (right) {
		TNode<Whatever> readRightNode (right, fio);
		readRightNode.Write_AllTNodes (stream, fio);
	}

	return stream;
}

