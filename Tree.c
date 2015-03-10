/*------------------------------------------------------------------------
						                                            Stazia Tronboll
                                                        cs12xpr
                                                        Kevin _____
                                                        ________
                                                        CSE 12, Winter 2015
                                                        9 March 2015
			                        Assignment 9
File: Driver.c

FILE DECSRIPTION
-------------------------------------------------------------------------*/

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

/*--------------------------------------------------------------------------
Name: struct TNode
Purpose:
Functions that we wrote/edited:
	Tree Insert - 


THE TREE AND TNODE FUNCTIONS ARE INTERMINGLING! HOW DO WE FORMAT THE HEADER?
--------------------------------------------------------------------------*/
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
Name: Tree Insert
Purpose: Insert the root or enter the tree to find entry
Description:
      If the first entry, just make a node to start the tree. Otherwise, use
      root to enter the tree and search for the correct place. TNode's insert
      will handle insertion.
Parameters:
      Whatever & element - info to store as data
Return: TRUE/1 for true, FALSE/0 for false
Result : yay new node
*/
template <class Whatever>
unsigned long Tree<Whatever> :: Insert (Whatever & element) {
  IncrementOperation();

  fio->seekp(0, ios::end);
  offset ending = fio->tellp(); // get ending pointer

  // insert root
  if (ending == root) {
    TNode<Whatever> temp (element, fio, occupancy); // write ctor  
    root = temp.this_position;

    return 1;

  // launch search for every other entry
  } else {
    TNode<Whatever> readRootNode (root, fio);
    return readRootNode.Insert(element, fio, occupancy, root);
  }

}

/*
Name: ReplaceAndRemoveMax
Purpose: Handle removal of a node with two children
Description:
	    Travel right as far as you can, and put the data there into the
	    targetNode, which has the data that's being removed. Fully remove the
    	rightmost node.
Parameters: 
      TNode<Whatever> & targetTNode - node being removed, has two kids
      fstream * fio - the filestream
      offset & PositionInParent - current position, obtained through parent
Return: void
Result: a node is removed, and tree structure is maintained
*/
template <class Whatever>
void TNode<Whatever> :: ReplaceAndRemoveMax (TNode<Whatever> & targetTNode, 
	fstream * fio, offset & PositionInParent) {
  // TNode<Whatever> thisNode(PositionInParent,fio);
  //if there is a right child, recursively call ReplaceRemoveMax on it
  if(right)
  {
    TNode<Whatever> RightNode (right, fio);
    RightNode.ReplaceAndRemoveMax(targetTNode, fio, right);
    if(data)
      SetHeightAndBalance(fio, PositionInParent);
  }
  /* otherwise, set targetTNode's data to this data, PointerInParent to left
    and delete this data. Print debug message if debug is on */
  else    
  {
    targetTNode.data = data;
    PositionInParent = left;
  }
}

/*
Name: TNode Remove
Purpose: remove a node
Description:
      Determines if the current offset holds the data being removed, is greater
      than the data being removed, or less than the data being removed. If
      greater or less, sends it to another speace in the tree to continue
      searching. If equal, reassigns PositionInParent appropriate to the number
      of children that the node being removed has. Writes the edits to disk,
      decrements occupancy.
Parameters: 
      TNode<Whatever> & elementTNode - node of the element being removed
      fstream * fio - the filestream
      occupancy - number of nodes in the tree
      offset & PositionInParent - current position, obtained through parent
      long fromSHB - did SHAB call remove?
Return: TRUE/1 for success, FALSE/0 for failure
Result: the data is removed from the tree
Side effects: tree's static occupancy adjusted, cost increment
*/
template <class Whatever>
unsigned long TNode<Whatever> :: Remove (TNode<Whatever> & elementTNode,
	fstream * fio, long & occupancy, offset & PositionInParent,
	long fromSHB) {
  TNode<Whatever> thisNode(PositionInParent,fio);

    // once the element is found, reassign pointers and delete, return TRUE
    if (elementTNode.data == data) { // found node to remove

      elementTNode.data = data; // reassign element to have the student's #
      
      // two children removal
      if (left != 0 && right != 0) {
        TNode<Whatever> leftNode(left,fio);
        leftNode.ReplaceAndRemoveMax(thisNode,fio,left);
      
      // left child only removal
      } else if (left != 0) {
        PositionInParent = left;
      
      // right child only removal
      } else if (right != 0) {
        PositionInParent = right;
      
      // leaf removal
      } else {
        PositionInParent = 0;
      }

      occupancy--;
      fio -> seekp(this_position);
      Write(fio);
      return TRUE;

    // search when element greater than this
    } else if (elementTNode.data > data) {
      
      if (right != 0) { // recursive case
        TNode<Whatever> rightNode(right,fio);
        rightNode.Remove(elementTNode,fio,occupancy,right,0);
      } else {
        return FALSE;
      }

    // search when element less than this
    } else {

      if (left != 0) { // recursive case
        TNode<Whatever> leftNode(left,fio);
        leftNode.Remove(elementTNode,fio,occupancy,left,0);
      } else {
        return FALSE;
      }

    } // end placement if-else
    
    // update height and balance of each node visited
    if (!fromSHB) {
      SetHeightAndBalance(fio,PositionInParent);
    }

    fio -> seekp(this_position);
    Write(fio);
    return TRUE;
}

/*
Name: Tree Remove
Purpose: remove data from the tree
Description:
      Determines if the element is the root data. If greater or less, sends it 
      into nodes to search. If equal, reassigns root appropriate to the number
      of children that root has. Writes the edits to disk, decrements 
      occupancy.
Parameters: 
      Whatever & element - data being removed
Return: TRUE/1 for success, FALSE/0 for failure
Result: the data is removed from the tree
Side effects: tree's static occupancy adjusted
*/
template <class Whatever>
unsigned long Tree<Whatever> :: Remove (Whatever & element) {

  // check for empty tree
  fio->seekg(0, ios::end);
  if (root == fio -> tellg()) {
    return 0;
  }

  unsigned long retval; // value to return
  TNode<Whatever> elementTNode(element); // temp for TNode's remove
  TNode<Whatever> rootNode(root,fio); // temp for root check
  
  IncrementOperation();

  // once the element is found, reassign pointers and delete, return TRUE
  if (rootNode.data == element) { // found node to remove

    element = rootNode.data; // reassign element to have the student's number
    
    // two children removal
    if (rootNode.left != 0 && rootNode.right != 0) {
      TNode<Whatever> leftNode(rootNode.left,fio);
      leftNode.ReplaceAndRemoveMax(rootNode,fio,rootNode.left);
      
      // SHAB of updated root node
      rootNode.SetHeightAndBalance(fio,root);

      // write the updated node
      fio -> seekp(root);
      rootNode.Write(fio);

    // left child only removal
    } else if (rootNode.left != 0) {
      root = rootNode.left;
    
    // right child only removal
    } else if (rootNode.right != 0) {
      root = rootNode.right;
    
    // leaf removal
    } else {
      ResetRoot();
    }

    occupancy--;
    return 1;
  }

  retval = rootNode.Remove(elementTNode,fio,occupancy,root);
  element = elementTNode.data;

  return retval;
}

/*
Name: SetHeightAndBalance
Purpose: fix the height and balance of nodes upon insertion and deletion
Description:
      pre-assign heights of each of the left and right chid nodes as
      -1, for the event of null. Then update to actual value if they are
      not null. Fix height of current by determining which of the child
      heigths is greater. Fix balance by taking difference of lefth to
      righth. Remove and reinsert in better location if it breaks
      balance threshold.
Parameters: 
      fstream * fio - filestream
      offset & PositionInParent - who called it? What's its position there?
Return: void
Result: 
      the node's balance and height fields are updated to reflect current
      tree. With recursive implementation of the file, the entire tree is
      kept up to date.
*/
template <class Whatever>
void TNode<Whatever> :: SetHeightAndBalance (fstream * fio,
	offset & PositionInParent) {

  long lefth, righth; // height of children

  // initialize for the case of null children
  lefth = -1;
  righth = -1;

  // assign left and right, if they aren't null
  if (left != 0) {
    TNode<Whatever> leftHeight(left,fio);
    lefth = leftHeight.height;
  }
  if (right != 0) {
    TNode<Whatever> rightHeight(right,fio);
    righth = rightHeight.height;
  }

  // fix height of current node
  if (lefth > righth) {
    height = lefth + 1;
  } else {
    height = righth + 1;
  }

  balance = lefth - righth; // time for the balancing act

  // fix if surpasses threshold
  if (abs(balance) > THRESHOLD) {
    long fakeOccupancy = 42;
    TNode<Whatever> removable(data);
    Remove(*this,fio,fakeOccupancy,PositionInParent,TRUE);
    TNode<Whatever> temp(PositionInParent,fio);
    temp.Insert(removable.data,fio,fakeOccupancy,PositionInParent);
  }
}

/*
Name: GetCost
Purpose: accessor for cost count (number of forays into the disk)
Parameters: none
Return:
      long - value of cost
Result: caller receives cost
*/
template <class Whatever>
long Tree <Whatever> :: GetCost () {
	return cost;
}

/* 
Name: GetOperation
Purpose: accessor for operation count
Parameters: none
Return:
      long - value of operation
Result: caller receives number of operations
*/
template <class Whatever>
long Tree <Whatever> :: GetOperation () {
  return operation;
}

/*
Name: IncrementCost
Purpose: increase cost by one. Called by Write and Read.
Paramters: none
Return: none
Result: cost is incremented
*/
template <class Whatever>
void Tree <Whatever> :: IncrementCost () {
	cost++;
}

/*
Name: IncrementOperation
Purpose: Increase number of operations by one. Called by Remove, Insert, Lookup
Parameters: none
Return: none
Result: operation is incremented
*/
template <class Whatever>
void Tree <Whatever> :: IncrementOperation () {
	operation++;
}


template <class Whatever>
void Tree <Whatever> :: ResetRoot () {
    fio -> seekp(0, ios :: end);
    offset end_position = fio -> tellp();
    //((TNode<Whatever>)root).this_position = end_position;
    root = end_position;

  //NEED TO FIGURE OUT PROPER SYNTAX!!!!!!!!! I Believe this is the 
  //gist of this function though
  // STAZIA REPLY: syntax-wise, I don't think root needs to be cast. We're
  // in the Tree, not the nodes, so root node isn't a thing. This is what I 
  // had in the case of removing a root with no children before I realized
  // that that's what ResetRoot is for:
  //
  //      fio -> seekg(0, ios::end);
  //    root = fio -> tellg();

}
	
/*
* Name: Insert
* Purpose: Insert a node into the tree
* Description:
        Check if duplicate entry. If so, write the new data to disk. Else,
        recursively navigate to the correct location of the element. Create a
        new node at that location and call SetHeightAndBalance for all nodes
        visited.
* Parameters: 
        Whatever & element - info to be entered as data
        fstream * fio - filestream
        long & occupancy - number of nodes in the tree
        offset & PositionInParent - position as interpreted by parent
* Return: TRUE/1 for success, FALSE/0 for failure
* Result: New node! Whoopie!
*/
template <class Whatever>
unsigned long TNode<Whatever> :: Insert (Whatever & element, fstream * fio,
	long & occupancy, offset & PositionInParent) {

  if (element == data) { // duplicate entry, switch to new data
    data = element;
    fio -> seekp(this_position);
    Write(fio);
    return 1;

  } else if (element > data) { // element larger, enter right
    fio -> seekp(0, ios :: end);

    if (right != 0) { // recursive case
      TNode<Whatever> rightNode(right,fio);
      rightNode.Insert(element, fio, occupancy,right);
    } else { // base case
      TNode<Whatever> inserted(element,fio,occupancy);
      right = inserted.this_position;
    }

  } else { // element less than, enter left
    fio -> seekp(0, ios :: end);
    if (left != 0) { // recursive case
      TNode<Whatever> leftNode(left,fio);
      leftNode.Insert(element,fio,occupancy,left);
    } else { // base case
      TNode<Whatever> inserted(element, fio, occupancy);
      left = inserted.this_position;
    }

  } // end placement if-else


  SetHeightAndBalance(fio,PositionInParent); // update height and balance
  fio -> seekp(this_position); // write updated node
  Write(fio);
}

/*
Name: Lookup
Purpose: search the tree for an element
Description: 
     for every element searched, check for equality. If not equal, it
     is either larger than or less than the element stored in the
     tree. Continue until element found. If null is found, then the
     item is not in the tree. Loop-based
Parameters:
     Whatever & element - the element to search for
Return: if successful, the data in the node. If unsuccessful, null
Result: item is found or not found. It answers the question.
*/
template <class Whatever>
unsigned long Tree<Whatever> :: Lookup (Whatever & element) const {
  IncrementOperation();
    
  // empty tree check
  fio->seekg(0, ios::end);
  if (root == fio -> tellg()) {
    return 0;
  }

  TNode<Whatever> working(root,fio); // working TNode
  
  while (true) {

    // is dulpicate?
    if (element == working.data) {
      element = working.data;
      return 1;
      
    // move to right pointer if element is greater than current node
    } else if (element > working.data) {
      if (working.right != 0) {
          working.Read(working.right,fio);
      } else {
        return 0;
      }

    // move to left pointer if element is less than current node
    } else if (!(element > working.data)) {
      if (working.left != 0) {
        working.Read(working.left,fio);
      } else {
        return 0;
      }
    } // end greaterthan/lessthan if
  } // end while
}

/*
Name: Read
Purpose: read a node from the disk
Description:
      Increment cost. Put pointer at the position of the data being requested,
      read the size of a node into this.
Parameters:
      const offset & position - where in the disk to start reading
      fstream * fio - filestream
Return: void
Result: data is read into memory
*/
template <class Whatever>
void TNode<Whatever> :: Read (const offset & position, fstream * fio) {
  
  Tree<Whatever> :: IncrementCost(); // increment cost

  // place pointer and read into this
	fio -> seekp (position);
  fio -> read ((char *) this, sizeof(TNode<Whatever>));

  // debug check
  if (Tree<Whatever> :: debug_on) {
    cerr << COST_READ << (const char *)data << "]\n";
  } 
}

/*
Name: TNode
Purpose: TNode Read constructor
Description: Call Read to get the data
Parameters:
      offset & position - where on the disk to start reading
      fstream * fio - filestream
Return: N/A
Result: TNode is construced, populated with data from disk
*/
template <class Whatever>
TNode<Whatever> :: TNode (const offset & position, fstream * fio) {
  Read (position, fio);
}

/*
Name: TNode
Purpose: TNode Write constructor
Description: 
      Populates TNode fields with default data and element. Increments
      occupancy, sets this_position to spot set by the caller, calls write to
      put the data on the disk
Parameters:
      Whatever & element - data being entered
      fstream * fio - filestream
      long & occupancy - number of nodes int the tree
Return: N/A
Result: TNode constructed and saved on disk
*/
template <class Whatever>
TNode<Whatever> :: TNode (Whatever & element, fstream * fio, long & occupancy): 
			data (element), height (0), balance (0), left (0), 
			right (0) {
  this_position = fio -> tellp();
  occupancy++;
  Write(fio);
}

/*
Name: Write
Purpose: write a node to disk
Description: Write the size of a node to the disk
Parameters: 
      fstream * fio - filestream
Return: void
Result: data stored on disk
*/
template <class Whatever>
void TNode<Whatever> :: Write (fstream * fio) const {

  // debug check
  if (Tree<Whatever> :: debug_on) {
    cerr << COST_WRITE << (const char *)data << "]\n";
  }

  Tree<Whatever> :: IncrementCost(); // cost increment

  fio -> write((const char *) this, sizeof(TNode<Whatever>)); // write data
}

/*
Name: Tree
Purpose: Tree constructor
Description:
      initialize tree_count, occupancy, and root. get beginning and ending
      locations, and compare to see if file is empty. If fle empty, write
      the root and occupancy to disk. Otherwise, Read the disk contents tp
      user.
Parameters:
      const char * datafile - file to which to write
Return: N/A
Result: Tree locked and loaded upon running dfatabase
*/
template <class Whatever>
Tree<Whatever> :: Tree (const char * datafile) :
	fio (new fstream (datafile, ios :: out | ios :: in)) {
  // fio initialiced
  
  // initialize values
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

  } else { // disk not empty, read out what's there
    fio -> seekg(0, ios :: beg);
    fio -> read((char *) &root, sizeof(root));
    fio -> read((char *) &occupancy, sizeof(occupancy));
  }
}

template <class Whatever>
Tree<Whatever> :: ~Tree (void)
/***************************************************************************
%                       HEADER SUPPLIED BY INSTRUCTOR
%
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

  fio -> seekp(0, ios :: beg); // navigate to beginning of file

  // save final root and occupancy to reflect changes made in this run
  fio -> write((const char *) &root, sizeof(root));
  fio -> write((const char *) &occupancy, sizeof(occupancy));
  root = 0;
  occupancy = 0;
  delete fio;
}	/* end: ~Tree */

/*
Name: Set_Debug_On
Purpose: set debug message to show by making debug_on = true
Parameters: void
Return: void
Result: debug messages will show
*/
template <class Whatever>
void Tree<Whatever> :: Set_Debug_On(void) {
  debug_on = 1;
}

/*
Name: Set_Debug_Off
Purpose: set debug messages to not show
Parameters: none
Return: none
Result: debug messages will not show
*/
template <class Whatever>
void Tree<Whatever> :: Set_Debug_Off(void) {
  debug_on = 0;
}

/* I did not write this function */
template <class Whatever>
ostream & operator << (ostream & stream, const TNode<Whatever> & nnn) {
	stream << "at height:  :" << nnn.height << " with balance:  "
		<< nnn.balance << "  ";
	return stream << nnn.data << "\n";
}

template <class Whatever>
ostream & Tree<Whatever> :: Write (ostream & stream) const
/***************************************************************************
%                       HEADER SUPPLIED BY INSTRUCTOR
%
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

/* I did not write this function */
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
