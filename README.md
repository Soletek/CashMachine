This is a school programming project made by myself in December 2014. The program, documentation and comments are in Finnish.
Copyright (C) 2016 Tuukka Kurtti 


I think the most challenging part of this exercise was designing the algorithm for dividing the user inputted money sum into bills. The original design only required the program to be able to divide the sum to bills with values of 20, 50 and 100 euros but I expanded the program to be able to divide the amount to any values. The algorithm should always find a closest value to the user input but never greater than it. The given result also contains a maximum number of highest value bills, which is not necessarily the combination with smallest number of bills, though. For example the user inputted sum 173€ will be divided into bills (30, 45 and 100) in the following way:
	
	3x 45€
	
	1x 30€

	TOTAL: 165€

The detailed explanation how this algorithm functions can be found in the file seteleihinjako.pdf (in Finnish)

Another part of my was a library I made to handle user inputs. The library allows the programmer to define predicates for every user input. If the user input doesn’t fill all the predicates an error message telling what went wrong is printed and the input is prompted again.
