 /// 
 ///  @ Author: Kevin Gilliam
 ///  @ Create Time: 2022-09-07 14:03:11
 ///  @ Modified by: Kevin Gilliam
 ///  @ Modified time: 2022-09-07 15:30:41
 ///  @ Description:
 ///

#pragma once
#define CONCAT_(A, B) A ## B
#define CONCAT(A, B) CONCAT_(A, B)

#define EVAL(M) M


#if defined (STR)
#undef STR
#endif
#define STR(s) #s

#if defined (XSTR)
#undef XSTR
#endif
#define XSTR(s) STR(s)
