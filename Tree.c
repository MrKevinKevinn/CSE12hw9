/*------------------------------------------------------------------------
  Stazia Tronboll
  cs12xpr
  Kevin Koutney
  cs12xdx
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
static const char ENDD[] = "]\n"; 

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
#define MEANING_OF_LIFE_THE_UNIVERSE_AND_EVERYTHING 42

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
  long height; //height of TNode in tree
  long balance; //balance of TNode
  offset left; //position of left child
  offset right; //position of right child
  offset this_position;  // current position of this node

  // function fields:
  TNode () : height (0), balance (0), left (0), right (0), 
  this_position (0) {}

  // to declare the working TNode in Tree's Remove
  TNode (Whatever & element) : data (element), height (0), balance (0), 
  left (0), right (0), this_position (0) {}

  TNode (Whatever &, fstream *, long &);  // to add new node to disk
  TNode (const offset &, fstream *);  // to read node from disk

  unsigned long Insert (Whatever &, fstream *, long &, offset &); //insert node
  void Read (const offset &, fstream *);  // read node from disk
  unsigned long Remove (TNode<Whatever> &, fstream *, long &, offset &, 
      long fromSHB = FALSE); //remove node
  void ReplaceAndRemoveMax (TNode<Whatever> &, fstream *, offset &); //RARM
  void SetHeightAndBalance (fstream *, offset &); //SHAB
  void Write (fstream *) const;    // update node to disk

  ostream & Write_AllTNodes (ostream &, fstream *) const; //write all TNodes
}; //end of TNode struct



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
  //increment operation
  IncrementOperation();

  // get ending pointer
  fio->seekp(0, ios::end);
  offset ending = fio->tellp();

  // insert root
  if (ending == root) {
    TNode<Whatever> temp (element, fio, occupancy); // to write root Node  
    root = temp.this_position;
    return TRUE;

  // launch search for every other entry
  } else {
    TNode<Whatever> readRootNode (root, fio); // to read root Node
    return readRootNode.Insert(element, fio, occupancy, root);
  } //end of if/else insert chain

} //end of tree insert



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

  //if there is a right child, recursively call ReplaceRemoveMax on it
  if(right)
  {
    TNode<Whatever> RightNode (right, fio); // to read & RARM right Node
    RightNode.ReplaceAndRemoveMax(targetTNode, fio, right);
    SetHeightAndBalance(fio, PositionInParent);
  }

  /* otherwise, set targetTNode's data to this data, PointerInParent to left
     and delete this data. Print debug message if debug is on */
  else    
  {
    targetTNode.data = data;
    PositionInParent = left;
  } //end of if/else RARM chain

} //end of RARM



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
  /* retVal reverse recursively passes the return value up the tree after
    a successful or failed recursive search and removal */
  unsigned long retVal;

  // once the element is found, reassign pointers and delete, return TRUE
  if (elementTNode.data == data) { // found node to remove
    elementTNode.data = data; // reassign element to have the student's #

    // two children removal
    if (left && right) {
      TNode<Whatever> leftNode(left, fio); // to read left Node
      leftNode.ReplaceAndRemoveMax(*this, fio, left);
      occupancy--;

      //if this was called from SetHeightAndBalance, write fio
      if (fromSHB)
        Write(fio);
      //otherwise call SetHeightAndBalance
      else
        SetHeightAndBalance(fio, PositionInParent);

      //return true for sucessful remove
      return TRUE;

    // left child only removal, set PositionInParent to this node's left child
    } else if (left)
      PositionInParent = left;

    // right child only removal, set PositionInParent to this node's right child
    else if (right)
      PositionInParent = right;

    // leaf removal, set PositionInParent to NULL
    else
      PositionInParent = NULL;

    //decrement occupancy
    occupancy--;

    //if not from SetHeightAndBalance: Write fio
    if(!fromSHB)
      Write(fio);

    //return true for sucessful remove
    return TRUE;

  // search right when element is greater than this data
  } else if (elementTNode.data > data) {

    //recursive case: if a right node exists
    if (right) {
      // create right temporary node to call remove on, based on right position
      TNode<Whatever> rightNode(right, fio); //to read right node
      //set retVal to the result of rightNode's remove
      retVal = rightNode.Remove(elementTNode, fio, occupancy, right, fromSHB);
    //base case: if there is no right node
    } else {
      //search and remove failed, return false
      return FALSE;
    }

  // search left when element is less than this data
  } else {

    //recursive case: if a left node exists
    if (left) {
      // create left temporary node to call remove on, based on left position
      TNode<Whatever> leftNode(left, fio); //to read left node
      //set retVal to the result of leftNode's remove
      retVal = leftNode.Remove(elementTNode, fio, occupancy, left, fromSHB);
    //base case: if there is no right node
    } else {
      //search and remove failed, return false
      return FALSE;
    }

  } // end of recursive search if-else chain

  //if this was called from SetHeightAndBalance, write fio
  if (fromSHB)
    Write(fio);
  //otherwise call SetHeightAndBalance
  else
    SetHeightAndBalance(fio, PositionInParent);

  //return retVal, passing return value back up the tree if called recursively
  return retVal;
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
  //used to store the return value of TNode's remove
  unsigned long retVal;

  // check for empty tree
  if(!occupancy)
    return FALSE;

  TNode<Whatever> elementTNode(element); // temp TNode for TNode's remove
  TNode<Whatever> rootNode(root, fio); // temp TNode for root check

  //increment operation
  IncrementOperation();

  //if root has children, call remove on root
  if(occupancy > 1) {
    retVal = rootNode.Remove(elementTNode, fio, occupancy, root);
    element = elementTNode.data; 
  //otherwise copy data to output parameter element, reset root & write fio
  } else {
    element = rootNode.data; 
    ResetRoot();
    rootNode.Write(fio);
    //sucessful remove, set retVal to true
    retVal = TRUE;
  }

  //return the result of retval
  return retVal;
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

  // left and right node heights, initialize -1 for the case of null children
  long lefth = -1;
  long righth = -1;

  // assign left and/or right heights, depending on if they aren't null
  if (left) {
    TNode<Whatever> leftHeight(left, fio); // to read left node
    lefth = leftHeight.height;
  }
  if (right) {
    TNode<Whatever> rightHeight(right, fio); // to read right now
    righth = rightHeight.height;
  }

  // set height of current node based on height of tallest child
  if (lefth > righth) {
    height = lefth + 1;
  } else {
    height = righth + 1;
  }

  // set balance to the height of the left child minus height of right child
  balance = lefth - righth;

  // fix tree structure if this node surpasses threshold
  if (abs(balance) > THRESHOLD) {
    // yes, we know it's actually just the answer to the ultimate question
    long fakeOccupancy = MEANING_OF_LIFE_THE_UNIVERSE_AND_EVERYTHING;
    TNode<Whatever> removable(data);//copy tnode data to reinsert after remove
    //call remomove on this
    Remove(*this, fio, fakeOccupancy, PositionInParent, TRUE);
    TNode<Whatever> temp(PositionInParent, fio); //read temp to call insert
    temp.Insert(removable.data, fio, fakeOccupancy, PositionInParent);

    // otherwise, Write fio
  } else {
    Write(fio);
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
  //return the cost in this tree
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
  //return the operation in this tree
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
  //increment cost by 1
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
  //increment operation by 1
  operation++;
}



template <class Whatever>
void Tree <Whatever> :: ResetRoot () {
  //get end position
  fio -> seekp(0, ios :: end);
  offset end_position = fio -> tellp();
  //set root to end position
  root = end_position;
  //decrement occupancy
  occupancy--;
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

  // if data matches, it is a duplicate entry. Switch this to the new data
  if (element == data) {
    data = element;
    Write(fio);
    return TRUE;

  // if data is greater than element, search right
  } else if (element > data) {
    //seek to end
    fio -> seekp(0, ios :: end);
    // recursive case: if there is a right node, call Insert on it
    if (right) {
      TNode<Whatever> rightNode(right, fio); //node to call insert on
      rightNode.Insert(element, fio, occupancy, right);
    // base case: if there is no right node, insert element here
    } else {
      TNode<Whatever> inserted(element, fio, occupancy);
      right = inserted.this_position;
    } //end of right check
  // if data is less than element, search left
  } else {
    //seek to end
    fio -> seekp(0, ios :: end);
    // recursive case: if there is a left node, call Insert on it
    if (left) {
      TNode<Whatever> leftNode(left, fio); //node to call insert on
      leftNode.Insert(element, fio, occupancy, left);
    // base case: if there is no right node, insert element here
    } else {
      TNode<Whatever> inserted(element, fio, occupancy);
      left = inserted.this_position;
    } //end of left check

  } // end placement if-else

  SetHeightAndBalance(fio, PositionInParent); // update height and balance

  //return true for successful insert
  return TRUE;
} //end insert



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
  //increment operation
  IncrementOperation();
  // empty tree check, if empty return false. No node has been found
  fio->seekg(0, ios::end);
  if (root == fio -> tellg()) {
    return FALSE;
  }
  // working TNode to check data and children for node
  TNode<Whatever> working(root, fio);
  while (true) {
    // check if working node is the node we're looking for, if so return TRUE
    if (element == working.data) {
      element = working.data;
      return TRUE;

    // check to the right if element is greater than current node
    } else if (element > working.data) {
      //if there is a node to the right, continue loop on the right node
      if (working.right) {
        working.Read(working.right, fio);
      //otherwise, node is not where it would be. Lookup returns false
      } else {
        return FALSE;
      }

    // check to the left if element is less than current node
    } else {
      //if there is a node to the right, continue loop on the right node
      if (working.left) {
        working.Read(working.left, fio);
      //otherwise, node is not where it would be. Lookup returns false
      } else {
        return FALSE;
      }
    } // end greaterthan/lessthan if
  } // end while
} //end lookup



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

  // debug check, print cost debug message
  if (Tree<Whatever> :: debug_on) {
    cerr << COST_READ << (const char *)data << ENDD;
  } //end of debug check/message
} //end of Read



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
  //read fio at position
  Read (position, fio);
} //end of TNode Read constructor



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
    //set this_position to the current position
    this_position = fio -> tellp();
    //increment occupancy
    occupancy++;
    //write fio
    Write(fio);
} //end of TNode Write constructor



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

  // debug check, print cost debug message
  if (Tree<Whatever> :: debug_on) {
    cerr << COST_WRITE << (const char *)data << ENDD;
  }

  // increment cost
  Tree<Whatever> :: IncrementCost(); 
  // seek to this position and write data here
  fio -> seekp(this_position);
  fio -> write((const char *) this, sizeof(TNode<Whatever>));
} //end of Write



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

  // debug check, allocation message
  if (debug_on) {
    cerr << TREE << Tree<Whatever> :: tree_count << ALLOCATE;
  }

  // get beginning pointer
  fio -> seekg(0, ios::beg);
  offset beginning = fio->tellg();

  // get ending pointer
  fio->seekg(0, ios::end);
  offset ending = fio->tellg();

  // if beginning is same position as ending, file is empty, make root
  if (beginning == ending) { 
    fio -> seekp(0, ios :: beg);
    fio -> write((const char *) &root, sizeof(root));
    fio -> write ((const char *) &occupancy, sizeof(occupancy));

    root = fio->tellp();

  // otherwise disk not empty, read out what's there
  } else {
    fio -> seekg(0, ios :: beg);
    fio -> read((char *) &root, sizeof(root));
    fio -> read((char *) &occupancy, sizeof(occupancy));
  }
} //end of Tree Constructor

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
  // debug check and deallocate error message
  if (debug_on) {
    cerr << TREE << Tree<Whatever> :: tree_count << DEALLOCATE;
  }

  fio -> seekp(0, ios :: beg); // navigate to beginning of file

  // save final root and occupancy to reflect changes made in this run
  fio -> write((const char *) &root, sizeof(root));
  fio -> write((const char *) &occupancy, sizeof(occupancy));
  delete fio;
} // end of Tree Destructor

/*
Name: Set_Debug_On
Purpose: set debug message to show by making debug_on = true
Parameters: void
Return: void
Result: debug messages will show
 */
template <class Whatever>
void Tree<Whatever> :: Set_Debug_On(void) {
  //set debug_on to TRUE
  debug_on = TRUE;
} //end of Set_Debug_On

/*
Name: Set_Debug_Off
Purpose: set debug messages to not show
Parameters: none
Return: none
Result: debug messages will not show
 */
template <class Whatever>
void Tree<Whatever> :: Set_Debug_Off(void) {
  //set debug_on to TRUE
  debug_on = FALSE;
} //end of Set_Debug_Off



/* I did not write this function */
template <class Whatever>
ostream & operator << (ostream & stream, const TNode<Whatever> & nnn) {
  stream << "at height:  :" << nnn.height << " with balance:  "
    << nnn.balance << "  ";
  return stream << nnn.data << "\n";
} //end of 



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
template <class Whatever>
ostream & Tree<Whatever> :: Write (ostream & stream) const
{
  //temp cost to store cost
  long old_cost = cost;
  //write Tree
  stream << "Tree " << tree_count << ":\n"
    << "occupancy is " << occupancy << " elements.\n";
  //seek end
  fio->seekg (0, ios :: end);
  offset end = fio->tellg ();

  // check for new file
  if (root != end) {
    TNode<Whatever> readRootNode (root, fio);
    readRootNode.Write_AllTNodes (stream, fio);
  }

  // ignore cost when displaying nodes to users
  cost = old_cost;

  //return the stream
  return stream;
} //end of Tree's Write

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
} //end of Write_AllTNodes

//end of Tree.c file
