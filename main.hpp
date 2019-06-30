/* File name: main.hpp
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Includes necessary headers to init symbol table and command
 * 			table, and to call state machines. Includes ctime and chrono
 * 			to grab timestamp for list file.
 * Last Modified: 2019-06-02
 */

#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <ctime>
#include <chrono>

#include "firstpass.hpp"	//	first pass state machine
#include "symbols.hpp"
#include "record.hpp"
#include "secondpass.hpp"

extern unsigned int START; // starting memory location for loader
// modified by END directive

#endif
