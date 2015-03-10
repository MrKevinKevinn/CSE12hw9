#include <iostream>
#include <cstdio>
#include <string>
#include <getopt.h>
#include "Driver.h"
#include "SymTab.h"
#include <fstream>

using namespace std;

#ifdef NULL
#undef NULL
#define NULL 0
#endif

ostream & operator << (ostream & stream, const UCSDStudent & stu) {
        return stream << "name:  " << stu.name
                << " with studentnum:  " << stu.studentnum;
}

int main (int argc, char * const * argv) {
        char buffer[BUFSIZ];
        char command;
        long number;
        char option;
        istream * is = &cin; // added for ASCII file input
        ostream * os = &cout;
        
        SymTab<UCSDStudent>::Set_Debug_Off ();

        while ((option = getopt (argc, argv, "x")) != EOF) {

        switch (option) {
                case 'x': SymTab<UCSDStudent>::Set_Debug_On ();
                        break;
                }       
        }
        
        SymTab<UCSDStudent> ST ("Driver.datafile");
        ST.Write (cout << "Initial Symbol Table:\n" );

        while (cin) {
                if (*is == NULL) {
                  if (*is == cin) {
                    break;
                  }

                  delete(is);
                  delete(os);

                  is = &cin;
                  os = &cout;

                }
                    
                command = NULL;         // reset command each time in loop
                if (is == &cin) {
                  *os << "Please enter a command ((i)nsert, "
                        << "(l)ookup, (r)emove, (w)rite):  ";
                }
                
                *is >> command;

                switch (command) {

                case 'f': {
                      if ((*is).peek() == '\n') {
                        *os << "Please enter file name for commands: ";
                      }

                      *is >> buffer; // get the file to read

                      if (is != &cin) {
                         delete(is);
                         delete(os);
                      }

                      is = new ifstream(buffer);
                      os = new ofstream("/dev/null");

                      break;

                }
                case 'i': {
                        *os << "Please enter UCSD student name to insert:  ";
                        *is >> buffer;  // formatted input

                        *os << "Please enter UCSD student number:  ";
                        *is >> number;

                        UCSDStudent stu (buffer, number);

                        // create student and place in symbol table
                        ST.Insert (stu);
                        break;
                }
                case 'l': { 
                        unsigned long found;    // whether found or not

                        *os << "Please enter UCSD student name to lookup:  ";
                        *is >> buffer;  // formatted input

                        UCSDStudent stu (buffer, 0);
                        found = ST.Lookup (stu);
                        
                        if (found)
                                cout << "Student found!!!\n" << stu << "\n";
                        else
                                cout << "student " << buffer << " not there!\n";
                        break;
                        }
                case 'r': { 
                        unsigned long removed;

                        *os << "Please enter UCSD student name to remove:  ";
                        *is >> buffer;  // formatted input

                        UCSDStudent stu (buffer, 0);
                        removed = ST.Remove(stu);

                        if (removed)
                                cout << "Student removed!!!\n" << stu << "\n";
                        else
                                cout << "student " << buffer << " not there!\n";
                        break;
                }
                case 'w':
                        ST.Write (cout << "The Symbol Table contains:\n");
                }
        }

        ST.Write (cout << "\nFinal Symbol Table:\n");
        if (ST.GetOperation() != 0) {
          *os << "\nCost of operations: ";
          *os << ST.GetCost();
          *os << " tree accesses";

          *os << "\nNumber of operations: ";
          *os << ST.GetOperation();

          *os << "\nAverage cost: ";
          *os << (((float)(ST.GetCost()))/(ST.GetOperation()));
          *os << " tree accesses/operation\n";
        } else
        *os << "\nNo cost information available.\n";

}
