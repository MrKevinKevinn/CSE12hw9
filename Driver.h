/*----------------------------------------------------------------------------
                                                        Stazia Tronboll
                                                        cs12xpr
                                                        CSE 12, Winter 2015
                                                        Feb 27 2015
                                Assignment 8
File: Driver.h

Defines UCSDStudent, constructor and overridden operators

Functions I wrote/edited:
      UCSDStudent - constructor
      const char * - get name
      == - equals according to name, regardless of data
      > - greater than
---------------------------------------------------------------------------*/


#ifndef DRIVER_H
#define DRIVER_H

#include <string.h>
#include <iostream>
#include <cstdlib>
using namespace std;

class UCSDStudent {
        friend ostream & operator << (ostream &, const UCSDStudent &);
        char name[20];
        long studentnum;



public:

  UCSDStudent(char * nm, long val) : studentnum(val) {
    memset(name, '\0', sizeof(name));
    strcpy(name, nm);
  }

  /*
  * Name: UCSDStudent
  * Purpose: assign name and number of a student
  * Description: copies the parameter name, assigns studentnum to val
  * Parameters:
          char * nm - the name of the student
          long val - student number
  * Return: N/A
  * Result: student created
  */
  UCSDStudent(void) : studentnum(0) {
    memset(name, '\0', sizeof(name));
  }


  /*
   * Name: UCSDStudent
   * Purpose: copy constructor
   * Description: 
   * Parameters:
   *        UCSDStudent & student - student to be copied
   * Return: N/A
   * Result: Deep copy of parameter student
   */
  UCSDStudent (const UCSDStudent & student) {
    memset (name, '\0', sizeof(name));
    strcpy (name, student.name);
    studentnum = student.studentnum;
  }

  /*
  * Name: const char *
  * Purpose: make an easy name to get to the student's name
  * Parameters: none
  * Return: 
          name - name of the student
  * Result: name is reutrned to caller
  */
  operator const char * () const {
    return name;
  }

  /*
  * Name: ==
  * Purpose: compare the names of this and the parameter object to determine
          if equal
  * Return: TRUE or FALSE
  * Result: comparison returned to caller
  */
  long operator == (const UCSDStudent & bbb) const {
    return !strcmp(name, bbb.name);
  }


  /*
  * Name: >
  * Purpose: compare the names of this and the parameter object to determine
          if greater than or less than, for placement of the compared
          node
  * Return: <= returns FALSE, > returns TRUE
  * Result: comparison returned to caller
  */
  long operator > (const UCSDStudent & bbb) const {
    return (strcmp (name, bbb.name) > 0) ? 1 : 0;
  }

};

#endif
